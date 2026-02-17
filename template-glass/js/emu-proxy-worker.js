//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// emu-proxy-worker.js - Worker-backed proxy with the exact same API as emu-proxy.js
//
// In Worker mode, the WASM module runs in a Web Worker. This proxy:
//  - Returns cached snapshot values for all getters
//  - Sends fire-and-forget commands via postMessage
//  - Uses Promise-based request/response for operations that return results
//  - Dispatches terminal output from Worker frame messages
//
// This file MUST load after module-init.js and before all other JS modules.

(function() {
  'use strict';

  // =========================================================
  // Worker lifecycle
  // =========================================================
  var _worker = new Worker('js/emu-worker.js');
  var _ready = false;
  var _initialized = false;
  var _pendingId = 0;
  var _pending = {};  // id -> {resolve, reject}
  var _snapshot = {};  // cached register/state snapshot
  var _terminals = [];  // cached terminal info from 'initialized' response

  // Phase 3 stub warnings: log each message at most once
  var _warned = {};
  function warnOnce(msg) {
    if (!_warned[msg]) { _warned[msg] = true; console.warn(msg); }
  }

  // Callbacks for async operations
  var _onInitialized = null;  // set by toolbar.js
  var _onBooted = null;       // set by toolbar.js
  var _onBreakpoint = null;   // set by emulation.js / debugger.js
  var _onStopped = null;      // set by emulation.js
  var _onStepDone = null;     // set by debugger.js
  var _onRunDbgDone = null;   // set by debugger.js

  function nextId() {
    return ++_pendingId;
  }

  function resolveRequest(id, data) {
    if (_pending[id]) {
      _pending[id].resolve(data);
      delete _pending[id];
    }
  }

  // =========================================================
  // Worker message handler
  // =========================================================
  _worker.onmessage = function(e) {
    var msg = e.data;
    if (!msg || !msg.type) return;

    switch (msg.type) {

      case 'ready':
        _ready = true;
        // Trigger module-init.js startup path
        if (typeof window.onWorkerReady === 'function') {
          window.onWorkerReady();
        }
        break;

      case 'initialized':
        _initialized = true;
        _terminals = msg.terminals || [];
        if (_onInitialized) {
          _onInitialized(msg);
          _onInitialized = null;
        }
        break;

      case 'booted':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        dispatchTermOutput(msg.termOutput);
        if (_onBooted) {
          _onBooted(msg);
          _onBooted = null;
        }
        resolveRequest(msg.id, msg);
        break;

      case 'frame':
        // Update CPU state from frame
        _snapshot.runMode = msg.runMode;
        _snapshot.pil = msg.pil;
        _snapshot.sts = msg.sts;
        _snapshot.pc = msg.pc;
        _snapshot.instrCount = msg.instrCount;
        _snapshot.regA = msg.regA;
        _snapshot.regD = msg.regD;
        _snapshot.regB = msg.regB;
        _snapshot.regT = msg.regT;
        _snapshot.regL = msg.regL;
        _snapshot.regX = msg.regX;
        _snapshot.ea = msg.ea;
        // Dispatch terminal output
        dispatchTermOutput(msg.termOutput);
        break;

      case 'breakpoint':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        if (_onBreakpoint) _onBreakpoint(msg);
        break;

      case 'stopped':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        if (_onStopped) _onStopped(msg);
        break;

      case 'stepDone':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        dispatchTermOutput(msg.termOutput);
        if (_onStepDone) _onStepDone(msg);
        resolveRequest(msg.id, msg);
        break;

      case 'runDbgDone':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        dispatchTermOutput(msg.termOutput);
        if (_onRunDbgDone) _onRunDbgDone(msg);
        resolveRequest(msg.id, msg);
        break;

      case 'snapshot':
        if (msg.snapshot) applySnapshot(msg.snapshot);
        break;

      case 'diskLoaded':
        resolveRequest(msg.id, msg);
        break;

      case 'fsResult':
        resolveRequest(msg.id, msg);
        break;

      // --- Phase 3: read operation results ---
      case 'readMemoryResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'readMemoryBlockResult':
        resolveRequest(msg.id, msg.values);
        break;
      case 'readPhysicalMemoryResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'readPhysicalMemoryBlockResult':
        resolveRequest(msg.id, msg.values);
        break;
      case 'dumpPhysicalMemoryResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'getBreakpointListResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'getWatchpointInfoResult':
        resolveRequest(msg.id, msg);
        break;
      case 'disassembleResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'getLevelInfoResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'getPageTableEntryRawResult':
        resolveRequest(msg.id, msg.value);
        break;
      case 'getPageTableMapResult':
        resolveRequest(msg.id, msg.entries);
        break;
      case 'getDriveInfoResult':
        resolveRequest(msg.id, msg.data);
        break;
      case 'ccallResult':
        resolveRequest(msg.id, msg.error ? null : msg.value);
        break;
      case 'fsReadResult':
        if (msg.error) {
          resolveRequest(msg.id, new Uint8Array(0));
        } else {
          resolveRequest(msg.id, new Uint8Array(msg.buffer));
        }
        break;
      case 'fsStatResult':
        if (msg.error) {
          resolveRequest(msg.id, { size: 0 });
        } else {
          resolveRequest(msg.id, msg.stat);
        }
        break;

      // --- WebSocket bridge status ---
      case 'ws-status':
        if (typeof window.onWsStatusChange === 'function') {
          window.onWsStatusChange(msg.connected, msg.error);
        }
        break;

      case 'ws-client':
        if (typeof window.onWsClientChange === 'function') {
          window.onWsClientChange(msg.action, msg.identCode, msg.clientAddr);
        }
        break;

      case 'ws-stats':
        if (typeof window.onWsStatsUpdate === 'function') {
          window.onWsStatsUpdate(msg.stats);
        }
        break;

      case 'enableRemoteTerminalsResult':
        resolveRequest(msg.id, msg);
        break;

      case 'opfsMountResult':
        resolveRequest(msg.id, msg);
        break;

      case 'gatewayMountResult':
        resolveRequest(msg.id, msg);
        break;

      case 'disk-connected':
        if (typeof window.onDiskConnected === 'function') {
          window.onDiskConnected(msg.blockIO);
        }
        break;

      case 'disk-disconnected':
        if (typeof window.onDiskDisconnected === 'function') {
          window.onDiskDisconnected();
        }
        break;

      case 'disk-list':
        if (typeof window.onDiskList === 'function') {
          window.onDiskList(msg.data);
        }
        break;

      case 'log':
        if (msg.level === 'error') {
          console.error('[Worker]', msg.text);
        } else if (msg.level === 'warn') {
          warnOnce('[Worker]', msg.text);
        } else {
          console.log('[Worker]', msg.text);
        }
        break;
    }
  };

  // =========================================================
  // Helpers
  // =========================================================
  function applySnapshot(snap) {
    for (var key in snap) {
      _snapshot[key] = snap[key];
    }
  }

  function dispatchTermOutput(termOutput) {
    if (!termOutput || termOutput.length === 0) return;
    var handler = window.handleTerminalOutputFromC;
    if (!handler) return;
    for (var i = 0; i < termOutput.length; i++) {
      var entry = termOutput[i];
      handler((entry >> 8) & 0xFF, entry & 0xFF);
    }
  }

  function postCmd(type, payload) {
    var msg = payload || {};
    msg.type = type;
    _worker.postMessage(msg);
  }

  function postRequest(type, payload) {
    var msg = payload || {};
    msg.type = type;
    var id = nextId();
    msg.id = id;
    return new Promise(function(resolve, reject) {
      _pending[id] = { resolve: resolve, reject: reject };
      _worker.postMessage(msg);
    });
  }

  function postTransfer(type, payload, transferList) {
    var msg = payload || {};
    msg.type = type;
    var id = nextId();
    msg.id = id;
    return new Promise(function(resolve, reject) {
      _pending[id] = { resolve: resolve, reject: reject };
      _worker.postMessage(msg, transferList);
    });
  }

  // =========================================================
  // Proxy API (same surface as emu-proxy.js)
  // =========================================================
  window.emu = {

    // --- Lifecycle ---
    init: function() {
      postCmd('init');
      return 0;  // Return immediately; result arrives via callback
    },
    boot: function(t) {
      postCmd('boot', { bootType: t, id: nextId() });
      return 0;  // Async - result via onBooted callback
    },
    step: function(n) {
      // In Worker mode, step is a no-op - Worker runs autonomously
    },
    stop: function() {
      postCmd('stop');
    },
    isInitialized: function() { return _initialized ? 1 : 0; },

    // --- Terminal I/O ---
    sendKey:              function(id, k) { postCmd('key', { identCode: id, keyCode: k }); return 1; },
    getTerminalAddress:   function(i) {
      for (var t = 0; t < _terminals.length; t++) {
        if (_terminals[t].index === i) return _terminals[t].address;
      }
      return -1;
    },
    getTerminalIdentCode: function(i) {
      for (var t = 0; t < _terminals.length; t++) {
        if (_terminals[t].index === i) return _terminals[t].identCode;
      }
      return -1;
    },
    getTerminalLogicalDevice: function(i) {
      for (var t = 0; t < _terminals.length; t++) {
        if (_terminals[t].index === i) return _terminals[t].logicalDevice;
      }
      return -1;
    },
    setTerminalCarrier: function(f, id) { postCmd('carrier', { flag: f, identCode: id }); },

    // --- Terminal output handler setup ---
    hasJSTerminalHandler: function() { return true; },
    setJSTerminalOutputHandler: function(v) { /* no-op in Worker mode */ },

    // --- Terminal ring buffer ---
    enableRingBuffer: function() { return true; },  // Worker enables it at init
    hasRingBuffer: function() { return true; },
    flushTerminalOutput: function() {
      // No-op in Worker mode - output arrives via frame messages
    },

    // --- Legacy terminal callback registration ---
    hasAddFunction: function() { return false; },
    addFunction: function(fn, sig) { return 0; },
    hasSetTerminalOutputCallback: function() { return false; },
    setTerminalOutputCallback: function(id, cb) { },

    // --- Registers (getters from snapshot) ---
    getPC:    function() { return _snapshot.pc || 0; },
    getRegA:  function() { return _snapshot.regA || 0; },
    getRegD:  function() { return _snapshot.regD || 0; },
    getRegB:  function() { return _snapshot.regB || 0; },
    getRegT:  function() { return _snapshot.regT || 0; },
    getRegL:  function() { return _snapshot.regL || 0; },
    getRegX:  function() { return _snapshot.regX || 0; },
    getSTS:   function() { return _snapshot.sts || 0; },
    getEA:    function() { return _snapshot.ea || 0; },
    getPIL:   function() { return _snapshot.pil || 0; },

    // --- Registers (setters - fire-and-forget) ---
    setPC:    function(v) { postCmd('setReg', { reg: 'pc', value: v }); _snapshot.pc = v; },
    setRegA:  function(v) { postCmd('setReg', { reg: 'a', value: v }); _snapshot.regA = v; },
    setRegD:  function(v) { postCmd('setReg', { reg: 'd', value: v }); _snapshot.regD = v; },
    setRegB:  function(v) { postCmd('setReg', { reg: 'b', value: v }); _snapshot.regB = v; },
    setRegT:  function(v) { postCmd('setReg', { reg: 't', value: v }); _snapshot.regT = v; },
    setRegL:  function(v) { postCmd('setReg', { reg: 'l', value: v }); _snapshot.regL = v; },
    setRegX:  function(v) { postCmd('setReg', { reg: 'x', value: v }); _snapshot.regX = v; },
    setSTS:   function(v) { postCmd('setReg', { reg: 'sts', value: v }); _snapshot.sts = v; },

    // --- System registers (from snapshot) ---
    getPANS: function() { return _snapshot.pans || 0; },
    getOPR:  function() { return _snapshot.opr || 0; },
    getPGS:  function() { return _snapshot.pgs || 0; },
    getPVL:  function() { return _snapshot.pvl || 0; },
    getIIC:  function() { return _snapshot.iic || 0; },
    getIID:  function() { return _snapshot.iid || 0; },
    getPID:  function() { return _snapshot.pid || 0; },
    getPIE:  function() { return _snapshot.pie || 0; },
    getCSR:  function() { return _snapshot.csr || 0; },
    getALD:  function() { return _snapshot.ald || 0; },
    getPES:  function() { return _snapshot.pes || 0; },
    getPGC:  function() { return _snapshot.pgc || 0; },
    getPEA:  function() { return _snapshot.pea || 0; },

    // --- System registers (write-only / mixed from snapshot) ---
    getPANC: function() { return _snapshot.panc || 0; },
    getLMP:  function() { return _snapshot.lmp || 0; },
    getIIE:  function() { return _snapshot.iie || 0; },
    getCCL:  function() { return _snapshot.ccl || 0; },
    getLCIL: function() { return _snapshot.lcil || 0; },
    getUCIL: function() { return _snapshot.ucil || 0; },
    getECCR: function() { return _snapshot.eccr || 0; },
    getPCR:  function(l) {
      if (_snapshot.pcr && l >= 0 && l < 16) return _snapshot.pcr[l];
      return 0;
    },

    // --- Execution state (from snapshot) ---
    getRunMode:    function() { return _snapshot.runMode || 0; },
    getStopReason: function() { return _snapshot.stopReason || 0; },
    getInstrCount: function() { return _snapshot.instrCount || 0; },
    setPaused:     function(p) { postCmd('setPaused', { paused: !!p }); },
    isPaused:      function() { return _snapshot.isPaused || 0; },

    // --- Step/Run (async in Worker mode) ---
    stepOne: function() {
      return postRequest('step', { method: 'stepOne', count: 1 });
    },
    stepOver: function() {
      return postRequest('step', { method: 'stepOver', count: 1 });
    },
    stepOut: function() {
      return postRequest('step', { method: 'stepOut', count: 1 });
    },
    runWithBreakpoints: function(n) {
      return postRequest('runDbg', { maxSteps: n });
    },

    // --- Memory (async in Worker mode - returns Promises) ---
    readMemory: function(a) {
      return postRequest('readMemory', { addr: a });
    },
    writeMemory: function(a, v) {
      postCmd('writeMemory', { addr: a, value: v });
    },
    readMemoryBlock: function(a, n) {
      return postRequest('readMemoryBlock', { addr: a, count: n });
    },
    readPhysicalMemory: function(a) {
      return postRequest('readPhysicalMemory', { addr: a });
    },
    readPhysicalMemoryBlock: function(a, n) {
      return postRequest('readPhysicalMemoryBlock', { addr: a, count: n });
    },
    dumpPhysicalMemory: function(n) {
      return postRequest('dumpPhysicalMemory', { count: n });
    },

    // --- Breakpoints (fire-and-forget) ---
    addBreakpoint:     function(a) { postCmd('addBreakpoint', { addr: a }); },
    removeBreakpoint:  function(a) { postCmd('removeBreakpoint', { addr: a }); },
    clearBreakpoints:  function()  { postCmd('clearBreakpoints'); },
    getBreakpointList: function() {
      return postRequest('getBreakpointList', {});
    },

    // --- Watchpoints (fire-and-forget) ---
    addWatchpoint:      function(a, t) { postCmd('addWatchpoint', { addr: a, wtype: t }); },
    removeWatchpoint:   function(a)    { postCmd('removeWatchpoint', { addr: a }); },
    clearWatchpoints:   function()     { postCmd('clearWatchpoints'); },
    getWatchpointCount: function() {
      return postRequest('getWatchpointInfo', {}).then(function(r) { return r.count; });
    },
    getWatchpointAddr: function(i) {
      return postRequest('getWatchpointInfo', {}).then(function(r) {
        return (r.watchpoints && r.watchpoints[i]) ? r.watchpoints[i].addr : 0;
      });
    },
    getWatchpointType: function(i) {
      return postRequest('getWatchpointInfo', {}).then(function(r) {
        return (r.watchpoints && r.watchpoints[i]) ? r.watchpoints[i].wtype : 0;
      });
    },

    // --- Disassembly / Level info (async in Worker mode) ---
    disassemble: function(a, n) {
      return postRequest('disassemble', { addr: a, count: n });
    },
    getLevelInfo: function() {
      return postRequest('getLevelInfo', {});
    },

    // --- DAP (proxied through Worker) ---
    ccall: function(name, retType, argTypes, argValues) {
      return postRequest('ccall', { name: name, retType: retType, argTypes: argTypes || [], argValues: argValues || [] });
    },

    // --- Page tables (from snapshot) ---
    getPageTableCount:    function() { return _snapshot.pageTableCount || 0; },
    getPageTableEntryRaw: function(p, v) {
      return postRequest('getPageTableEntryRaw', { pt: p, vpage: v });
    },
    getPageTableMap: function(pt) {
      return postRequest('getPageTableMap', { pt: pt });
    },
    getExtendedMode: function() { return _snapshot.extendedMode || 0; },

    // --- Floppy/SMD ---
    remountFloppy: function(u) {
      postRequest('remountFloppy', { unit: u });
      return 0;  // Fire and forget - result comes async
    },
    remountSMD: function(u) {
      postRequest('remountSMD', { unit: u });
      return 0;
    },
    unmountFloppy: function(u) { postCmd('unmountFloppy', { unit: u }); },
    unmountSMD:    function(u) { postCmd('unmountSMD', { unit: u }); },

    // --- FS (routed through Worker) ---
    fsWriteFile: function(p, d) {
      // Transfer ArrayBuffer to Worker
      var buffer = d.buffer ? d.buffer.slice(0) : d.slice(0);
      postTransfer('fsWrite', { path: p, buffer: buffer }, [buffer]);
    },
    fsReadFile: function(p) {
      return postRequest('fsRead', { path: p });
    },
    fsChmod: function(p, m) {
      // Chmod handled in Worker's fsWrite - no separate command needed
    },
    fsStat: function(p) {
      return postRequest('fsStat', { path: p });
    },
    fsUnlink: function(p) { postCmd('fsUnlink', { path: p }); },
    fsAvailable: function() { return true; },  // Worker has FS

    // --- HEAPU16 access ---
    getHEAPU16Buffer: function() { return null; },  // Not available in Worker mode
    hasHEAPU16: function() { return false; },

    // --- Module state queries ---
    isReady: function() { return _ready && _initialized; },
    hasFunction: function(fnName) { return false; },  // Module not on main thread
    callFunction: function(fnName) { return undefined; },

    // --- Drive info (unified registry query) ---
    getDriveInfo: function() {
      return postRequest('getDriveInfo', {});
    },

    // --- Mode flag ---
    isWorkerMode: function() { return true; },

    // --- Worker-specific methods ---
    workerStart: function() {
      postCmd('start');
    },
    workerStop: function() {
      postCmd('stop');
    },
    workerLoadDisk: function(path, arrayBuffer) {
      var buffer = arrayBuffer.slice(0);  // Copy for transfer
      postTransfer('loadDisk', { path: path, buffer: buffer }, [buffer]);
    },
    requestSnapshot: function() {
      postCmd('snapshot');
    },

    // --- OPFS persistent storage ---
    opfsMountSMD: function(unit, fileName) {
      return postRequest('opfsMountSMD', { unit: unit, fileName: fileName });
    },
    opfsUnmountSMD: function(unit) {
      return postRequest('opfsUnmountSMD', { unit: unit });
    },
    mountSMDFromBuffer: function(unit, data) {
      // Not used in Worker mode - Worker uses OPFS directly
      console.warn('mountSMDFromBuffer not applicable in Worker mode');
      return Promise.reject(new Error('Use opfsMountSMD in Worker mode'));
    },
    getSMDBuffer: function(unit) { return 0; },
    getSMDBufferSize: function(unit) { return 0; },

    // --- Gateway disk mount/unmount ---
    gatewayMountSMD: function(unit, imageSize) {
      return postRequest('gatewayMountSMD', { unit: unit, imageSize: imageSize });
    },
    gatewayUnmountSMD: function(unit) {
      return postRequest('gatewayUnmountSMD', { unit: unit });
    },
    gatewayMountFloppy: function(unit, imageSize) {
      return postRequest('gatewayMountFloppy', { unit: unit, imageSize: imageSize });
    },
    gatewayUnmountFloppy: function(unit) {
      return postRequest('gatewayUnmountFloppy', { unit: unit });
    },

    // --- WebSocket bridge ---
    wsConnect: function(url) { postCmd('ws-connect', { url: url }); },
    wsDisconnect: function() { postCmd('ws-disconnect'); },
    enableRemoteTerminals: function() {
      return postRequest('enableRemoteTerminals', {});
    },

    // --- Callback registration ---
    set onInitialized(fn)  { _onInitialized = fn; },
    get onInitialized()    { return _onInitialized; },
    set onBooted(fn)       { _onBooted = fn; },
    get onBooted()         { return _onBooted; },
    set onBreakpoint(fn)   { _onBreakpoint = fn; },
    get onBreakpoint()     { return _onBreakpoint; },
    set onStopped(fn)      { _onStopped = fn; },
    get onStopped()        { return _onStopped; },
    set onStepDone(fn)     { _onStepDone = fn; },
    get onStepDone()       { return _onStepDone; },
    set onRunDbgDone(fn)   { _onRunDbgDone = fn; },
    get onRunDbgDone()     { return _onRunDbgDone; }
  };

})();
