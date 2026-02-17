#!/usr/bin/env node
//
// test-gateway.js - Unit tests for the ND-100 Terminal Gateway
//
// Starts the gateway server, then runs a series of tests simulating
// the emulator (WebSocket client) and remote terminal clients (TCP).
//
// Usage: node test-gateway.js [--verbose]
//
// Exit code 0 = all tests passed, 1 = failures

'use strict';

const net = require('net');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const WebSocket = require('ws');

const verbose = process.argv.includes('--verbose') || process.argv.includes('-v');
const GATEWAY_SCRIPT = path.join(__dirname, 'gateway.js');

// Test config: use non-standard ports to avoid conflicts
const TEST_WS_PORT = 18765;
const TEST_TCP_PORT = 15001;
const TEST_CONFIG = {
  websocket: { port: TEST_WS_PORT },
  terminals: { port: TEST_TCP_PORT, welcome: 'Test Terminal Server' },
  hdlc: []
};

let gatewayProc = null;
let passed = 0;
let failed = 0;
let testName = '';
const testConfigPath = path.join(__dirname, '.test-config.json');

function log(...args) {
  if (verbose) console.log('  [test]', ...args);
}

function assert(condition, msg) {
  if (!condition) {
    console.log('  FAIL: ' + testName + ' - ' + msg);
    failed++;
    return false;
  }
  return true;
}

function pass() {
  console.log('  PASS: ' + testName);
  passed++;
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// =========================================================
// Gateway process management
// =========================================================
function startGateway() {
  return new Promise((resolve, reject) => {
    fs.writeFileSync(testConfigPath, JSON.stringify(TEST_CONFIG));

    gatewayProc = spawn('node', [GATEWAY_SCRIPT, '--config', testConfigPath], {
      stdio: verbose ? 'inherit' : 'pipe'
    });

    gatewayProc.on('error', reject);

    // Wait for gateway to be ready by polling the TCP port
    let attempts = 0;
    const check = setInterval(() => {
      attempts++;
      if (attempts > 50) {
        clearInterval(check);
        reject(new Error('Gateway did not start in time'));
        return;
      }
      const sock = net.createConnection(TEST_TCP_PORT, '127.0.0.1');
      sock.on('connect', () => {
        sock.destroy();
        clearInterval(check);
        resolve();
      });
      sock.on('error', () => {
        sock.destroy();
      });
    }, 100);
  });
}

function stopGateway() {
  return new Promise(resolve => {
    try { fs.unlinkSync(testConfigPath); } catch(e) {}
    if (gatewayProc) {
      gatewayProc.on('exit', resolve);
      gatewayProc.kill('SIGTERM');
      setTimeout(() => {
        try { gatewayProc.kill('SIGKILL'); } catch(e) {}
        resolve();
      }, 2000);
    } else {
      resolve();
    }
  });
}

// =========================================================
// Helper: connect WebSocket as emulator, wait for open
// =========================================================
function connectEmulator() {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket('ws://127.0.0.1:' + TEST_WS_PORT);
    ws.on('open', () => resolve(ws));
    ws.on('error', reject);
    setTimeout(() => reject(new Error('WS connect timeout')), 5000);
  });
}

// Close WebSocket and wait for it to fully close on the gateway side
function closeEmulator(ws) {
  return new Promise((resolve) => {
    if (!ws || ws.readyState === WebSocket.CLOSED) {
      resolve();
      return;
    }
    ws.on('close', () => {
      // Extra delay to let gateway process the close
      setTimeout(resolve, 150);
    });
    ws.close();
    // Safety timeout
    setTimeout(resolve, 2000);
  });
}

// =========================================================
// Helper: connect TCP as terminal client, collect initial data
// =========================================================
function connectTCPClient() {
  return new Promise((resolve, reject) => {
    const sock = net.createConnection(TEST_TCP_PORT, '127.0.0.1');
    let data = '';
    sock.setEncoding('ascii');
    sock.on('connect', () => {
      // Collect initial banner data
      setTimeout(() => resolve({ socket: sock, banner: data }), 300);
    });
    sock.on('data', (chunk) => { data += chunk; });
    sock.on('error', reject);
    setTimeout(() => reject(new Error('TCP connect timeout')), 5000);
  });
}

