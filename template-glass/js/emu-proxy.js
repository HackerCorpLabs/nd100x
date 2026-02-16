//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// emu-proxy.js - Proxy abstraction layer for Module._* WASM calls
//
// All JS modules call emu.* instead of Module._* directly.
// In direct mode (default), each method delegates to Module._* synchronously.
// In worker mode (Phase 2), the proxy returns cached snapshot values.
//
// This file MUST load after module-init.js (Module config) and before nd100wasm.js.

(function() {
  'use strict';

  // =========================================================
  // Direct-mode proxy: every call passes through to Module._*
  // =========================================================
  window.emu = {

    // --- Lifecycle ---
    init:          function()  { return Module._Init(); },
    boot:          function(t) { return Module._Boot(t); },
    step:          function(n) { Module._Step(n); },
    stop:          function()  { Module._Stop(); },
    isInitialized: function()  { return Module._IsInitialized ? Module._IsInitialized() : 0; },

    // --- Terminal I/O ---
    sendKey:              function(id, k) { return Module._SendKeyToTerminal(id, k); },
    getTerminalAddress:   function(i)     { return Module._GetTerminalAddress(i); },
    getTerminalIdentCode: function(i)     { return Module._GetTerminalIdentCode(i); },
    getTerminalLogicalDevice: function(i) { return Module._GetTerminalLogicalDevice(i); },
    setTerminalCarrier:   function(f, id) { Module._SetTerminalCarrier(f, id); },

    // --- Terminal output handler setup ---
    hasJSTerminalHandler: function() {
      return typeof Module._TerminalOutputToJS !== 'undefined' &&
             typeof Module._SetJSTerminalOutputHandler !== 'undefined';
    },
    setJSTerminalOutputHandler: function(v) { Module._SetJSTerminalOutputHandler(v); },

    // --- Terminal ring buffer ---
    enableRingBuffer: function() {
      if (typeof Module._EnableTerminalRingBuffer === 'function') {
        Module._EnableTerminalRingBuffer(1);
        return true;
      }
      return false;
    },
    hasRingBuffer: function() {
      return typeof Module._PollTerminalOutput === 'function';
    },
    flushTerminalOutput: function() {
      if (typeof Module._PollTerminalOutput !== 'function') return;
      var handler = window.handleTerminalOutputFromC;
      if (!handler) return;
      var entry;
      while ((entry = Module._PollTerminalOutput()) >= 0) {
        handler((entry >> 8) & 0xFF, entry & 0xFF);
      }
    },

    // --- Legacy terminal callback registration ---
    hasAddFunction: function()  { return typeof Module.addFunction !== 'undefined'; },
    addFunction:    function(fn, sig) { return Module.addFunction(fn, sig); },
    hasSetTerminalOutputCallback: function() { return typeof Module._SetTerminalOutputCallback === 'function'; },
    setTerminalOutputCallback: function(id, cb) { Module._SetTerminalOutputCallback(id, cb); },

    // --- Registers (getters) ---
    getPC:    function() { return Module._Dbg_GetPC(); },
    getRegA:  function() { return Module._Dbg_GetRegA(); },
    getRegD:  function() { return Module._Dbg_GetRegD(); },
    getRegB:  function() { return Module._Dbg_GetRegB(); },
    getRegT:  function() { return Module._Dbg_GetRegT(); },
    getRegL:  function() { return Module._Dbg_GetRegL(); },
    getRegX:  function() { return Module._Dbg_GetRegX(); },
    getSTS:   function() { return Module._Dbg_GetSTS(); },
    getEA:    function() { return Module._Dbg_GetEA(); },
    getPIL:   function() { return Module._Dbg_GetPIL(); },

    // --- Registers (setters) ---
    setPC:    function(v) { Module._Dbg_SetPC(v); },
    setRegA:  function(v) { Module._Dbg_SetRegA(v); },
    setRegD:  function(v) { Module._Dbg_SetRegD(v); },
    setRegB:  function(v) { Module._Dbg_SetRegB(v); },
    setRegT:  function(v) { Module._Dbg_SetRegT(v); },
    setRegL:  function(v) { Module._Dbg_SetRegL(v); },
    setRegX:  function(v) { Module._Dbg_SetRegX(v); },
    setSTS:   function(v) { Module._Dbg_SetSTS(v); },

    // --- System registers (read-only) ---
    getPANS: function() { return Module._Dbg_GetPANS(); },
    getOPR:  function() { return Module._Dbg_GetOPR(); },
    getPGS:  function() { return Module._Dbg_GetPGS(); },
    getPVL:  function() { return Module._Dbg_GetPVL(); },
    getIIC:  function() { return Module._Dbg_GetIIC(); },
    getIID:  function() { return Module._Dbg_GetIID(); },
    getPID:  function() { return Module._Dbg_GetPID(); },
    getPIE:  function() { return Module._Dbg_GetPIE(); },
    getCSR:  function() { return Module._Dbg_GetCSR(); },
    getALD:  function() { return Module._Dbg_GetALD(); },
    getPES:  function() { return Module._Dbg_GetPES(); },
    getPGC:  function() { return Module._Dbg_GetPGC(); },
    getPEA:  function() { return Module._Dbg_GetPEA(); },

    // --- System registers (write-only / mixed) ---
    getPANC: function()  { return Module._Dbg_GetPANC(); },
    getLMP:  function()  { return Module._Dbg_GetLMP(); },
    getIIE:  function()  { return Module._Dbg_GetIIE(); },
    getCCL:  function()  { return Module._Dbg_GetCCL(); },
    getLCIL: function()  { return Module._Dbg_GetLCIL(); },
    getUCIL: function()  { return Module._Dbg_GetUCIL(); },
    getECCR: function()  { return Module._Dbg_GetECCR(); },
    getPCR:  function(l) { return Module._Dbg_GetPCR(l); },

    // --- Execution state ---
    getRunMode:    function()  { return Module._Dbg_GetRunMode(); },
    getStopReason: function()  { return Module._Dbg_GetStopReason(); },
    getInstrCount: function()  { return Module._Dbg_GetInstrCount(); },
    setPaused:     function(p) { Module._Dbg_SetPaused(p); },
    isPaused:      function()  { return Module._Dbg_IsPaused(); },

    // --- Step/Run ---
    stepOne:            function()  { Module._Dbg_StepOne(); },
    stepOver:           function()  { Module._Dbg_StepOver(); },
    stepOut:            function()  { Module._Dbg_StepOut(); },
    runWithBreakpoints: function(n) { return Module._Dbg_RunWithBreakpoints(n); },

    // --- Memory ---
    readMemory:      function(a)    { return Module._Dbg_ReadMemory(a); },
    writeMemory:     function(a, v) { Module._Dbg_WriteMemory(a, v); },
    readMemoryBlock: function(a, n) { return Module._Dbg_ReadMemoryBlock(a, n); },
    readPhysicalMemory:      function(a)    { return Module._Dbg_ReadPhysicalMemory(a); },
    readPhysicalMemoryBlock: function(a, n) { return Module._Dbg_ReadPhysicalMemoryBlock(a, n); },
    dumpPhysicalMemory:      function(n)    { return Module._Dbg_DumpPhysicalMemory(n); },

    // --- Breakpoints ---
    addBreakpoint:     function(a)    { Module._Dbg_AddBreakpoint(a); },
    removeBreakpoint:  function(a)    { Module._Dbg_RemoveBreakpoint(a); },
    clearBreakpoints:  function()     { Module._Dbg_ClearBreakpoints(); },
    getBreakpointList: function()     { return Module.UTF8ToString(Module._Dbg_GetBreakpointList()); },

    // --- Watchpoints ---
    addWatchpoint:      function(a, t) { Module._Dbg_AddWatchpoint(a, t); },
    removeWatchpoint:   function(a)    { Module._Dbg_RemoveWatchpoint(a); },
    clearWatchpoints:   function()     { Module._Dbg_ClearWatchpoints(); },
    getWatchpointCount: function()     { return Module._Dbg_GetWatchpointCount(); },
    getWatchpointAddr:  function(i)    { return Module._Dbg_GetWatchpointAddr(i); },
    getWatchpointType:  function(i)    { return Module._Dbg_GetWatchpointType(i); },

    // --- Disassembly / Level info ---
    disassemble:  function(a, n) { return Module.UTF8ToString(Module._Dbg_Disassemble(a, n)); },
    getLevelInfo:  function()     { return Module.UTF8ToString(Module._Dbg_GetLevelInfo()); },

    // --- DAP (JSON via ccall) ---
    ccall: function(name, retType, argTypes, argValues) {
      return Module.ccall(name, retType, argTypes, argValues);
    },

    // --- Page tables ---
    getPageTableCount:    function()     { return Module._Dbg_GetPageTableCount(); },
    getPageTableEntryRaw: function(p, v) { return Module._Dbg_GetPageTableEntryRaw(p, v); },
    getPageTableMap: function(pt) {
      var entries = new Array(64);
      for (var i = 0; i < 64; i++) {
        entries[i] = Module._Dbg_GetPageTableEntryRaw(pt, i) >>> 0;
      }
      return entries;
    },
    getExtendedMode:      function()     { return Module._Dbg_GetExtendedMode(); },

    // --- Floppy/SMD ---
    remountFloppy:  function(u) { return Module._RemountFloppy(u); },
    remountSMD:     function(u) { return Module._RemountSMD(u); },
    unmountFloppy:  function(u) { Module._UnmountFloppy(u); },
    unmountSMD:     function(u) { Module._UnmountSMD(u); },

    // --- FS (pass-through in direct mode) ---
    fsWriteFile: function(p, d) { Module.FS.writeFile(p, d); },
    fsReadFile:  function(p)    { return Module.FS.readFile(p); },
    fsChmod:     function(p, m) { Module.FS.chmod(p, m); },
    fsStat:      function(p)    { return Module.FS.stat(p); },
    fsUnlink:    function(p)    { Module.FS.unlink(p); },
    fsAvailable: function()     { return typeof Module.FS !== 'undefined'; },

    // --- HEAPU16 access for bulk memory reads ---
    getHEAPU16Buffer: function() { return Module.HEAPU16 ? Module.HEAPU16.buffer : null; },
    hasHEAPU16: function() {
      var desc = Object.getOwnPropertyDescriptor(Module, 'HEAPU16');
      if (!desc) return false;
      if ('value' in desc) return !!desc.value;
      if (desc.get && desc.get.toString().indexOf('abort') === -1) return true;
      return false;
    },

    // --- Module state queries ---
    isReady: function() {
      return Module && Module.calledRun && Module._Dbg_GetPC;
    },
    hasFunction: function(fnName) {
      return typeof Module[fnName] === 'function';
    },
    callFunction: function(fnName) {
      if (typeof Module[fnName] === 'function') {
        return Module[fnName].apply(Module, Array.prototype.slice.call(arguments, 1));
      }
      return undefined;
    },

    // --- OPFS persistent storage (Direct mode: buffer-based) ---
    opfsMountSMD: function(unit, fileName) {
      // In Direct mode, we cannot use SyncAccessHandle. Reject so caller falls back.
      return Promise.reject(new Error('OPFS SyncAccessHandle not available in Direct mode'));
    },
    opfsUnmountSMD: function(unit) {
      Module._UnmountSMD(unit);
    },
    mountSMDFromBuffer: function(unit, data) {
      // Copy data into WASM heap and call MountSMDFromBuffer
      var ptr = Module._malloc(data.byteLength);
      Module.HEAPU8.set(data instanceof Uint8Array ? data : new Uint8Array(data), ptr);
      var rc = Module._MountSMDFromBuffer(unit, ptr, data.byteLength);
      // Do NOT free ptr - MountSMDFromBuffer copies into its own malloc'd buffer,
      // but the C function already copied so we free the temp copy
      Module._free(ptr);
      return rc;
    },
    getSMDBuffer: function(unit) { return Module._GetSMDBuffer(unit); },
    getSMDBufferSize: function(unit) { return Module._GetSMDBufferSize(unit); },

    // --- WebSocket bridge (requires Worker mode) ---
    wsConnect: function(url) { console.warn('WebSocket bridge requires Worker mode'); },
    wsDisconnect: function() { },
    enableRemoteTerminals: function() {
      console.warn('WebSocket bridge requires Worker mode');
      return Promise.resolve({ count: 0 });
    },

    // --- Mode flag ---
    isWorkerMode: function() { return false; }
  };

})();
