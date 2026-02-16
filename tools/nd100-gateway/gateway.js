#!/usr/bin/env node
//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//
// gateway.js - ND-100/CX Terminal Gateway Server
//
// Bridges TCP terminal connections to the ND-100 WASM emulator running
// in a browser Web Worker via WebSocket.
//
// Architecture:
//   Remote client (PuTTY/telnet) --TCP--> Gateway --WebSocket--> Worker
//
// Usage: node gateway.js [--config path] [--verbose]

'use strict';

const net = require('net');
const fs = require('fs');
const path = require('path');
const { WebSocketServer } = require('ws');

// =========================================================
// Configuration
// =========================================================
const args = process.argv.slice(2);
let configPath = path.join(__dirname, 'gateway.conf.json');
let verbose = false;

for (let i = 0; i < args.length; i++) {
  if (args[i] === '--config' && args[i + 1]) {
    configPath = args[++i];
  } else if (args[i] === '--verbose' || args[i] === '-v') {
    verbose = true;
  } else if (args[i] === '--help' || args[i] === '-h') {
    console.log('Usage: node gateway.js [--config path] [--verbose]');
    console.log('');
    console.log('Options:');
    console.log('  --config path  Path to config file (default: gateway.conf.json)');
    console.log('  --verbose, -v  Enable verbose logging');
    console.log('  --help, -h     Show this help');
    process.exit(0);
  }
}

let config;
try {
  config = JSON.parse(fs.readFileSync(configPath, 'utf8'));
} catch (err) {
  console.error('Failed to read config file:', configPath);
  console.error(err.message);
  process.exit(1);
}

function log(...args) {
  const ts = new Date().toISOString().slice(11, 19);
  console.log(`[${ts}]`, ...args);
}

function vlog(...args) {
  if (verbose) log(...args);
}

// =========================================================
// State
// =========================================================
let emulatorWs = null;           // Single WebSocket connection to emulator
let registeredTerminals = [];    // Terminal list from emulator { identCode, name, logicalDevice }
const tcpBindings = new Map();   // identCode -> { socket, clientAddr }
const menuClients = new Set();   // TCP sockets in menu-selection mode

// =========================================================
// WebSocket Server (emulator connection)
// =========================================================
const wsPort = config.websocket.port || 8765;
const wss = new WebSocketServer({ port: wsPort });

wss.on('listening', function() {
  log('WebSocket server listening on ws://localhost:' + wsPort);
});

wss.on('connection', function(ws, req) {
  const clientIp = req.socket.remoteAddress;

  if (emulatorWs) {
    log('Rejecting additional WebSocket connection from', clientIp);
    ws.close(4000, 'Only one emulator connection allowed');
    return;
  }

  emulatorWs = ws;
  log('Emulator connected from', clientIp);

  ws.on('message', function(data) {
    let msg;
    try {
      msg = JSON.parse(data);
    } catch (err) {
      vlog('Invalid JSON from emulator:', data.toString().slice(0, 100));
      return;
    }

    switch (msg.type) {

      case 'register': {
        registeredTerminals = msg.terminals || [];
        log('Registered', registeredTerminals.length, 'terminal(s):');
        for (const t of registeredTerminals) {
          log('  identCode=' + t.identCode, t.name,
              '(logical device ' + t.logicalDevice + ')');
        }
        break;
      }

      case 'term-output': {
        const binding = tcpBindings.get(msg.identCode);
        if (binding && binding.socket && !binding.socket.destroyed) {
          // Write raw bytes to TCP client
          binding.socket.write(Buffer.from(msg.data));
        }
        break;
      }

      case 'carrier': {
        vlog('Carrier', msg.missing ? 'missing' : 'present',
             'on identCode', msg.identCode);
        break;
      }

      default:
        vlog('Unknown emulator message type:', msg.type);
    }
  });

  ws.on('close', function(code, reason) {
    log('Emulator disconnected (code=' + code + ')');
    emulatorWs = null;
    registeredTerminals = [];

    // Disconnect all TCP clients gracefully
    for (const [identCode, binding] of tcpBindings) {
      if (binding.socket && !binding.socket.destroyed) {
        binding.socket.write('\r\nEmulator disconnected.\r\n');
        binding.socket.destroy();
      }
    }
    tcpBindings.clear();
  });

  ws.on('error', function(err) {
    log('WebSocket error:', err.message);
  });
});