function closeTCPClient(sock) {
  return new Promise(resolve => {
    if (!sock || sock.destroyed) { resolve(); return; }
    sock.on('close', () => setTimeout(resolve, 50));
    sock.destroy();
    setTimeout(resolve, 1000);
  });
}

// Helper: read from TCP socket with timeout
function tcpRead(sock, timeoutMs) {
  return new Promise((resolve) => {
    let data = '';
    const handler = (chunk) => { data += chunk; };
    sock.on('data', handler);
    setTimeout(() => {
      sock.removeListener('data', handler);
      resolve(data);
    }, timeoutMs || 500);
  });
}

// Helper: collect WebSocket messages for a duration (handles both JSON and binary)
function wsCollect(ws, timeoutMs) {
  return new Promise((resolve) => {
    let messages = [];
    const handler = (data, isBinary) => {
      if (isBinary) {
        var buf = Buffer.isBuffer(data) ? data : Buffer.from(data);
        if (buf.length >= 2 && buf[0] === 0x01) {
          // term-input binary frame: [0x01][identCode][data bytes...]
          messages.push({
            type: 'term-input',
            identCode: buf[1],
            data: Array.from(buf.slice(2))
          });
        } else {
          messages.push({ type: 'binary', raw: buf });
        }
      } else {
        try { messages.push(JSON.parse(data)); } catch(e) {}
      }
    };
    ws.on('message', handler);
    setTimeout(() => {
      ws.removeListener('message', handler);
      resolve(messages);
    }, timeoutMs || 500);
  });
}

// Helper: register terminals on a WebSocket connection
function registerTerminals(ws, terminals) {
  ws.send(JSON.stringify({ type: 'register', terminals: terminals }));
}

// Standard test terminal set
const TEST_TERMINALS = [
  { identCode: 43, name: 'TERMINAL 12', logicalDevice: 51 },
  { identCode: 44, name: 'TERMINAL 13', logicalDevice: 52 },
  { identCode: 45, name: 'TERMINAL 14', logicalDevice: 53 }
];

// =========================================================
// Tests
// =========================================================

async function test01_GatewayStarts() {
  testName = '01 Gateway starts and listens';
  // Already verified by startGateway()
  pass();
}

async function test02_WebSocketConnect() {
  testName = '02 Emulator connects via WebSocket';
  const ws = await connectEmulator();
  if (assert(ws.readyState === WebSocket.OPEN, 'WebSocket should be open')) {
    pass();
  }
  await closeEmulator(ws);
}

async function test03_RejectThirdWebSocket() {
  testName = '03 Third WebSocket connection rejected (emulator + disk = 2 allowed)';
  const ws1 = await connectEmulator();  // accepted as emulator
  const ws2 = await connectEmulator();  // accepted as disk I/O client
  await sleep(100);

  const result = await new Promise((resolve) => {
    const ws3 = new WebSocket('ws://127.0.0.1:' + TEST_WS_PORT);
    ws3.on('close', (code) => {
      resolve({ rejected: true, code: code });
    });
    ws3.on('error', () => {
      resolve({ rejected: true, code: -1 });
    });
    setTimeout(() => {
      try { ws3.close(); } catch(e) {}
      resolve({ rejected: false });
    }, 2000);
  });

  if (assert(result.rejected, 'Third connection should be rejected') &&
      assert(result.code === 4000 || result.code === -1, 'Should close with code 4000, got ' + result.code)) {
    pass();
  }

  await closeEmulator(ws2);
  await closeEmulator(ws1);
}

async function test04_TCPBannerNoEmulator() {
  testName = '04 TCP banner shows no-terminals when emulator not connected';
  const { socket, banner } = await connectTCPClient();
  log('Banner:', JSON.stringify(banner));
  if (assert(banner.includes('Test Terminal Server'), 'Banner should contain welcome message') &&
      assert(banner.includes('No terminals available'), 'Should say no terminals')) {
    pass();
  }
  await closeTCPClient(socket);
}

