//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// emu-worker.js - Web Worker script for WASM emulation core
//
// Runs the ND-100 emulator in a dedicated Worker thread so the emulation loop
// continues unthrottled even when the browser tab is backgrounded.
// Communication with the main thread happens via postMessage.
//
// Activated by ?worker=1 URL parameter or localStorage 'nd100x-worker' = 'true'.

'use strict';

// =========================================================
// State
// =========================================================
var running = false;
var initialized = false;

// =========================================================
// Unthrottled scheduling via MessageChannel
// =========================================================
// setTimeout(0) gets throttled to 1s+ in background tabs, even in Workers.
// MessageChannel.postMessage is NOT throttled - it fires immediately.
var _schedCh = new MessageChannel();
_schedCh.port1.onmessage = function() { runLoop(); };
function scheduleNext() { _schedCh.port2.postMessage(null); }

// =========================================================
// Module pre-configuration (non-modularized build)
// =========================================================
// The build is NOT modularized - nd100wasm.js creates a global Module.
// We pre-define Module config here; importScripts merges into it.
var Module = {
  locateFile: function(path) {
    // .wasm file is in parent dir relative to js/ where this worker lives
    return '../' + path;
  },
  print: function(text) {
    postMessage({ type: 'log', level: 'info', text: text });
  },
  printErr: function(text) {
    postMessage({ type: 'log', level: 'error', text: text });
  },
  onRuntimeInitialized: function() {
    Module._EnableTerminalRingBuffer(1);
    postMessage({ type: 'ready' });
  }
};

// Load the Emscripten-generated JS (merges into our pre-defined Module)
importScripts('../nd100wasm.js');

// =========================================================
// WebSocket bridge state (remote terminal gateway)
// =========================================================
var _ws = null;              // WebSocket connection to gateway
var _wsUrl = '';             // For reconnection
var _wsReconnectTimer = null;
var _remoteIdentCodes = {};  // identCode -> true (set of remote terminals)
var _remoteTerminals = [];   // { identCode, name, logicalDevice } for register msg

function wsConnect(url) {
  if (_ws) {
    try { _ws.close(); } catch(e) {}
  }
  _wsUrl = url;
  if (_wsReconnectTimer) { clearTimeout(_wsReconnectTimer); _wsReconnectTimer = null; }

  try {
    _ws = new WebSocket(url);
  } catch(err) {
    postMessage({ type: 'ws-status', connected: false, error: err.message });
    return;
  }

  _ws.binaryType = 'arraybuffer';

  _ws.onopen = function() {
    postMessage({ type: 'ws-status', connected: true, error: null });
    // Send register message with remote terminal list
    if (_remoteTerminals.length > 0) {
      _ws.send(JSON.stringify({ type: 'register', terminals: _remoteTerminals }));
    }
  };

  _ws.onmessage = function(ev) {
    // Binary frame: [type:1][identCode:1][data:N]
    // Type 0x01 = term-input
    if (ev.data instanceof ArrayBuffer) {
      var buf = new Uint8Array(ev.data);
      if (buf.length >= 2 && buf[0] === 0x01) {
        // term-input (binary)
        var identCode = buf[1];
        for (var i = 2; i < buf.length; i++) {
          Module._SendKeyToTerminal(identCode, buf[i]);
        }
      }
      return;
    }

    // JSON text frame (control messages only)
    var msg;
    try { msg = JSON.parse(ev.data); } catch(e) { return; }

    switch (msg.type) {
      case 'client-connected': {
        // Restore carrier on this terminal
        Module._SetTerminalCarrier(0, msg.identCode);
        postMessage({
          type: 'ws-client',
          action: 'connected',
          identCode: msg.identCode,
          clientAddr: msg.clientAddr
        });
        break;
      }
      case 'client-disconnected': {
        // Set carrier missing
        Module._SetTerminalCarrier(1, msg.identCode);
        postMessage({
          type: 'ws-client',
          action: 'disconnected',
          identCode: msg.identCode
        });
        break;
      }
    }
  };

  _ws.onclose = function(ev) {
    postMessage({ type: 'ws-status', connected: false, error: null });
    _ws = null;
    // Auto-reconnect after 3 seconds if we had a URL
    if (_wsUrl) {
      _wsReconnectTimer = setTimeout(function() {
        wsConnect(_wsUrl);
      }, 3000);
    }
  };

  _ws.onerror = function(ev) {
    postMessage({ type: 'ws-status', connected: false, error: 'WebSocket error' });
  };
}