wss.on('error', function(err) {
  console.error('WebSocket server error:', err.message);
  process.exit(1);
});

// =========================================================
// Helper: send JSON to emulator
// =========================================================
function sendToEmulator(msg) {
  if (emulatorWs && emulatorWs.readyState === 1) {
    emulatorWs.send(JSON.stringify(msg));
    return true;
  }
  return false;
}

// =========================================================
// Helper: build terminal menu text
// =========================================================
function buildMenu() {
  let text = '\r\n';
  text += '  ' + (config.terminals.welcome || 'ND-100/CX Terminal Server') + '\r\n';
  text += '  ' + '='.repeat((config.terminals.welcome || 'ND-100/CX Terminal Server').length) + '\r\n';
  text += '\r\n';

  if (registeredTerminals.length === 0) {
    text += '  No terminals available. Is the emulator connected?\r\n\r\n';
    text += '  Press Enter to refresh, or Ctrl-C to disconnect.\r\n';
    return text;
  }

  text += '  Available terminals:\r\n\r\n';
  for (let i = 0; i < registeredTerminals.length; i++) {
    const t = registeredTerminals[i];
    const inUse = tcpBindings.has(t.identCode);
    const num = String(i + 1).padStart(2, ' ');
    const status = inUse ? ' [in use]' : '';
    text += '  ' + num + '. ' + t.name + status + '\r\n';
  }
  text += '\r\n   0. Disconnect\r\n';
  text += '\r\n  Select terminal (1-' + registeredTerminals.length + ', 0=quit): ';
  return text;
}

