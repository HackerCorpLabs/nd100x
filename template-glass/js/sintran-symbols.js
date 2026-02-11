//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// sintran-symbols.js - Shared symbol table and memory read helpers for SINTRAN III
// Provides addresses, structure offsets, and utility functions used by all
// SINTRAN debug windows (processes, queues, segments, devices).

(function() {
  'use strict';

  // =========================================================
  // Fixed root pointers (identical across K03/L07/M06)
  // =========================================================
  var FIXED = {
    RTREF:  0x807,   // 004007 - current running process RT address
    CURPR:  0x808,   // 004010 - current process number
    MQUEU:  0x809,   // 004011 - monitor queue head (I/O datafield chain)
    BTIMQ:  0x80A,   // 004012 - time queue head
    BEXQU:  0x80B,   // 004013 - execution queue head
    SGMAX:  0x80D,   // 004015 - max segment number
    RTSTA:  0x810,   // 004020 - RT description table start
    SEGTB:  0x8D0,   // 004320 - segment table bank number
    SEGST:  0x8D1,   // 004321 - segment table offset within bank
    CORMB:  0x8D2,   // 004322 - core map bank number
    RTEND:  0x8D3,   // 004323 - RT description table end pointer
    CNVRT:  0x8D7    // 004327 - CCNVRT array (32 group offsets for LOGDBANK)
  };

  // =========================================================
  // Version-dependent symbols
  // =========================================================
  var VERSIONED = {
    K: { FBPR:0x6DA2, LTBP:0x77B0, LBPR:0x788C, STSIN:0x5F06,
         SBPRT:0x9AAF, DT01R:0x23DA, BPRTS:11,
         LGTFP:null, LGDGM:25 },       // K03: no LGTFPHPAGE symbol, max group 31oct=25dec
    L: { FBPR:0x26DF, LTBP:0x3145, LBPR:0x3221, STSIN:0x144F,
         SBPRT:0xBE73, DT01R:0x5787, BPRTS:12,
         LGTFP:0xF093, LGDGM:31 },     // L07: LGTFPHPAGE=170223oct, max group 37oct=31dec
    M: { FBPR:0x29CC, LTBP:0x3B12, LBPR:0x3BEE, STSIN:0x147C,
         SBPRT:0xBF08, DT01R:0x6013, BPRTS:12,
         LGTFP:0xF093, LGDGM:31 }      // M06: LGTFPHPAGE=170223oct, max group 37oct=31dec
  };

  // =========================================================
  // RT-Description field offsets (22 words, stable across versions)
  // =========================================================
  var RT_DESC = {
    SIZE:   22,
    TLINK:  0x00,   // Time queue link
    STATU:  0x01,   // Status flags
    INPRI:  0x02,   // Initial priority
    PRITY:  0x03,   // Priority / Type+Ring
    DTIM1:  0x04,   // Delay time high
    DTIM2:  0x05,   // Delay time low
    DTIN1:  0x06,   // DT interval high
    DTIN2:  0x07,   // DT interval low
    STADR:  0x08,   // Start address
    SEGM1:  0x09,   // Code segment
    SEGM2:  0x0A,   // Data segment
    WLINK:  0x0B,   // Exec/wait queue link
    ACT1S:  0x0C,   // Active segment 1
    ACT2S:  0x0D,   // Active segment 2
    INIPR:  0x0E,   // Initial priority register
    ACTPR:  0x0F,   // Active PCR value
    BRESL:  0x10,   // Reservation chain head
    RSEGM:  0x11,   // Reentrant segment
    BUFWI:  0x12,   // Buffer window page
    TRMWI:  0x13,   // Terminal window page
    N5WIN:  0x14,   // ND-500 window page
    RTDLG:  0x15    // Pointer to register save block
  };

  // =========================================================
  // STATU bit definitions
  // =========================================================
  var STATU_BITS = {
    BACK: 0,    // Background process
    USED: 1,    // Entry in use
    TSLI: 2,    // Time slice
    ESCF: 3,    // Escape flag
    BRKF: 4,    // Break flag
    SWWA: 8,    // Swap wait
    RTOF: 9,    // RT off
    TMOU: 10,   // Timeout
    INT:  12,   // Internal process
    RWAI: 13,   // Resource wait
    WAIT: 15    // I/O wait
  };

  // =========================================================
  // Segment table entry offsets (8 words per entry)
  // =========================================================
  var SEG_ENTRY = {
    SIZE:   8,
    SEGLI:  0,    // Segment link (next in chain)
    PRESE:  1,    // Previous segment
    LOGAD:  2,    // Logical address
    SEGLE:  3,    // Segment length (pages)
    MADR:   4,    // Memory address
    FLAG:   5,    // Flags
    SGSTA:  6,    // Segment status
    BPAGL:  7     // Base page logical
  };

  // =========================================================
  // I/O Datafield offsets
  // =========================================================
  var IO_DF = {
    RESLI:  0x00,   // Reservation link (next DF in chain)
    RTRES:  0x01,   // RT that reserved this DF
    BWLIN:  0x02,   // Buffer wait link
    TYPRI:  0x03,   // Device type + ring bits
    ISTAT:  0x04,   // I/O status word
    MLINK:  0x05,   // Monitor queue link
    MFUNC:  0x06,   // Monitor function code
    HSTAT:  0x08,   // Hardware status
    MTRAN:  0x09,   // Monitor transfer count
    MEMA1:  0x0D    // Memory address 1
  };

  // =========================================================
  // TYPRI bit definitions (device type word at IO_DF offset 003)
  // Tested in GDEVTY priority order: TERM, BAD, IBDV, FLOP, MT, RFIL
  // =========================================================
  var TYPRI_BITS = {
    CLDV:  2,    // Closable device
    NORE:  3,    // No reservation required
    BAD:   4,    // TAD (Terminal Adapter)
    TERM:  5,    // Terminal
    IBDV:  6,    // Indexed block device (disk)
    HDMA:  7,    // HDMA / X.21
    FLOP:  8,    // Floppy disk
    MT:    9,    // Magnetic tape
    M144: 10,    // 144-byte block format
    SPLI: 11,    // Split datafield (R/W halves)
    ISET: 12,    // I/O initialized
    CONC: 13,    // Concurrent I/O
    RFIL: 14,    // Remote file
    IOBT: 15     // Block transfer capable
  };

  // Legacy alias for backward compatibility
  var ISTAT_BITS = TYPRI_BITS;

  // =========================================================
  // Register save block offsets (via RTDLG pointer)
  // =========================================================
  var REG_SAVE = {
    P: 0, X: 1, T: 2, A: 3,
    D: 4, L: 5, S: 6, B: 7,
    BITMAP_START: 8,  // 8 words (128 bits) page bitmap
    BITMAP_END: 15
  };

  // =========================================================
  // Utility functions
  // =========================================================

  // Detect once whether Module.HEAPU16 is safely accessible.
  // Emscripten creates an abort-getter for non-exported items that poisons
  // the entire Module if triggered, so we must check without calling it.
  var heapU16Safe = false;
  function checkHeapU16() {
    if (heapU16Safe) return true;
    if (!Module) return false;
    var desc = Object.getOwnPropertyDescriptor(Module, 'HEAPU16');
    if (!desc) return false;
    if ('value' in desc) { heapU16Safe = !!desc.value; return heapU16Safe; }
    if (desc.get && desc.get.toString().indexOf('abort') === -1) {
      heapU16Safe = true;
      return true;
    }
    return false;
  }

  // Raw read: always goes through MemoryRead (standard PIT), no DPIT translation
  function readBlockRaw(addr, count) {
    if (!Module) return new Uint16Array(0);

    // Fast path: bulk read + HEAPU16 (only if safely available)
    if (Module._Dbg_ReadMemoryBlock && checkHeapU16()) {
      var ptr = Module._Dbg_ReadMemoryBlock(addr, count);
      var view = new Uint16Array(Module.HEAPU16.buffer, ptr, count);
      var copy = new Uint16Array(count);
      copy.set(view);
      return copy;
    }

    // Fallback: read word by word (always works)
    if (!Module._Dbg_ReadMemory) return new Uint16Array(0);
    var result = new Uint16Array(count);
    for (var i = 0; i < count; i++) {
      result[i] = Module._Dbg_ReadMemory(addr + i) & 0xFFFF;
    }
    return result;
  }

  // Raw read: single word through standard PIT
  function readWordRaw(addr) {
    if (!Module || !Module._Dbg_ReadMemory) return 0;
    return Module._Dbg_ReadMemory(addr) & 0xFFFF;
  }

  // Smart read: uses DPIT translation when SINTRAN is detected, otherwise raw
  function readBlock(addr, count) {
    if (typeof sintranState !== 'undefined' && sintranState.detected &&
        Module && Module._Dbg_GetPageTableEntryRaw) {
      return readBlockDPIT(addr, count);
    }
    return readBlockRaw(addr, count);
  }

  // Smart read: single word, uses DPIT when available
  function readWord(addr) {
    if (typeof sintranState !== 'undefined' && sintranState.detected &&
        Module && Module._Dbg_GetPageTableEntryRaw) {
      return readWordDPIT(addr);
    }
    return readWordRaw(addr);
  }

  // Get version-specific symbols based on detected SINTRAN version
  function getVersionSymbols() {
    if (!sintranState || !sintranState.versionLetter) return null;
    var letter = sintranState.versionLetter.toUpperCase();
    return VERSIONED[letter] || null;
  }

  // Decode ND-100 packed ASCII string (2 chars per word, strip parity bit 7)
  function decodeNDString(words, maxChars) {
    var result = '';
    var max = maxChars || words.length * 2;
    for (var i = 0; i < words.length && result.length < max; i++) {
      var w = words[i];
      var hi = (w >> 8) & 0x7F;
      var lo = w & 0x7F;
      if (hi >= 0x20 && hi <= 0x7E) result += String.fromCharCode(hi);
      else break;
      if (result.length < max) {
        if (lo >= 0x20 && lo <= 0x7E) result += String.fromCharCode(lo);
        else break;
      }
    }
    return result.replace(/\s+$/, '');
  }

  // Format a 16-bit value as 6-digit octal
  function toOctal(val) {
    return (val & 0xFFFF).toString(8).padStart(6, '0');
  }

  // Test if a specific bit is set in a 16-bit word
  function testBit(word, bit) {
    return (word & (1 << bit)) !== 0;
  }

  // =========================================================
  // Physical memory access (bypasses MMS, reads VolatileMemory directly)
  // =========================================================

  function readWordPhysical(physAddr) {
    if (!Module || !Module._Dbg_ReadPhysicalMemory) return 0;
    return Module._Dbg_ReadPhysicalMemory(physAddr) & 0xFFFF;
  }

  function readBlockPhysical(physAddr, count) {
    if (!Module) return new Uint16Array(0);

    if (Module._Dbg_ReadPhysicalMemoryBlock && checkHeapU16()) {
      var ptr = Module._Dbg_ReadPhysicalMemoryBlock(physAddr, count);
      var view = new Uint16Array(Module.HEAPU16.buffer, ptr, count);
      var copy = new Uint16Array(count);
      copy.set(view);
      return copy;
    }

    // Fallback: word by word
    if (!Module._Dbg_ReadPhysicalMemory) return new Uint16Array(0);
    var result = new Uint16Array(count);
    for (var i = 0; i < count; i++) {
      result[i] = Module._Dbg_ReadPhysicalMemory(physAddr + i) & 0xFFFF;
    }
    return result;
  }

  // =========================================================
  // DPIT (Data Page Table) translation
  // Kernel data structures are accessed through the Alternative PT.
  // We read DPIT entries via Dbg_GetPageTableEntryRaw, extract the PPN,
  // then do physical memory reads.
  // =========================================================

  // =========================================================
  // Page table translation cache
  // Keyed by PT number -> Array of 64 PPNs
  // =========================================================
  var ptCache = {};  // { ptNumber: [ppn0, ppn1, ..., ppn63] }

  // Build a translation map for any given page table: vpn -> ppn (64 entries)
  function buildPTMap(ptNumber) {
    if (!Module || !Module._Dbg_GetPageTableEntryRaw) return null;
    if (ptNumber < 0 || ptNumber >= 16) return null;
    var extended = Module._Dbg_GetExtendedMode ? Module._Dbg_GetExtendedMode() : 0;
    var map = new Array(64);
    for (var vpn = 0; vpn < 64; vpn++) {
      var pte = Module._Dbg_GetPageTableEntryRaw(ptNumber, vpn) >>> 0;
      if (extended) {
        map[vpn] = pte & 0x3FFF;   // 14-bit PPN
      } else {
        map[vpn] = pte & 0x01FF;   // 9-bit PPN
      }
    }
    return map;
  }

  // Get or build cached PT map for a given page table number
  function ensurePTCache(ptNumber) {
    if (!ptCache[ptNumber]) {
      ptCache[ptNumber] = buildPTMap(ptNumber);
    }
    return ptCache[ptNumber];
  }

  // Invalidate all PT caches
  function invalidatePTCache() {
    ptCache = {};
  }

  // Get the DPIT (Alternative Page Table) number.
  // DPIT is always PIT #7 in SINTRAN III - hardcoded because:
  // 1) Level 0 (idle/DUMMY) uses ADTPI (PIT#15), not DPIT
  // 2) All kernel data levels (1,2,4,5,10-16) use APIT = DPIT #7
  function getDPITNumber() {
    return 7;
  }

  // Translate a logical address through a specific page table to physical
  // Returns physical word address, or -1 if translation fails
  function translateViaPT(ptNumber, logicalAddr) {
    var map = ensurePTCache(ptNumber);
    if (!map) return -1;
    var vpn = (logicalAddr >>> 10) & 0x3F; // 6-bit VPN (addr / 1024)
    var offset = logicalAddr & 0x3FF;       // 10-bit offset within page
    var ppn = map[vpn];
    return ppn * 1024 + offset;
  }

  // Convenience: translate through the DPIT specifically
  function translateDPIT(logicalAddr) {
    var dpit = getDPITNumber();
    if (dpit < 0) return -1;
    return translateViaPT(dpit, logicalAddr);
  }

  // Read a single 16-bit word via a specific page table translation
  function readWordViaPT(ptNumber, logicalAddr) {
    var phys = translateViaPT(ptNumber, logicalAddr);
    if (phys < 0) return readWordRaw(logicalAddr); // fallback to raw
    return readWordPhysical(phys);
  }

  // Read a block of N consecutive words via a specific page table translation
  // Handles page boundary crossings correctly
  function readBlockViaPT(ptNumber, logicalAddr, count) {
    var map = ensurePTCache(ptNumber);
    if (!map) return readBlockRaw(logicalAddr, count); // fallback to raw

    var result = new Uint16Array(count);
    var i = 0;

    while (i < count) {
      var addr = (logicalAddr + i) & 0xFFFF;
      var vpn = (addr >>> 10) & 0x3F;
      var offset = addr & 0x3FF;
      var ppn = map[vpn];
      var physBase = ppn * 1024 + offset;

      // How many words until end of this page?
      var remaining = 1024 - offset;
      var chunk = Math.min(remaining, count - i);

      // Bulk read this page chunk from physical memory
      var data = readBlockPhysical(physBase, chunk);
      for (var j = 0; j < chunk; j++) {
        result[i + j] = data[j];
      }
      i += chunk;
    }

    return result;
  }

  // Convenience: DPIT-specific reads (uses kernel APT)
  function readWordDPIT(logicalAddr) {
    var dpit = getDPITNumber();
    if (dpit < 0) return readWordRaw(logicalAddr);
    return readWordViaPT(dpit, logicalAddr);
  }

  function readBlockDPIT(logicalAddr, count) {
    var dpit = getDPITNumber();
    if (dpit < 0) return readBlockRaw(logicalAddr, count);
    return readBlockViaPT(dpit, logicalAddr, count);
  }

  // =========================================================
  // RT table discovery (shared by all SINTRAN windows)
  // =========================================================

  var cachedRtInfo = null;

  function discoverRtTable() {
    // Return cached result if available (cleared on each refresh cycle)
    if (cachedRtInfo) return cachedRtInfo;

    // Simple approach matching Python reference: use RTSTA as base, RTEND as end
    var rtstaVal = readWord(FIXED.RTSTA);
    var rtendVal = readWord(FIXED.RTEND);

    if (rtstaVal === 0 && rtendVal === 0) {
      cachedRtInfo = { base: 0, count: 0 };
      return cachedRtInfo;
    }

    var tableBase = Math.min(rtstaVal, rtendVal);
    var tableEnd = Math.max(rtstaVal, rtendVal);
    if (tableBase === 0) tableBase = tableEnd;

    var rtCount = Math.floor((tableEnd - tableBase) / RT_DESC.SIZE);

    if (rtCount <= 0) rtCount = 1;
    if (rtCount > 512) rtCount = 512;

    cachedRtInfo = { base: tableBase, count: rtCount };
    return cachedRtInfo;
  }

  function invalidateRtCache() {
    cachedRtInfo = null;
    invalidatePTCache();
  }

  // Resolve an RT address to its RT number using discovered table base
  function rtAddrToNumber(rtAddr) {
    var info = discoverRtTable();
    if (info.base === 0 || rtAddr < info.base) return -1;
    var offset = rtAddr - info.base;
    if (offset % RT_DESC.SIZE !== 0) return -1;
    return Math.floor(offset / RT_DESC.SIZE);
  }

  // =========================================================
  // Export
  // =========================================================
  window.sintranSymbols = {
    FIXED: FIXED,
    VERSIONED: VERSIONED,
    RT_DESC: RT_DESC,
    STATU_BITS: STATU_BITS,
    SEG_ENTRY: SEG_ENTRY,
    IO_DF: IO_DF,
    TYPRI_BITS: TYPRI_BITS,
    ISTAT_BITS: ISTAT_BITS,
    REG_SAVE: REG_SAVE,
    readBlock: readBlock,
    readWord: readWord,
    readWordRaw: readWordRaw,
    readBlockRaw: readBlockRaw,
    readWordPhysical: readWordPhysical,
    readBlockPhysical: readBlockPhysical,
    readWordViaPT: readWordViaPT,
    readBlockViaPT: readBlockViaPT,
    readWordDPIT: readWordDPIT,
    readBlockDPIT: readBlockDPIT,
    translateViaPT: translateViaPT,
    translateDPIT: translateDPIT,
    getDPITNumber: getDPITNumber,
    ensurePTCache: ensurePTCache,
    invalidatePTCache: invalidatePTCache,
    getVersionSymbols: getVersionSymbols,
    decodeNDString: decodeNDString,
    toOctal: toOctal,
    testBit: testBit,
    discoverRtTable: discoverRtTable,
    invalidateRtCache: invalidateRtCache,
    rtAddrToNumber: rtAddrToNumber
  };
})();
