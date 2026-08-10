#!/usr/bin/env node
/*
 * test-ethernet.js - the gateway's ethernet segment, on its own.
 *
 * Runs gateway.js as a child process and joins its ethernet port as ordinary
 * RETH clients. No emulator, no wasm, no browser.
 *
 * WHAT THIS IS REALLY GUARDING
 * ----------------------------
 * Ethernet is MULTIPOINT and HDLC is not. The gateway's HDLC code keeps one
 * socket per channel; ethernet keeps a SET per segment and repeats to all the
 * others. A version that kept one socket would work perfectly with two members
 * and silently drop the third - and two is the first thing anyone tests by
 * hand. So the important case here is THREE members, not two.
 *
 * The second thing guarded is "never echo to the sender". NDIX drops a frame
 * whose source is its own address (if_ether.c:286), so an echo is harmless but
 * doubles every packet counter and reads like a duplicate-address fault.
 *
 *   node test-ethernet.js
 */
'use strict';

const net = require('net');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');

const RETH_MAGIC = Buffer.from('RETH', 'ascii');
const HANDSHAKE_LEN = 5;

const ETH_PORT = 13094;   // not the default 3094, so a real gateway can run too
const WS_PORT = 18765;

let passed = 0, failed = 0;
function check(what, ok) {
  if (ok) { passed++; console.log('  [PASS] ' + what); }
  else    { failed++; console.log('  [FAIL] ' + what); }
}
function checkEq(what, expect, got) {
  const e = JSON.stringify(expect), g = JSON.stringify(got);
  if (e === g) { passed++; console.log('  [PASS] ' + what); }
  else { failed++; console.log('  [FAIL] ' + what + ': expected ' + e + ', got ' + g); }
}

const sleep = ms => new Promise(r => setTimeout(r, ms));

/* One RETH member: connect, exchange the hello, collect whole frames. */
function joinSegment(port) {
  return new Promise((resolve, reject) => {
    const sock = net.connect(port, '127.0.0.1');
    const member = { sock, frames: [], rx: Buffer.alloc(0), greeted: false };
    sock.setNoDelay(true);
    sock.on('connect', () => {
      const hello = Buffer.alloc(HANDSHAKE_LEN);
      RETH_MAGIC.copy(hello, 0);
      hello[4] = 1;                       // version 1 = ordinary member
      sock.write(hello);
    });
    sock.on('data', d => {
      member.rx = Buffer.concat([member.rx, d]);
      if (!member.greeted) {
        if (member.rx.length < HANDSHAKE_LEN) return;
        if (member.rx.slice(0, 4).compare(RETH_MAGIC) !== 0) {
          reject(new Error('gateway did not send a RETH hello'));
          return;
        }
        member.rx = member.rx.slice(HANDSHAKE_LEN);
        member.greeted = true;
        resolve(member);
      }
      for (;;) {
        if (member.rx.length < 2) break;
        const len = (member.rx[0] << 8) | member.rx[1];
        if (member.rx.length < 2 + len) break;
        member.frames.push(member.rx.slice(2, 2 + len));
        member.rx = member.rx.slice(2 + len);
      }
    });
    sock.on('error', reject);
  });
}

function sendFrame(member, buf) {
  const out = Buffer.alloc(2 + buf.length);
  out[0] = (buf.length >> 8) & 0xFF;
  out[1] = buf.length & 0xFF;
  buf.copy(out, 2);
  member.sock.write(out);
}

/* A frame shaped like the one NDIX actually puts on the wire: broadcast ARP
 * request, 58 bytes - NDIX's own under-padded minimum. */
function arpFrame(stamp) {
  const f = Buffer.alloc(58);
  f.fill(0xFF, 0, 6);                                  // destination
  Buffer.from([0x08, 0x00, 0x26, 0xF4, 0x01, 0x00]).copy(f, 6);   // source
  f.writeUInt16BE(0x0806, 12);                         // ETHERTYPE_ARP
  f[57] = stamp;                                       // so we can tell them apart
  return f;
}