function wsDisconnect() {
  _wsUrl = '';  // Clear URL to prevent reconnection
  if (_wsReconnectTimer) { clearTimeout(_wsReconnectTimer); _wsReconnectTimer = null; }
  if (_ws) {
    try { _ws.close(); } catch(e) {}
    _ws = null;
  }
  _remoteIdentCodes = {};
  _remoteTerminals = [];
  postMessage({ type: 'ws-status', connected: false, error: null });
}

function wsSend(msg) {
  if (_ws && _ws.readyState === 1) {
    _ws.send(JSON.stringify(msg));
  }
}

// =========================================================
// Terminal ring buffer flush
// =========================================================
function flushRingBuffer() {
  var output = [];
  if (typeof Module._PollTerminalOutput !== 'function') return output;
  var entry;
  var wsOutBuf = {};
  while ((entry = Module._PollTerminalOutput()) >= 0) {
    var ident = (entry >> 8) & 0xFF;
    if (_remoteIdentCodes[ident]) {
      if (!wsOutBuf[ident]) wsOutBuf[ident] = [];
      wsOutBuf[ident].push(entry & 0xFF);
    } else {
      output.push(entry);  // packed: (identCode << 8) | charCode
    }
  }
  // Send remote output over WebSocket as binary
  if (_ws && _ws.readyState === 1) {
    for (var rid in wsOutBuf) {
      var bytes = wsOutBuf[rid];
      var frame = new Uint8Array(2 + bytes.length);
      frame[0] = 0x02;  // term-output
      frame[1] = parseInt(rid) & 0xFF;
      for (var bi = 0; bi < bytes.length; bi++) {
        frame[2 + bi] = bytes[bi];
      }
      _ws.send(frame.buffer);
    }
  }
  return output;
}

// =========================================================
// Snapshot builder
// =========================================================
function buildSnapshot() {
  var snap = {
    // CPU registers
    pc:    Module._Dbg_GetPC(),
    regA:  Module._Dbg_GetRegA(),
    regD:  Module._Dbg_GetRegD(),
    regB:  Module._Dbg_GetRegB(),
    regT:  Module._Dbg_GetRegT(),
    regL:  Module._Dbg_GetRegL(),
    regX:  Module._Dbg_GetRegX(),
    sts:   Module._Dbg_GetSTS(),
    ea:    Module._Dbg_GetEA(),
    pil:   Module._Dbg_GetPIL(),

    // System registers
    pans:  Module._Dbg_GetPANS(),
    opr:   Module._Dbg_GetOPR(),
    pgs:   Module._Dbg_GetPGS(),
    pvl:   Module._Dbg_GetPVL(),
    iic:   Module._Dbg_GetIIC(),
    iid:   Module._Dbg_GetIID(),
    pid:   Module._Dbg_GetPID(),
    pie:   Module._Dbg_GetPIE(),
    csr:   Module._Dbg_GetCSR(),
    ald:   Module._Dbg_GetALD(),
    pes:   Module._Dbg_GetPES(),
    pgc:   Module._Dbg_GetPGC(),
    pea:   Module._Dbg_GetPEA(),

    // Write-only / mixed registers
    panc:  Module._Dbg_GetPANC(),
    lmp:   Module._Dbg_GetLMP(),
    iie:   Module._Dbg_GetIIE(),
    ccl:   Module._Dbg_GetCCL(),
    lcil:  Module._Dbg_GetLCIL(),
    ucil:  Module._Dbg_GetUCIL(),
    eccr:  Module._Dbg_GetECCR(),

    // Execution state
    runMode:    Module._Dbg_GetRunMode(),
    stopReason: Module._Dbg_GetStopReason(),
    instrCount: Module._Dbg_GetInstrCount(),
    isPaused:   Module._Dbg_IsPaused(),

    // Page tables
    pageTableCount: Module._Dbg_GetPageTableCount(),
    extendedMode:   Module._Dbg_GetExtendedMode()
  };

  // PCR for all 16 levels
  snap.pcr = [];
  for (var i = 0; i < 16; i++) {
    snap.pcr.push(Module._Dbg_GetPCR(i));
  }

  return snap;
}