async function test05_RegisterTerminals() {
  testName = '05 Emulator registers terminals and TCP menu shows them';
  const ws = await connectEmulator();
  registerTerminals(ws, TEST_TERMINALS);
  await sleep(200);

  const { socket, banner } = await connectTCPClient();
  log('Banner with terminals:', JSON.stringify(banner));

  if (assert(banner.includes('TERMINAL 12'), 'Menu should list TERMINAL 12') &&
      assert(banner.includes('TERMINAL 13'), 'Menu should list TERMINAL 13') &&
      assert(banner.includes('TERMINAL 14'), 'Menu should list TERMINAL 14') &&
      assert(banner.includes('1.'), 'Menu should have numbered items')) {
    pass();
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

async function test06_SelectTerminal() {
  testName = '06 TCP client selects terminal and gets connected message';
  const ws = await connectEmulator();
  registerTerminals(ws, TEST_TERMINALS);
  await sleep(200);

  const { socket } = await connectTCPClient();

  // Start collecting WS messages before selecting
  const wsPromise = wsCollect(ws, 1000);

  socket.write('1\r\n');
  const response = await tcpRead(socket, 500);
  log('Select response:', JSON.stringify(response));

  if (assert(response.includes('Connected to TERMINAL 12'), 'Should confirm connection to TERMINAL 12')) {
    const wsMessages = await wsPromise;
    log('WS messages:', JSON.stringify(wsMessages));
    const connectMsg = wsMessages.find(m => m.type === 'client-connected');
    if (assert(connectMsg, 'Should send client-connected to emulator') &&
        assert(connectMsg.identCode === 43, 'identCode should be 43')) {
      pass();
    }
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

async function test07_InvalidSelection() {
  testName = '07 Invalid terminal selection shows error';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();

  socket.write('99\r\n');
  const response = await tcpRead(socket, 500);
  log('Invalid response:', JSON.stringify(response));

  if (assert(response.includes('Invalid selection'), 'Should say invalid selection')) {
    pass();
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

async function test08_TerminalInUse() {
  testName = '08 Terminal already in use is marked in menu';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  // First client grabs terminal 1
  const client1 = await connectTCPClient();
  client1.socket.write('1\r\n');
  await sleep(300);

  // Second client sees it in use
  const client2 = await connectTCPClient();
  log('Client2 banner:', JSON.stringify(client2.banner));

  // The menu should show [in use]
  if (assert(client2.banner.includes('[in use]'), 'Menu should show [in use] for occupied terminal')) {
    // Try to select it
    client2.socket.write('1\r\n');
    const response = await tcpRead(client2.socket, 500);
    log('In-use select response:', JSON.stringify(response));
    if (assert(response.includes('already in use'), 'Should say already in use')) {
      pass();
    }
  }

  await closeTCPClient(client1.socket);
  await closeTCPClient(client2.socket);
  await closeEmulator(ws);
}

async function test09_ByteForwarding_TCPtoWS() {
  testName = '09 Bytes forwarded from TCP client to emulator';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();
  socket.write('1\r\n');
  await sleep(300);

  // Start collecting WS messages, then send TCP data
  const wsPromise = wsCollect(ws, 1000);
  socket.write('Hello');
  const wsMessages = await wsPromise;
  log('WS messages from TCP input:', JSON.stringify(wsMessages));

  const inputMsg = wsMessages.find(m => m.type === 'term-input');
  if (assert(inputMsg, 'Should receive term-input message') &&
      assert(inputMsg.identCode === 43, 'identCode should be 43') &&
      assert(Array.isArray(inputMsg.data), 'data should be array') &&
      assert(inputMsg.data.length === 5, 'Should have 5 bytes for "Hello", got ' + (inputMsg.data ? inputMsg.data.length : 0)) &&
      assert(inputMsg.data[0] === 72, 'First byte should be H (72), got ' + (inputMsg.data ? inputMsg.data[0] : '?'))) {
    pass();
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

async function test10_ByteForwarding_WStoTCP() {
  testName = '10 Bytes forwarded from emulator to TCP client';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();
  socket.write('1\r\n');
  await sleep(300);

  // Send term-output from emulator as binary frame [0x02][identCode][data...]
  const tcpPromise = tcpRead(socket, 500);
  var termOutFrame = Buffer.from([0x02, 43, 87, 111, 114, 108, 100]);  // "World"
  ws.send(termOutFrame);
  const tcpData = await tcpPromise;
  log('TCP received:', JSON.stringify(tcpData));

  if (assert(tcpData === 'World', 'TCP client should receive "World", got ' + JSON.stringify(tcpData))) {
    pass();
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

async function test11_Disconnect_SendsMessage() {
  testName = '11 TCP disconnect sends client-disconnected to emulator';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();
  socket.write('1\r\n');
  await sleep(300);

  // Start collecting WS messages, then disconnect TCP
  const wsPromise = wsCollect(ws, 1000);
  socket.destroy();
  const wsMessages = await wsPromise;
  log('WS messages on disconnect:', JSON.stringify(wsMessages));

  const disconnMsg = wsMessages.find(m => m.type === 'client-disconnected');
  if (assert(disconnMsg, 'Should send client-disconnected') &&
      assert(disconnMsg.identCode === 43, 'identCode should be 43')) {
    pass();
  }

  await closeEmulator(ws);
}

async function test12_ReconnectAfterDisconnect() {
  testName = '12 Terminal available again after client disconnects';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  // First client connects then disconnects
  const client1 = await connectTCPClient();
  client1.socket.write('1\r\n');
  await sleep(300);
  await closeTCPClient(client1.socket);
  await sleep(300);

  // Second client should be able to connect to same terminal
  const client2 = await connectTCPClient();
  client2.socket.write('1\r\n');
  const response = await tcpRead(client2.socket, 500);
  log('Reconnect response:', JSON.stringify(response));

  if (assert(response.includes('Connected to TERMINAL 12'),
      'Should connect to TERMINAL 12 after previous client disconnected')) {
    pass();
  }

  await closeTCPClient(client2.socket);
  await closeEmulator(ws);
}

async function test13_EmulatorDisconnect_ClosesClients() {
  testName = '13 Emulator disconnect closes TCP clients';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();
  socket.write('1\r\n');
  await sleep(300);

  // Watch for TCP close
  const closePromise = new Promise(resolve => {
    socket.on('close', () => resolve(true));
    socket.on('end', () => resolve(true));
    setTimeout(() => resolve(socket.destroyed), 2000);
  });

  // Disconnect emulator
  ws.close();
  const closed = await closePromise;

  if (assert(closed, 'TCP client should be disconnected when emulator disconnects')) {
    pass();
  }

  if (!socket.destroyed) socket.destroy();
  // Wait for gateway to fully process the WS close
  await sleep(300);
}

async function test14_MenuRefresh() {
  testName = '14 Empty Enter refreshes menu';
  const ws = await connectEmulator();
  registerTerminals(ws, [TEST_TERMINALS[0]]);
  await sleep(200);

  const { socket } = await connectTCPClient();

  socket.write('\r\n');
  const response = await tcpRead(socket, 500);
  log('Refresh response:', JSON.stringify(response));

  if (assert(response.includes('TERMINAL 12'), 'Refreshed menu should show terminals')) {
    pass();
  }

  await closeTCPClient(socket);
  await closeEmulator(ws);
}

// =========================================================
// Runner
// =========================================================
async function runAllTests() {
  console.log('');
  console.log('ND-100 Terminal Gateway - Test Suite');
  console.log('====================================');
  console.log('');

  console.log('Starting gateway server...');
  try {
    await startGateway();
  } catch(err) {
    console.error('Failed to start gateway:', err.message);
    process.exit(1);
  }
  console.log('Gateway started (WS:' + TEST_WS_PORT + ' TCP:' + TEST_TCP_PORT + ')');
  console.log('');

  const tests = [
    test01_GatewayStarts,
    test02_WebSocketConnect,
    test03_RejectThirdWebSocket,
    test04_TCPBannerNoEmulator,
    test05_RegisterTerminals,
    test06_SelectTerminal,
    test07_InvalidSelection,
    test08_TerminalInUse,
    test09_ByteForwarding_TCPtoWS,
    test10_ByteForwarding_WStoTCP,
    test11_Disconnect_SendsMessage,
    test12_ReconnectAfterDisconnect,
    test13_EmulatorDisconnect_ClosesClients,
    test14_MenuRefresh
  ];

  for (const test of tests) {
    try {
      await test();
    } catch(err) {
      console.log('  FAIL: ' + testName + ' - EXCEPTION: ' + err.message);
      if (verbose) console.log(err.stack);
      failed++;
    }
    // Let sockets settle between tests
    await sleep(200);
  }

  console.log('');
  console.log('Results: ' + passed + ' passed, ' + failed + ' failed, ' + (passed + failed) + ' total');
  console.log('');

  await stopGateway();
  process.exit(failed > 0 ? 1 : 0);
}

runAllTests().catch(err => {
  console.error('Test runner failed:', err);
  stopGateway().then(() => process.exit(1));
});
