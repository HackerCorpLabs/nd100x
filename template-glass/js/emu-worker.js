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
// Gateway WebSocket statistics
// =========================================================
var _wsStats = {
  connectedSince: 0,
  termIn:    { frames: 0, bytes: 0 },
  termOut:   { frames: 0, bytes: 0 },
  diskRead:  { ops: 0, bytes: 0 },
  diskWrite: { ops: 0, bytes: 0 },
  hdlcRx:    { frames: 0, bytes: 0 },
  hdlcTx:    { frames: 0, bytes: 0 },
  clientConnects: 0,
  clientDisconnects: 0
};
var _wsStatsTimer = null;

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
// OPFS SyncAccessHandle pool for persistent SMD block I/O
// =========================================================
var _opfsHandles = [null, null, null, null];  // SyncAccessHandle per SMD unit
var _opfsReady = [false, false, false, false];

// Open OPFS file for a unit using SyncAccessHandle (synchronous I/O in Worker)
// Always closes any existing handle on this unit first to prevent leaks.
async function opfsOpenUnit(unit, fileName) {
  if (unit < 0 || unit > 3) return false;

  // Close existing handle first (prevents SyncAccessHandle leaks)
  opfsCloseUnit(unit);

  try {
    var root = await navigator.storage.getDirectory();
    var dir = await root.getDirectoryHandle('smd-images', { create: false });
    var fileHandle = await dir.getFileHandle(fileName);
    var accessHandle = await fileHandle.createSyncAccessHandle();
    _opfsHandles[unit] = accessHandle;
    _opfsReady[unit] = true;
    postMessage({ type: 'log', level: 'info', text: '[OPFS] Unit ' + unit + ' opened: ' + fileName + ' (' + accessHandle.getSize() + ' bytes)' });
    return true;
  } catch (e) {
    postMessage({ type: 'log', level: 'error', text: '[OPFS] Failed to open unit ' + unit + ': ' + e.message });
    _opfsHandles[unit] = null;
    _opfsReady[unit] = false;
    return false;
  }
}

function opfsCloseUnit(unit) {
  if (unit < 0 || unit > 3) return;
  if (_opfsHandles[unit]) {
    try {
      _opfsHandles[unit].flush();
      _opfsHandles[unit].close();
    } catch (e) {}
    _opfsHandles[unit] = null;
    _opfsReady[unit] = false;
  }
}

// Synchronous block read from OPFS (called from C via EM_JS -> opfsBlockRead)
function opfsBlockRead(unit, wasmPtr, bytes, offset) {
  if (!_opfsReady[unit] || !_opfsHandles[unit]) return -1;
  var dest = new Uint8Array(Module.HEAPU8.buffer, wasmPtr, bytes);
  var read = _opfsHandles[unit].read(dest, { at: offset });
  return read;
}

// Synchronous block write to OPFS (called from C via EM_JS -> opfsBlockWrite)
function opfsBlockWrite(unit, wasmPtr, bytes, offset) {
  if (!_opfsReady[unit] || !_opfsHandles[unit]) return -1;
  var src = new Uint8Array(Module.HEAPU8.buffer, wasmPtr, bytes);
  var written = _opfsHandles[unit].write(src, { at: offset });
  return written;
}

// Check if OPFS is available for a unit
function opfsIsAvailable(unit) {
  return _opfsReady[unit] ? 1 : 0;
}

// =========================================================
// Gateway disk I/O sub-worker (SharedArrayBuffer + Atomics)
// =========================================================
var _diskWorker = null;
var _diskControlArray = null;  // Int32Array view (8 x Int32)
var _diskDataArray = null;     // Uint8Array view at offset 32
var _diskSharedBuffer = null;
var _diskReady = false;
var _diskGatewayDrives = { smd: {}, floppy: {} };  // [type][unit] -> true

var DRIVE_TYPE_NAMES = ['smd', 'floppy'];
var _fullReadRequestId = null;  // pending gatewayReadFullImage request id

// Block read cache: key = "driveType-unit-offset-bytes" -> Uint8Array
var _diskBlockCache = new Map();
var _diskReadCount = 0;   // total gateway reads (cache misses)
var _diskWriteCount = 0;  // total gateway writes