async function main() {
  // A config with only what this test needs. Written to a temp file so the
  // repository's own gateway.conf.json is left alone.
  const conf = {
    websocket: { port: WS_PORT },
    staticDir: '',
    terminals: { port: 15001, welcome: 'test' },
    hdlc: [],
    ethernet: [{ name: 'ETH-TEST', segment: 0, port: ETH_PORT, enabled: true }],
    smd: { images: [] }, floppy: { images: [] }, scsi: { images: [] }
  };
  const confPath = path.join(os.tmpdir(), 'gw-eth-test.json');
  fs.writeFileSync(confPath, JSON.stringify(conf));

  const gw = spawn('node', [path.join(__dirname, 'gateway.js'), '--config', confPath],
                   { stdio: ['ignore', 'pipe', 'pipe'] });
  let gwOut = '';
  gw.stdout.on('data', d => { gwOut += d.toString(); });
  gw.stderr.on('data', d => { gwOut += d.toString(); });

  try {
    await sleep(1200);
    if (!/ETH segment 0 listening/.test(gwOut)) {
      console.log('gateway did not start an ethernet segment. Output was:\n' + gwOut);
      process.exit(1);
    }
    check('the gateway announces the segment', true);

    console.log('\nthree members on one segment');
    const a = await joinSegment(ETH_PORT);
    const b = await joinSegment(ETH_PORT);
    const c = await joinSegment(ETH_PORT);
    check('three members completed the RETH handshake', true);
    await sleep(200);

    // THE CASE THAT MATTERS: one sender, TWO receivers.
    sendFrame(a, arpFrame(0xA1));
    await sleep(400);
    checkEq('B receives A\'s frame', 1, b.frames.length);
    checkEq('C receives it too - repeat is to ALL others, not one', 1, c.frames.length);
    checkEq('A does not get its own frame back', 0, a.frames.length);
    if (b.frames.length && c.frames.length) {
      checkEq('B got the bytes intact', 58, b.frames[0].length);
      checkEq('the stamp survives', 0xA1, b.frames[0][57]);
      check('B and C got identical bytes', b.frames[0].compare(c.frames[0]) === 0);
    }

    console.log('\nthe other direction');
    sendFrame(c, arpFrame(0xC3));
    await sleep(400);
    checkEq('A receives C\'s frame', 1, a.frames.length);
    checkEq('B has both now', 2, b.frames.length);
    checkEq('C still has only the one it received earlier', 1, c.frames.length);

    console.log('\na member leaving does not disturb the rest');
    b.sock.destroy();
    await sleep(400);
    sendFrame(a, arpFrame(0xA2));
    await sleep(400);
    checkEq('C still receives after B left', 2, c.frames.length);

    console.log('\nrubbish is refused, not forwarded');
    const bad = net.connect(ETH_PORT, '127.0.0.1');
    let closed = false;
    bad.on('close', () => { closed = true; });
    bad.on('end',   () => { closed = true; });   // FIN counts as dropped too
    bad.on('error', () => { closed = true; });   // so does an RST
    // MUST READ. A paused socket never observes the peer going away: the RST
    // is only surfaced on a read, so without this the test waits for a close
    // event that node has no reason to emit, and blames the gateway for it.
    // The gateway sends its RETH hello on connect, so there is always
    // something here to read.
    bad.resume();
    await new Promise(r => bad.once('connect', r));
    bad.write(Buffer.from('HTTP/1.1 GET /\r\n', 'ascii'));   // not a RETH hello

    // WAIT FOR THE CONDITION, not for a number of milliseconds. A fixed sleep
    // here passes on a quiet machine and fails on a busy one, which is the
    // worst kind of test - and the first version of this did exactly that.
    for (let i = 0; i < 40 && !closed && !bad.destroyed; i++) await sleep(50);
    check('a client that does not speak RETH is dropped', closed || bad.destroyed);
    if (!closed && !bad.destroyed) {
      console.log('  --- gateway said: ---');
      console.log(gwOut.split('\n').filter(l => /ETH/.test(l)).join('\n'));
    }

    a.sock.destroy();
    c.sock.destroy();
    await sleep(200);
  } finally {
    gw.kill('SIGTERM');
    await sleep(300);
    if (!gw.killed) gw.kill('SIGKILL');
    try { fs.unlinkSync(confPath); } catch (e) { /* already gone */ }
  }

  console.log('\n' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed === 0 ? 0 : 1);
}

main().catch(err => { console.error(err); process.exit(1); });