// =========================================================
// Execution loop
// =========================================================
// Run many Step batches per frame, post to main thread at ~60fps.
// This avoids flooding the main thread with thousands of postMessages/sec.
var STEPS_PER_BATCH = 10000;   // instructions per Step() call
var FRAME_INTERVAL_MS = 16;    // ~60fps target for posting to main thread

function runLoop() {
  if (!running) return;

  var frameStart = performance.now();
  var termOutput = [];
  var runMode = 1;  // CPU_RUNNING

  // Execute as many batches as fit in one frame interval
  var wsOutBuf = {};  // identCode -> [bytes] for batching across all Step batches

  while (running) {
    Module._Step(STEPS_PER_BATCH);

    // Drain ring buffer, routing remote output to WebSocket
    var entry;
    while ((entry = Module._PollTerminalOutput()) >= 0) {
      var ident = (entry >> 8) & 0xFF;
      if (_remoteIdentCodes[ident]) {
        // Remote terminal -> batch for WebSocket
        if (!wsOutBuf[ident]) wsOutBuf[ident] = [];
        wsOutBuf[ident].push(entry & 0xFF);
      } else {
        // Local terminal -> main thread
        termOutput.push(entry);
      }
    }

    runMode = Module._Dbg_GetRunMode();
    if (runMode === 2 || runMode === 4 || runMode === 5) break;

    // Yield after frame interval so main thread can process
    if (performance.now() - frameStart >= FRAME_INTERVAL_MS) break;
  }

  // Send batched remote terminal output over WebSocket as binary (once per frame)
  // Binary format: [0x02][identCode][data bytes...]
  if (_ws && _ws.readyState === 1) {
    for (var rid in wsOutBuf) {
      var bytes = wsOutBuf[rid];
      var frame = new Uint8Array(2 + bytes.length);
      frame[0] = 0x02;  // term-output
      frame[1] = parseInt(rid) & 0xFF;
      for (var bi = 0; bi < bytes.length; bi++) {
        frame[2 + bi] = bytes[bi];
      }
      _ws.send(frame.buffer);
    }
  }

  // Send one consolidated frame to main thread (includes all CPU registers)
  postMessage({
    type: 'frame',
    termOutput: termOutput,
    runMode: runMode,
    pil: Module._Dbg_GetPIL(),
    sts: Module._Dbg_GetSTS(),
    pc: Module._Dbg_GetPC(),
    instrCount: Module._Dbg_GetInstrCount(),
    regA: Module._Dbg_GetRegA(),
    regD: Module._Dbg_GetRegD(),
    regB: Module._Dbg_GetRegB(),
    regT: Module._Dbg_GetRegT(),
    regL: Module._Dbg_GetRegL(),
    regX: Module._Dbg_GetRegX(),
    ea:   Module._Dbg_GetEA()
  });

  if (runMode === 2) { // CPU_BREAKPOINT
    running = false;
    postMessage({ type: 'breakpoint', snapshot: buildSnapshot() });
    return;
  }

  if (runMode === 4 || runMode === 5) { // CPU_STOPPED or CPU_SHUTDOWN
    running = false;
    postMessage({ type: 'stopped', snapshot: buildSnapshot() });
    return;
  }

  scheduleNext();  // MessageChannel — immune to background-tab throttling
}