function initDiskWorker(gatewayUrl) {
  // Close existing sub-worker before creating a new one
  closeDiskWorker();

  var hasSAB = (typeof SharedArrayBuffer !== 'undefined');
  var initMsg = { type: 'init', wsUrl: gatewayUrl };

  if (hasSAB) {
    _diskSharedBuffer = new SharedArrayBuffer(32 + 65536);  // 32 bytes control + 64KB data (SMD reads up to 64KB)
    _diskControlArray = new Int32Array(_diskSharedBuffer, 0, 8);
    _diskDataArray = new Uint8Array(_diskSharedBuffer, 32);
    Atomics.store(_diskControlArray, 0, 0);  // idle
    initMsg.sharedBuffer = _diskSharedBuffer;
  } else {
    postMessage({ type: 'log', level: 'warn',
      text: '[DiskIO] SharedArrayBuffer not available - disk listing only (no block I/O). Serve page via gateway for full support.' });
  }

  _diskWorker = new Worker('disk-io-worker.js');
  _diskWorker.postMessage(initMsg);

  _diskWorker.onmessage = function(e) {
    if (e.data.type === 'connected') {
      _diskReady = true;
      postMessage({ type: 'disk-connected', blockIO: hasSAB });
    }
    if (e.data.type === 'disconnected') {
      _diskReady = false;
      postMessage({ type: 'disk-disconnected' });
    }
    if (e.data.type === 'disk-list') {
      postMessage({ type: 'disk-list', data: e.data.data });
    }
    if (e.data.type === 'error') {
      postMessage({ type: 'log', level: 'error', text: '[DiskIO] ' + e.data.error });
    }
    if (e.data.type === 'full-image-progress') {
      postMessage({ type: 'full-image-progress', id: _fullReadRequestId,
                    done: e.data.done, total: e.data.total });
    }
    if (e.data.type === 'full-image-data') {
      if (e.data.error) {
        postMessage({ type: 'full-image-data', id: _fullReadRequestId, error: e.data.error });
      } else {
        var buf = e.data.buffer;
        postMessage({ type: 'full-image-data', id: _fullReadRequestId,
                      driveType: e.data.driveType, unit: e.data.unit,
                      buffer: buf }, [buf]);
      }
      _fullReadRequestId = null;
    }
  };
}

function closeDiskWorker() {
  if (_diskWorker) {
    _diskWorker.postMessage({ type: 'close' });
    _diskWorker = null;
  }
  _diskReady = false;
  _diskGatewayDrives = { smd: {}, floppy: {} };
  _diskBlockCache.clear();
}

// Synchronous block read from gateway (called from C via EM_JS -> gatewayBlockRead)
function gatewayBlockRead(driveType, unit, wasmPtr, bytes, offset) {
  if (!_diskReady || !_diskControlArray) return -1;
  var typeName = DRIVE_TYPE_NAMES[driveType];
  if (!typeName || !_diskGatewayDrives[typeName][unit]) return -1;

  // Check block cache first
  var cacheKey = driveType + '-' + unit + '-' + offset + '-' + bytes;
  var cached = _diskBlockCache.get(cacheKey);
  if (cached) {
    Module.HEAPU8.set(cached, wasmPtr);
    _wsStats.diskRead.ops++;
    _wsStats.diskRead.bytes += cached.length;
    return cached.length;
  }

  // Cache miss - fetch from gateway via sub-worker
  _diskControlArray[1] = driveType;
  _diskControlArray[2] = unit;
  _diskControlArray[3] = offset;
  _diskControlArray[4] = bytes;
  _diskControlArray[7] = 0;  // read

  // Signal request and wait for response
  Atomics.store(_diskControlArray, 0, 1);
  Atomics.notify(_diskControlArray, 0);
  // Poll with short timeouts until sub-worker signals response ready
  var _rwc = 0;
  while (Atomics.load(_diskControlArray, 0) === 1) {
    Atomics.wait(_diskControlArray, 0, 1, 1);  // 1ms timeout
    if (++_rwc > 30000) {  // 30 second safety bail-out
      console.warn('[GW-READ] Timeout waiting for response');
      Atomics.store(_diskControlArray, 0, 0);
      return -1;
    }
  }

  // Read response
  var status = _diskControlArray[5];
  var dataLen = _diskControlArray[6];
  Atomics.store(_diskControlArray, 0, 0);  // reset to idle
  Atomics.notify(_diskControlArray, 0);    // wake sub-worker

  if (status !== 0 || dataLen <= 0) return -1;

  // Cache the block and copy to WASM heap
  var blockCopy = new Uint8Array(dataLen);
  blockCopy.set(_diskDataArray.subarray(0, dataLen));
  _diskBlockCache.set(cacheKey, blockCopy);

  Module.HEAPU8.set(blockCopy, wasmPtr);
  _wsStats.diskRead.ops++;
  _wsStats.diskRead.bytes += dataLen;
  _diskReadCount++;
  if (_diskReadCount === 1) {
    console.log('[Gateway I/O] First disk read: ' + DRIVE_TYPE_NAMES[driveType] + ' unit ' + unit + ' offset ' + offset + ' (' + dataLen + ' bytes)');
  } else if (_diskReadCount % 100 === 0) {
    console.log('[Gateway I/O] Disk reads: ' + _diskReadCount + ' blocks (' + (_wsStats.diskRead.bytes / 1024).toFixed(0) + ' KB), cache: ' + _diskBlockCache.size + ' entries');
  }
  return dataLen;
}