// =========================================================
// TCP Server (remote terminal clients)
// =========================================================
const tcpPort = config.terminals.port || 5001;
const tcpServer = net.createServer(function(socket) {
  const clientAddr = socket.remoteAddress + ':' + socket.remotePort;
  log('TCP client connected from', clientAddr);

  // Start in menu mode
  let menuMode = true;
  let inputBuf = '';
  let boundIdentCode = null;

  menuClients.add(socket);

  // Send banner + menu
  socket.write(buildMenu());

  socket.on('data', function(data) {
    if (menuMode) {
      // Accumulate input, process on CR or LF
      inputBuf += data.toString('ascii');

      // Check for line completion
      const lineEnd = inputBuf.indexOf('\r');
      const lineEnd2 = inputBuf.indexOf('\n');
      const endPos = (lineEnd >= 0 && lineEnd2 >= 0) ? Math.min(lineEnd, lineEnd2)
                   : (lineEnd >= 0) ? lineEnd : lineEnd2;

      if (endPos < 0) return;  // Wait for more data

      const line = inputBuf.slice(0, endPos).trim();
      inputBuf = '';

      if (line === '') {
        // Refresh menu
        socket.write(buildMenu());
        return;
      }

      const choice = parseInt(line, 10);
      if (choice === 0) {
        socket.write('\r\n  Goodbye.\r\n');
        socket.end();
        return;
      }
      if (isNaN(choice) || choice < 1 || choice > registeredTerminals.length) {
        socket.write('\r\n  Invalid selection. Try again.\r\n');
        socket.write(buildMenu());
        return;
      }

      const terminal = registeredTerminals[choice - 1];

      // Check if already in use
      if (tcpBindings.has(terminal.identCode)) {
        socket.write('\r\n  ' + terminal.name + ' is already in use.\r\n');
        socket.write(buildMenu());
        return;
      }

      // Check emulator is connected
      if (!emulatorWs) {
        socket.write('\r\n  Emulator not connected.\r\n');
        socket.write(buildMenu());
        return;
      }

      // Bind TCP socket to this terminal
      boundIdentCode = terminal.identCode;
      tcpBindings.set(boundIdentCode, { socket: socket, clientAddr: clientAddr });
      menuMode = false;
      menuClients.delete(socket);

      log('Client', clientAddr, 'connected to', terminal.name,
          '(identCode=' + boundIdentCode + ')');

      socket.write('\r\n  Connected to ' + terminal.name + '\r\n\r\n');

      // Send telnet negotiation: character mode, no local echo
      // IAC WILL ECHO (I'll echo, you don't), IAC WILL SGA, IAC DO SGA
      socket.write(Buffer.from([
        255, 251, 1,   // IAC WILL ECHO
        255, 251, 3,   // IAC WILL SUPPRESS-GO-AHEAD
        255, 253, 3    // IAC DO SUPPRESS-GO-AHEAD
      ]));

      // Notify emulator
      sendToEmulator({
        type: 'client-connected',
        identCode: boundIdentCode,
        clientAddr: clientAddr
      });

    } else {
      // Connected mode: forward raw bytes as term-input
      // Strip telnet IAC sequences (0xFF followed by command + option bytes)
      const raw = Array.from(data);
      const bytes = [];
      for (let i = 0; i < raw.length; i++) {
        if (raw[i] === 255 && i + 2 < raw.length) {
          // IAC + command + option: skip 3 bytes
          vlog('Telnet IAC:', raw[i], raw[i+1], raw[i+2]);
          i += 2;
        } else {
          bytes.push(raw[i]);
        }
      }
      if (bytes.length > 0) {
        vlog('TCP->WS term-input identCode=' + boundIdentCode +
            ' bytes=[' + bytes.map(b => '0x' + b.toString(16)).join(',') + ']');
        sendToEmulator({
          type: 'term-input',
          identCode: boundIdentCode,
          data: bytes
        });
      }
    }
  });

  socket.on('close', function() {
    log('TCP client', clientAddr, 'disconnected');
    menuClients.delete(socket);

    if (boundIdentCode !== null) {
      tcpBindings.delete(boundIdentCode);
      sendToEmulator({
        type: 'client-disconnected',
        identCode: boundIdentCode
      });
      log('Released terminal identCode=' + boundIdentCode);
    }
  });

  socket.on('error', function(err) {
    vlog('TCP socket error from', clientAddr + ':', err.message);
  });
});

tcpServer.on('error', function(err) {
  console.error('TCP server error:', err.message);
  process.exit(1);
});

tcpServer.listen(tcpPort, function() {
  log('TCP terminal server listening on port', tcpPort);
  log('');
  log('Waiting for emulator WebSocket connection on ws://localhost:' + wsPort);
});

// =========================================================
// HDLC TCP servers (future, structure ready)
// =========================================================
if (config.hdlc) {
  for (const hdlcConf of config.hdlc) {
    if (!hdlcConf.enabled) {
      vlog('HDLC', hdlcConf.name, 'disabled (port', hdlcConf.port + ')');
      continue;
    }
    // Future: create TCP server for each enabled HDLC controller
    log('HDLC', hdlcConf.name, 'on port', hdlcConf.port, '- not yet implemented');
  }
}

// =========================================================
// Graceful shutdown
// =========================================================
function shutdown() {
  log('Shutting down...');

  // Close all TCP client connections
  for (const [identCode, binding] of tcpBindings) {
    if (binding.socket && !binding.socket.destroyed) {
      binding.socket.destroy();
    }
  }
  for (const sock of menuClients) {
    if (!sock.destroyed) sock.destroy();
  }

  tcpServer.close();
  wss.close();
  process.exit(0);
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