// =========================================================
// Message handler
// =========================================================
onmessage = function(e) {
  var msg = e.data;
  if (!msg || !msg.type) return;

  switch (msg.type) {

    // --- Lifecycle ---
    case 'init': {
      var result = Module._Init();
      initialized = true;
      // Gather terminal info
      var terminalInfo = [];
      for (var i = 0; i < 16; i++) {
        var address = Module._GetTerminalAddress(i);
        if (address !== -1) {
          terminalInfo.push({
            index: i,
            address: address,
            identCode: Module._GetTerminalIdentCode(i),
            logicalDevice: Module._GetTerminalLogicalDevice(i)
          });
        }
      }
      postMessage({ type: 'initialized', result: result, terminals: terminalInfo });
      break;
    }

    case 'boot': {
      var bootResult = Module._Boot(msg.bootType);
      var termOutput = flushRingBuffer();
      postMessage({
        type: 'booted',
        result: bootResult,
        snapshot: buildSnapshot(),
        termOutput: termOutput,
        id: msg.id
      });
      break;
    }

    case 'start': {
      if (!running) {
        running = true;
        runLoop();
      }
      break;
    }

    case 'stop': {
      running = false;
      Module._Stop();
      postMessage({ type: 'stopped', snapshot: buildSnapshot() });
      break;
    }

    // --- Terminal I/O ---
    case 'key': {
      Module._SendKeyToTerminal(msg.identCode, msg.keyCode);
      break;
    }

    case 'carrier': {
      Module._SetTerminalCarrier(msg.flag, msg.identCode);
      break;
    }

    // --- Debugger step operations ---
    case 'step': {
      var stepMethods = {
        'stepOne':  Module._Dbg_StepOne,
        'stepOver': Module._Dbg_StepOver,
        'stepOut':  Module._Dbg_StepOut
      };
      var stepFn = stepMethods[msg.method];
      if (stepFn) {
        var count = msg.count || 1;
        for (var s = 0; s < count; s++) {
          stepFn();
        }
      }
      var stepTermOutput = flushRingBuffer();
      postMessage({
        type: 'stepDone',
        snapshot: buildSnapshot(),
        termOutput: stepTermOutput,
        id: msg.id
      });
      break;
    }

    case 'setPaused': {
      Module._Dbg_SetPaused(msg.paused ? 1 : 0);
      break;
    }

    case 'runDbg': {
      var wasRunning = running;
      running = false;  // Stop autonomous loop if running
      var remaining = Module._Dbg_RunWithBreakpoints(msg.maxSteps || 10000);
      var rdbTermOutput = flushRingBuffer();
      postMessage({
        type: 'runDbgDone',
        snapshot: buildSnapshot(),
        termOutput: rdbTermOutput,
        remaining: remaining,
        id: msg.id
      });
      break;
    }

    // --- Breakpoints ---
    case 'addBreakpoint': {
      Module._Dbg_AddBreakpoint(msg.addr);
      break;
    }

    case 'removeBreakpoint': {
      Module._Dbg_RemoveBreakpoint(msg.addr);
      break;
    }

    case 'clearBreakpoints': {
      Module._Dbg_ClearBreakpoints();
      break;
    }

    // --- Watchpoints ---
    case 'addWatchpoint': {
      Module._Dbg_AddWatchpoint(msg.addr, msg.wtype);
      break;
    }

    case 'removeWatchpoint': {
      Module._Dbg_RemoveWatchpoint(msg.addr);
      break;
    }

    case 'clearWatchpoints': {
      Module._Dbg_ClearWatchpoints();
      break;
    }

    // --- Register setters ---
    case 'setReg': {
      var setters = {
        pc:  Module._Dbg_SetPC,
        a:   Module._Dbg_SetRegA,
        d:   Module._Dbg_SetRegD,
        b:   Module._Dbg_SetRegB,
        t:   Module._Dbg_SetRegT,
        l:   Module._Dbg_SetRegL,
        x:   Module._Dbg_SetRegX,
        sts: Module._Dbg_SetSTS
      };
      var setter = setters[msg.reg];
      if (setter) setter(msg.value);
      break;
    }

    // --- Memory write ---
    case 'writeMemory': {
      Module._Dbg_WriteMemory(msg.addr, msg.value);
      break;
    }

    // --- Memory read ---
    case 'readMemory': {
      var val = Module._Dbg_ReadMemory(msg.addr);
      postMessage({ type: 'readMemoryResult', id: msg.id, value: val });
      break;
    }

    case 'readMemoryBlock': {
      var blockResult = [];
      for (var bi = 0; bi < msg.count; bi++) {
        blockResult.push(Module._Dbg_ReadMemory(msg.addr + bi));
      }
      postMessage({ type: 'readMemoryBlockResult', id: msg.id, values: blockResult });
      break;
    }

    case 'readPhysicalMemory': {
      var pval = Module._Dbg_ReadPhysicalMemory(msg.addr);
      postMessage({ type: 'readPhysicalMemoryResult', id: msg.id, value: pval });
      break;
    }

    case 'readPhysicalMemoryBlock': {
      var pblockResult = [];
      for (var pi = 0; pi < msg.count; pi++) {
        pblockResult.push(Module._Dbg_ReadPhysicalMemory(msg.addr + pi));
      }
      postMessage({ type: 'readPhysicalMemoryBlockResult', id: msg.id, values: pblockResult });
      break;
    }

    case 'dumpPhysicalMemory': {
      var dumpResult = Module._Dbg_DumpPhysicalMemory(msg.count);
      postMessage({ type: 'dumpPhysicalMemoryResult', id: msg.id, value: dumpResult });
      break;
    }

    // --- Breakpoint/Watchpoint queries ---
    case 'getBreakpointList': {
      var bpStr = Module.UTF8ToString(Module._Dbg_GetBreakpointList());
      postMessage({ type: 'getBreakpointListResult', id: msg.id, value: bpStr });
      break;
    }

    case 'getWatchpointInfo': {
      var wpCount = Module._Dbg_GetWatchpointCount();
      var wps = [];
      for (var wi = 0; wi < wpCount; wi++) {
        wps.push({
          addr: Module._Dbg_GetWatchpointAddr(wi),
          wtype: Module._Dbg_GetWatchpointType(wi)
        });
      }
      postMessage({ type: 'getWatchpointInfoResult', id: msg.id, count: wpCount, watchpoints: wps });
      break;
    }

    // --- Disassembly / Level info ---
    case 'disassemble': {
      var disasmStr = Module.UTF8ToString(Module._Dbg_Disassemble(msg.addr, msg.count));
      postMessage({ type: 'disassembleResult', id: msg.id, value: disasmStr });
      break;
    }

    case 'getLevelInfo': {
      var lvlStr = Module.UTF8ToString(Module._Dbg_GetLevelInfo());
      postMessage({ type: 'getLevelInfoResult', id: msg.id, value: lvlStr });
      break;
    }

    // --- ccall (generic Module.ccall proxy) ---
    case 'ccall': {
      try {
        var ccResult = Module.ccall(msg.name, msg.retType, msg.argTypes || [], msg.argValues || []);
        postMessage({ type: 'ccallResult', id: msg.id, value: ccResult });
      } catch(ccErr) {
        postMessage({ type: 'ccallResult', id: msg.id, error: ccErr.message });
      }
      break;
    }

    // --- Page tables ---
    case 'getPageTableEntryRaw': {
      var pteVal = Module._Dbg_GetPageTableEntryRaw(msg.pt, msg.vpage);
      postMessage({ type: 'getPageTableEntryRawResult', id: msg.id, value: pteVal });
      break;
    }

    case 'getPageTableMap': {
      var ptEntries = new Array(64);
      for (var pti = 0; pti < 64; pti++) {
        ptEntries[pti] = Module._Dbg_GetPageTableEntryRaw(msg.pt, pti) >>> 0;
      }
      postMessage({ type: 'getPageTableMapResult', id: msg.id, entries: ptEntries });
      break;
    }

    // --- FS read operations ---
    case 'fsRead': {
      try {
        var fileData = Module.FS.readFile(msg.path);
        var buf = fileData.buffer.slice(fileData.byteOffset, fileData.byteOffset + fileData.byteLength);
        postMessage({ type: 'fsReadResult', id: msg.id, buffer: buf }, [buf]);
      } catch(err) {
        postMessage({ type: 'fsReadResult', id: msg.id, error: err.message });
      }
      break;
    }

    case 'fsStat': {
      try {
        var stat = Module.FS.stat(msg.path);
        postMessage({ type: 'fsStatResult', id: msg.id, stat: { size: stat.size, mtime: stat.mtime } });
      } catch(err) {
        postMessage({ type: 'fsStatResult', id: msg.id, error: err.message });
      }
      break;
    }

    // --- Snapshot request ---
    case 'snapshot': {
      postMessage({ type: 'snapshot', snapshot: buildSnapshot() });
      break;
    }

    // --- Filesystem operations ---
    case 'loadDisk': {
      try {
        var data = new Uint8Array(msg.buffer);
        Module.FS.writeFile(msg.path, data);
        try { Module.FS.chmod(msg.path, 0x1B6); } catch(e) {}  // 0o666
        postMessage({ type: 'diskLoaded', path: msg.path, size: data.length, id: msg.id });
      } catch(err) {
        postMessage({ type: 'diskLoaded', path: msg.path, error: err.message, id: msg.id });
      }
      break;
    }

    case 'fsWrite': {
      try {
        var writeData = new Uint8Array(msg.buffer);
        Module.FS.writeFile(msg.path, writeData);
        try { Module.FS.chmod(msg.path, 0x1B6); } catch(e) {}
        postMessage({ type: 'fsResult', id: msg.id, size: writeData.length });
      } catch(err) {
        postMessage({ type: 'fsResult', id: msg.id, error: err.message });
      }
      break;
    }

    case 'fsUnlink': {
      try {
        Module.FS.unlink(msg.path);
      } catch(e) {}
      break;
    }

    // --- Floppy/SMD remount ---
    case 'remountFloppy': {
      var rf = Module._RemountFloppy(msg.unit);
      postMessage({ type: 'fsResult', id: msg.id, result: rf });
      break;
    }

    case 'remountSMD': {
      var rs = Module._RemountSMD(msg.unit);
      postMessage({ type: 'fsResult', id: msg.id, result: rs });
      break;
    }

    case 'unmountFloppy': {
      Module._UnmountFloppy(msg.unit);
      break;
    }

    case 'unmountSMD': {
      Module._UnmountSMD(msg.unit);
      break;
    }

    // --- WebSocket bridge ---
    case 'ws-connect': {
      wsConnect(msg.url);
      break;
    }

    case 'ws-disconnect': {
      wsDisconnect();
      break;
    }

    case 'enableRemoteTerminals': {
      var rtResult = Module._EnableRemoteTerminals();
      // Build remote terminal list for register message
      _remoteTerminals = [];
      _remoteIdentCodes = {};
      // Remote terminals occupy slots 8-15 (indices after the 8 local ones)
      for (var rti = 8; rti < 16; rti++) {
        var rtIdent = Module._GetTerminalIdentCode(rti);
        if (rtIdent !== -1) {
          var rtName = '';
          try {
            var namePtr = Module._GetTerminalName(rti);
            if (namePtr) rtName = Module.UTF8ToString(namePtr);
          } catch(e) {}
          var rtLogDev = Module._GetTerminalLogicalDevice(rti);
          _remoteTerminals.push({
            identCode: rtIdent,
            name: rtName || ('Terminal ' + rti),
            logicalDevice: rtLogDev
          });
          _remoteIdentCodes[rtIdent] = true;
        }
      }
      postMessage({
        type: 'enableRemoteTerminalsResult',
        id: msg.id,
        count: _remoteTerminals.length,
        terminals: _remoteTerminals
      });
      break;
    }

    default:
      postMessage({ type: 'log', level: 'warn', text: '[Worker] Unknown message type: ' + msg.type });
  }
};