// Synchronous block write to gateway (called from C via EM_JS -> gatewayBlockWrite)
function gatewayBlockWrite(driveType, unit, wasmPtr, bytes, offset) {
  if (!_diskReady || !_diskControlArray) return -1;
  var typeName = DRIVE_TYPE_NAMES[driveType];
  if (!typeName || !_diskGatewayDrives[typeName][unit]) return -1;

  // Copy write data to shared buffer
  _diskDataArray.set(Module.HEAPU8.subarray(wasmPtr, wasmPtr + bytes), 0);

  // Write request parameters
  _diskControlArray[1] = driveType;
  _diskControlArray[2] = unit;
  _diskControlArray[3] = offset;
  _diskControlArray[4] = bytes;
  _diskControlArray[7] = 1;  // write

  // Signal request and wait for response
  Atomics.store(_diskControlArray, 0, 1);
  Atomics.notify(_diskControlArray, 0);
  // Poll with short timeouts - Atomics.notify from sub-worker may not wake Atomics.wait
  var _wwc = 0;
  while (Atomics.load(_diskControlArray, 0) === 1) {
    Atomics.wait(_diskControlArray, 0, 1, 1);  // 1ms timeout
    if (++_wwc > 30000) {  // 30 second safety bail-out
      console.warn('[GW-WRITE] Timeout waiting for response');
      Atomics.store(_diskControlArray, 0, 0);
      return -1;
    }
  }

  var status = _diskControlArray[5];
  Atomics.store(_diskControlArray, 0, 0);
  Atomics.notify(_diskControlArray, 0);
  if (status === 0) {
    // Update block cache with written data
    var cacheKey = driveType + '-' + unit + '-' + offset + '-' + bytes;
    var blockCopy = new Uint8Array(bytes);
    blockCopy.set(Module.HEAPU8.subarray(wasmPtr, wasmPtr + bytes));
    _diskBlockCache.set(cacheKey, blockCopy);

    _wsStats.diskWrite.ops++;
    _wsStats.diskWrite.bytes += bytes;
    _diskWriteCount++;
    if (_diskWriteCount === 1) {
      console.log('[Gateway I/O] First disk write: ' + DRIVE_TYPE_NAMES[driveType] + ' unit ' + unit + ' offset ' + offset + ' (' + bytes + ' bytes)');
    } else if (_diskWriteCount % 100 === 0) {
      console.log('[Gateway I/O] Disk writes: ' + _diskWriteCount + ' blocks (' + (_wsStats.diskWrite.bytes / 1024).toFixed(0) + ' KB)');
    }
  }
  return status === 0 ? bytes : -1;
}

// Check if gateway disk is available for a unit
function gatewayIsAvailable(driveType, unit) {
  var typeName = DRIVE_TYPE_NAMES[driveType];
  return (_diskReady && typeName && _diskGatewayDrives[typeName][unit]) ? 1 : 0;
}


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
    // Reset stats on new connection
    _wsStats.connectedSince = Date.now();
    _wsStats.termIn.frames = 0;    _wsStats.termIn.bytes = 0;
    _wsStats.termOut.frames = 0;   _wsStats.termOut.bytes = 0;
    _wsStats.diskRead.ops = 0;     _wsStats.diskRead.bytes = 0;
    _wsStats.diskWrite.ops = 0;    _wsStats.diskWrite.bytes = 0;
    _wsStats.hdlcRx.frames = 0;   _wsStats.hdlcRx.bytes = 0;
    _wsStats.hdlcTx.frames = 0;   _wsStats.hdlcTx.bytes = 0;
    _wsStats.clientConnects = 0;
    _wsStats.clientDisconnects = 0;
    // Start periodic stats posting (~1/s)
    if (_wsStatsTimer) clearInterval(_wsStatsTimer);
    _wsStatsTimer = setInterval(function() {
      postMessage({ type: 'ws-stats', stats: _wsStats });
    }, 1000);
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
        _wsStats.termIn.frames++;
        _wsStats.termIn.bytes += buf.length;
        var identCode = buf[1];
        for (var i = 2; i < buf.length; i++) {
          Module._SendKeyToTerminal(identCode, buf[i]);
        }
      }
      else if (buf[0] === 0x10 && buf.length >= 4) {
        // HDLC RX frame: [0x10][channel][lenHi][lenLo][data...]
        _wsStats.hdlcRx.frames++;
        _wsStats.hdlcRx.bytes += buf.length;
        var channel = buf[1];
        var len = (buf[2] << 8) | buf[3];
        if (typeof Module._HDLC_InjectRxFrame === 'function' && len > 0) {
          var frameData = buf.subarray(4, 4 + len);
          var ptr = Module._malloc(len);
          Module.HEAPU8.set(frameData, ptr);
          Module._HDLC_InjectRxFrame(channel, ptr, len);
          Module._free(ptr);
        }
      }
      else if (buf[0] === 0x12 && buf.length >= 3) {
        // HDLC carrier status: [0x12][channel][present]
        if (typeof Module._HDLC_SetCarrier === 'function') {
          Module._HDLC_SetCarrier(buf[1], buf[2] & 0x01);
        }
      }
      return;
    }

    // JSON text frame (control messages only)
    var msg;
    try { msg = JSON.parse(ev.data); } catch(e) { return; }

    switch (msg.type) {
      case 'client-connected': {
        _wsStats.clientConnects++;
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
        _wsStats.clientDisconnects++;
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
    _wsStats.connectedSince = 0;
    if (_wsStatsTimer) { clearInterval(_wsStatsTimer); _wsStatsTimer = null; }
    postMessage({ type: 'ws-stats', stats: _wsStats });
    postMessage({ type: 'ws-status', connected: false, error: null });
    // Drop carrier on all active remote terminals. The gateway WebSocket is
    // gone, so any TCP clients it was serving are now disconnected. Signal
    // carrier missing for each so SINTRAN can clean up the sessions.
    if (Module && Module._SetTerminalCarrier) {
      for (var ic in _remoteIdentCodes) {
        Module._SetTerminalCarrier(1, parseInt(ic, 10));
      }
    }
    _ws = null;
    // Auto-reconnect after 3 seconds if we had a URL
    if (_wsUrl) {
      _wsReconnectTimer = setTimeout(function() {
        wsConnect(_wsUrl);
        // Re-init disk sub-worker so it reconnects and sends fresh disk list
        initDiskWorker(_wsUrl);
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
  if (_wsStatsTimer) { clearInterval(_wsStatsTimer); _wsStatsTimer = null; }
  if (_ws) {
    try { _ws.close(); } catch(e) {}
    _ws = null;
  }
  _remoteIdentCodes = {};
  _remoteTerminals = [];
  _wsStats.connectedSince = 0;
  postMessage({ type: 'ws-stats', stats: _wsStats });
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
      _wsStats.termOut.frames++;
      _wsStats.termOut.bytes += frame.length;
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
      _wsStats.termOut.frames++;
      _wsStats.termOut.bytes += frame.length;
    }

    // Poll HDLC TX frames and send over WebSocket
    if (typeof Module._HDLC_PollTxFrame === 'function') {
      while (Module._HDLC_PollTxFrame() > 0) {
        var txLen = Module._HDLC_GetLastTxLength();
        var txPtr = Module._HDLC_GetLastTxBuffer();
        var txChan = Module._HDLC_GetLastTxChannel();
        if (txLen > 0) {
          var txFrame = new Uint8Array(4 + txLen);
          txFrame[0] = 0x11;  // HDLC TX
          txFrame[1] = txChan;
          txFrame[2] = (txLen >> 8) & 0xFF;
          txFrame[3] = txLen & 0xFF;
          txFrame.set(Module.HEAPU8.subarray(txPtr, txPtr + txLen), 4);
          _ws.send(txFrame.buffer);
          _wsStats.hdlcTx.frames++;
          _wsStats.hdlcTx.bytes += txFrame.length;
        }
      }
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

      // After Init creates terminal devices, if WS is already connected
      // (auto-connect may have fired before Init), re-discover and register
      // remote terminals with the gateway.
      if (_ws && _ws.readyState === 1) {
        var rtRes = Module._EnableRemoteTerminals();
        _remoteTerminals = [];
        _remoteIdentCodes = {};
        for (var rti2 = 8; rti2 < 16; rti2++) {
          var rtId2 = Module._GetTerminalIdentCode(rti2);
          if (rtId2 !== -1) {
            var rtNm2 = '';
            try {
              var np2 = Module._GetTerminalName(rti2);
              if (np2) rtNm2 = Module.UTF8ToString(np2);
            } catch(e2) {}
            _remoteTerminals.push({
              identCode: rtId2,
              name: rtNm2 || ('Terminal ' + rti2),
              logicalDevice: Module._GetTerminalLogicalDevice(rti2)
            });
            _remoteIdentCodes[rtId2] = true;
          }
        }
        if (_remoteTerminals.length > 0) {
          _ws.send(JSON.stringify({ type: 'register', terminals: _remoteTerminals }));
          console.log('[Worker] Post-Init: re-registered ' + _remoteTerminals.length + ' remote terminals with gateway');
        }
      }
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

    // --- OPFS persistent storage ---
    case 'opfsMountSMD': {
      // Open OPFS SyncAccessHandle and mount in C as OPFS drive
      postMessage({ type: 'log', level: 'info', text: '[Worker] opfsMountSMD unit=' + msg.unit + ' file=' + msg.fileName });
      opfsOpenUnit(msg.unit, msg.fileName).then(function(ok) {
        if (ok) {
          var size = _opfsHandles[msg.unit].getSize();
          Module._MountSMDFromOPFS(msg.unit, size);
          postMessage({ type: 'log', level: 'info', text: '[Worker] MountSMDFromOPFS unit=' + msg.unit + ' size=' + size + ' OK' });
          postMessage({ type: 'opfsMountResult', id: msg.id, unit: msg.unit, ok: true, size: size });
        } else {
          postMessage({ type: 'log', level: 'error', text: '[Worker] opfsMountSMD FAILED unit=' + msg.unit });
          postMessage({ type: 'opfsMountResult', id: msg.id, unit: msg.unit, ok: false, size: 0 });
        }
      });
      break;
    }

    case 'opfsUnmountSMD': {
      postMessage({ type: 'log', level: 'info', text: '[Worker] opfsUnmountSMD unit=' + msg.unit });
      opfsCloseUnit(msg.unit);
      Module._UnmountSMD(msg.unit);
      postMessage({ type: 'fsResult', id: msg.id, result: 0 });
      break;
    }

    // --- WebSocket bridge ---
    case 'ws-connect': {
      // Connect to gateway WebSocket (single endpoint for all traffic)
      wsConnect(msg.url);
      // Also init disk I/O sub-worker (connects to same gateway)
      initDiskWorker(msg.url);
      break;
    }

    case 'ws-disconnect': {
      wsDisconnect();
      closeDiskWorker();
      break;
    }

    // --- Gateway disk mount/unmount ---
    case 'gatewayMountSMD': {
      _diskGatewayDrives.smd[msg.unit] = true;
      var gmsResult = Module._MountSMDFromGateway(msg.unit, msg.imageSize);
      postMessage({ type: 'gatewayMountResult', id: msg.id, unit: msg.unit, driveType: 'smd', ok: gmsResult === 0 });
      break;
    }

    case 'gatewayUnmountSMD': {
      delete _diskGatewayDrives.smd[msg.unit];
      Module._UnmountSMD(msg.unit);
      postMessage({ type: 'fsResult', id: msg.id, result: 0 });
      break;
    }

    case 'gatewayMountFloppy': {
      _diskGatewayDrives.floppy[msg.unit] = true;
      var gmfResult = Module._MountFloppyFromGateway(msg.unit, msg.imageSize);
      postMessage({ type: 'gatewayMountResult', id: msg.id, unit: msg.unit, driveType: 'floppy', ok: gmfResult === 0 });
      break;
    }

    case 'gatewayUnmountFloppy': {
      delete _diskGatewayDrives.floppy[msg.unit];
      Module._UnmountFloppy(msg.unit);
      postMessage({ type: 'fsResult', id: msg.id, result: 0 });
      break;
    }

    case 'getDriveInfo': {
      var driveJson = Module.UTF8ToString(Module._GetDriveInfo());
      var driveData;
      try { driveData = JSON.parse(driveJson); } catch(e) { driveData = []; }
      postMessage({ type: 'getDriveInfoResult', id: msg.id, data: driveData });
      break;
    }

    case 'gatewayReadFullImage': {
      if (!_diskWorker || !_diskReady) {
        postMessage({ type: 'full-image-data', id: msg.id, error: 'Disk worker not connected' });
        break;
      }
      _fullReadRequestId = msg.id;
      _diskWorker.postMessage({ type: 'read-full', driveType: msg.driveType,
                                unit: msg.unit, size: msg.size });
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
      // Send register message to gateway if WebSocket is connected
      if (_ws && _ws.readyState === 1 && _remoteTerminals.length > 0) {
        _ws.send(JSON.stringify({ type: 'register', terminals: _remoteTerminals }));
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
