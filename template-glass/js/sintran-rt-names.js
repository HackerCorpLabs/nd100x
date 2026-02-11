//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// sintran-rt-names.js - RT program name lookup tables
// Source: SINTRAN III NPL symbol tables (SYMBOL-2-LIST.SYMB.TXT)
// Verified against: K03, L07, M06 symbol tables and live SINTRAN L07 execution
//
// RT program names are NOT stored as strings in memory.
// Names exist only in the linker symbol tables. This module provides
// the reverse lookup: RT-Description address -> symbol name.
//
// Background programs (BAKnn/BCHnn) are named at runtime based on terminal number.

(function() {
  'use strict';

  // =========================================================
  // Address-to-name lookup tables (keyed by RT slot address)
  // Addresses are 16-bit DPIT logical addresses.
  // =========================================================

  var L07 = {};
  L07[0o012071] = 'DUMMY';   L07[0o012117] = 'STSIN';   L07[0o012145] = 'RTERR';
  L07[0o012173] = '1SWAP';   L07[0o012221] = 'TIMRT';   L07[0o012247] = 'RTDIL';
  L07[0o012275] = 'DIMWD';   L07[0o012323] = 'BPTMP';   L07[0o012351] = 'RTSLI';
  L07[0o012377] = 'ACCRT';   L07[0o012425] = 'TERMP';   L07[0o012453] = '5SWAP';
  L07[0o012501] = 'RWRT1';   L07[0o012527] = 'RWRT2';   L07[0o012555] = 'RWRT3';
  L07[0o012603] = 'RWRT5';   L07[0o012631] = 'RWRT7';   L07[0o012657] = 'RWRT8';
  L07[0o012705] = 'RWRT9';   L07[0o012733] = 'RTRFA';   L07[0o012761] = 'DUMM2';
  L07[0o013007] = 'SPRT1';   L07[0o013035] = 'SPRT2';   L07[0o013063] = 'SPRT3';
  L07[0o013111] = 'SPRT4';   L07[0o013137] = 'SPRT5';   L07[0o013165] = 'SPRT6';
  L07[0o013213] = 'SPRT7';   L07[0o013241] = 'SPRT8';   L07[0o013267] = 'SPRT9';
  L07[0o013315] = 'SPR10';   L07[0o013343] = 'SPR11';   L07[0o013371] = 'SPR12';
  L07[0o013417] = 'SPR13';   L07[0o013445] = 'SPR14';   L07[0o013473] = 'SPR15';
  L07[0o013521] = 'SPR16';   L07[0o013547] = 'COSPO';
  L07[0o013575] = 'RWR10';   L07[0o013623] = 'RWR11';   L07[0o013651] = 'RWR12';
  L07[0o013677] = 'RWR13';   L07[0o013725] = 'RWR20';   L07[0o013753] = 'RWR14';
  L07[0o014001] = 'RWR21';   L07[0o014027] = 'RWR25';   L07[0o014055] = 'RWR26';
  L07[0o014103] = 'RWR41';   L07[0o014131] = 'RWR42';
  L07[0o014157] = 'TADAD';
  L07[0o014205] = 'UDR01';   L07[0o014233] = 'UDR02';   L07[0o014261] = 'UDR03';
  L07[0o014307] = 'UDR04';   L07[0o014335] = 'UDR05';   L07[0o014363] = 'UDR06';
  L07[0o014411] = 'XROUT';   L07[0o014437] = 'XTRAC';   L07[0o014465] = 'XMFID';
  L07[0o014513] = 'NKSER';   L07[0o014541] = 'NKNAM';   L07[0o014567] = 'ERSWD';
  L07[0o014615] = 'PROMA';   L07[0o014643] = 'EVMES';   L07[0o014671] = 'BOPCO';
  L07[0o014717] = 'MTSER';   L07[0o014745] = 'RTREC';   L07[0o014773] = 'RTBES';

  var K03 = {};
  K03[0o057360] = 'DUMMY';   K03[0o057406] = 'STSIN';   K03[0o057434] = 'RTERR';
  K03[0o057462] = '1SWAP';   K03[0o057510] = 'TIMRT';   K03[0o057536] = 'RTDIL';
  K03[0o057564] = 'BPTMP';   K03[0o057612] = 'RTSLI';   K03[0o057640] = 'ACCRT';
  K03[0o057666] = 'TERMP';   K03[0o057714] = '5SWAP';
  K03[0o057742] = 'RWRT1';   K03[0o057770] = 'RWRT2';   K03[0o060016] = 'RWRT3';
  K03[0o060044] = 'RWRT5';   K03[0o060072] = 'RWRT7';   K03[0o060120] = 'RWRT8';
  K03[0o060146] = 'FDRT1';   K03[0o060174] = 'RWRT9';   K03[0o060222] = 'RTRFA';
  K03[0o060250] = 'DUMM2';
  K03[0o060276] = 'SPRT1';   K03[0o060324] = 'SPRT2';   K03[0o060352] = 'SPRT3';
  K03[0o060400] = 'SPRT4';   K03[0o060426] = 'SPRT5';   K03[0o060454] = 'SPRT6';
  K03[0o060502] = 'SPRT7';   K03[0o060530] = 'SPRT8';   K03[0o060556] = 'SPRT9';
  K03[0o060604] = 'SPR10';   K03[0o060632] = 'SPR11';   K03[0o060660] = 'SPR12';
  K03[0o060706] = 'SPR13';   K03[0o060734] = 'SPR14';   K03[0o060762] = 'SPR15';
  K03[0o061010] = 'SPR16';   K03[0o061036] = 'SPR17';   K03[0o061064] = 'SPR18';
  K03[0o061112] = 'SPR19';   K03[0o061140] = 'SPR20';   K03[0o061166] = 'COSPO';
  K03[0o061214] = 'WRT10';   K03[0o061242] = 'FDRT2';   K03[0o061270] = 'WRT11';
  K03[0o061316] = 'WRT12';   K03[0o061344] = 'WRT13';   K03[0o061372] = 'WRT20';
  K03[0o061420] = 'WRT14';   K03[0o061446] = 'WRT21';   K03[0o061474] = 'WRT25';
  K03[0o061522] = 'WRT26';   K03[0o061550] = 'WRT41';   K03[0o061576] = 'WRT42';
  K03[0o061624] = 'TADAD';
  K03[0o061652] = 'UDR01';   K03[0o061700] = 'UDR02';   K03[0o061726] = 'UDR03';
  K03[0o061754] = 'UDR04';   K03[0o062002] = 'UDR05';   K03[0o062030] = 'UDR06';
  K03[0o062056] = 'RTBES';

  var M06 = {};
  M06[0o012146] = 'DUMMY';   M06[0o012174] = 'STSIN';   M06[0o012222] = 'RTERR';
  M06[0o012250] = '1SWAP';   M06[0o012276] = 'TIMRT';   M06[0o012324] = 'RTDIL';
  M06[0o012352] = 'DIMWD';   M06[0o012400] = 'BPTMP';   M06[0o012426] = 'RTSLI';
  M06[0o012454] = 'ACCRT';   M06[0o012502] = 'TERMP';   M06[0o012530] = '5SWAP';
  M06[0o012556] = 'RWRT1';   M06[0o012604] = 'RWRT2';   M06[0o012632] = 'RWRT3';
  M06[0o012660] = 'RWRT5';   M06[0o012706] = 'RWRT7';   M06[0o012734] = 'RWRT8';
  M06[0o012762] = 'RWRT9';   M06[0o013010] = 'RTRFA';   M06[0o013036] = 'DUMM2';
  M06[0o013064] = 'SPRT1';   M06[0o013112] = 'SPRT2';   M06[0o013140] = 'SPRT3';
  M06[0o013166] = 'SPRT4';   M06[0o013214] = 'SPRT5';   M06[0o013242] = 'SPRT6';
  M06[0o013270] = 'SPRT7';   M06[0o013316] = 'SPRT8';   M06[0o013344] = 'SPRT9';
  M06[0o013372] = 'SPR10';   M06[0o013420] = 'SPR11';   M06[0o013446] = 'SPR12';
  M06[0o013474] = 'SPR13';   M06[0o013522] = 'SPR14';   M06[0o013550] = 'SPR15';
  M06[0o013576] = 'SPR16';   M06[0o013624] = 'SPR17';   M06[0o013652] = 'SPR18';
  M06[0o013700] = 'SPR19';   M06[0o013726] = 'SPR20';   M06[0o013754] = 'SPR21';
  M06[0o014002] = 'SPR22';   M06[0o014030] = 'COSPO';
  M06[0o014056] = 'RWR10';   M06[0o014104] = 'RWR11';   M06[0o014132] = 'RWR12';
  M06[0o014160] = 'RWR13';   M06[0o014206] = 'RWR20';   M06[0o014234] = 'RWR14';
  M06[0o014262] = 'RWR21';   M06[0o014310] = 'RWR25';   M06[0o014336] = 'RWR26';
  M06[0o014364] = 'RWR41';   M06[0o014412] = 'RWR42';
  M06[0o014720] = 'ERSWD';

  // =========================================================
  // RT program descriptions (slot-indexed, stable across versions)
  // Source: sintran-rt-programs.json verified reference data
  // =========================================================
  var DESCRIPTIONS = {};
  DESCRIPTIONS[0]  = 'Idle loop program. Runs when no other RT program is ready.';
  DESCRIPTIONS[1]  = 'System initialization. Performs post-boot terminal setup, file system mounting, and device configuration.';
  DESCRIPTIONS[2]  = 'RT error handler. Catches runtime errors including illegal instructions and memory protection violations.';
  DESCRIPTIONS[3]  = 'Primary swapper (swap-out). Swaps program segments between physical memory and disk.';
  DESCRIPTIONS[4]  = 'Timer RT program. Manages real-time clock events and wakes up RT programs whose timer has expired.';
  DESCRIPTIONS[5]  = 'RT DIL dispatch handler. Manages Device Independent Language subsystem calls for high-level I/O.';
  DESCRIPTIONS[6]  = 'Dimension watchdog. Monitors segment dimensions and memory allocation.';
  DESCRIPTIONS[7]  = 'Breakpoint/temporary RT. Handles software breakpoints for SEBUG debugger and single-step execution.';
  DESCRIPTIONS[8]  = 'Timeslice scheduler. Implements preemptive multitasking by periodically checking program priorities.';
  DESCRIPTIONS[9]  = 'Accounting RT. Tracks CPU time usage, I/O statistics, and resource consumption per user/program.';
  DESCRIPTIONS[10] = 'Terminal program handler. Core terminal I/O driver for all physical and virtual terminals.';
  DESCRIPTIONS[11] = 'Secondary swapper (swap-in). Loads program segments from disk back into physical memory.';
  DESCRIPTIONS[12] = 'Read/Write RT #1. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[13] = 'Read/Write RT #2. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[14] = 'Read/Write RT #3. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[15] = 'Read/Write RT #5. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[16] = 'Read/Write RT #7. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[17] = 'Read/Write RT #8. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[18] = 'Read/Write RT #9. Disk I/O worker for concurrent read/write operations.';
  DESCRIPTIONS[19] = 'Record File Access. Handles record-oriented file I/O for sequential and indexed file access.';
  DESCRIPTIONS[20] = 'Second dummy/placeholder RT. Separator between system RT slots and spool/extended slots.';
  DESCRIPTIONS[21] = 'Spool terminal handler #1. Manages terminal I/O and print spooling (ring 4).';
  DESCRIPTIONS[22] = 'Spool terminal handler #2.';
  DESCRIPTIONS[23] = 'Spool terminal handler #3.';
  DESCRIPTIONS[24] = 'Spool terminal handler #4.';
  DESCRIPTIONS[25] = 'Spool terminal handler #5.';
  DESCRIPTIONS[26] = 'Spool terminal handler #6.';
  DESCRIPTIONS[27] = 'Spool terminal handler #7.';
  DESCRIPTIONS[28] = 'Spool terminal handler #8.';
  DESCRIPTIONS[29] = 'Spool terminal handler #9.';
  DESCRIPTIONS[30] = 'Spool terminal handler #10.';
  DESCRIPTIONS[31] = 'Spool terminal handler #11.';
  DESCRIPTIONS[32] = 'Spool terminal handler #12.';
  DESCRIPTIONS[33] = 'Spool terminal handler #13.';
  DESCRIPTIONS[34] = 'Spool terminal handler #14.';
  DESCRIPTIONS[35] = 'Spool terminal handler #15.';
  DESCRIPTIONS[36] = 'Spool terminal handler #16.';
  DESCRIPTIONS[37] = 'Console spool program. Special spool handler for the system console (terminal 0).';
  DESCRIPTIONS[38] = 'Extended Read/Write RT #10. Additional disk I/O worker.';
  DESCRIPTIONS[39] = 'Extended Read/Write RT #11.';
  DESCRIPTIONS[40] = 'Extended Read/Write RT #12.';
  DESCRIPTIONS[41] = 'Extended Read/Write RT #13.';
  DESCRIPTIONS[42] = 'Extended Read/Write RT #20. For secondary disk controller.';
  DESCRIPTIONS[43] = 'Extended Read/Write RT #14.';
  DESCRIPTIONS[44] = 'Extended Read/Write RT #21. For secondary disk controller.';
  DESCRIPTIONS[45] = 'Extended Read/Write RT #25. For SCSI or additional controllers.';
  DESCRIPTIONS[46] = 'Extended Read/Write RT #26.';
  DESCRIPTIONS[47] = 'Extended Read/Write RT #41. For SMD or large disk controllers.';
  DESCRIPTIONS[48] = 'Extended Read/Write RT #42.';
  DESCRIPTIONS[49] = 'TAD adapter. Terminal access multiplexing for remote terminals via modem or network.';
  DESCRIPTIONS[50] = 'User Device Driver slot #1. Reserved for user-installable device drivers.';
  DESCRIPTIONS[51] = 'User Device Driver slot #2.';
  DESCRIPTIONS[52] = 'User Device Driver slot #3.';
  DESCRIPTIONS[53] = 'User Device Driver slot #4.';
  DESCRIPTIONS[54] = 'User Device Driver slot #5.';
  DESCRIPTIONS[55] = 'User Device Driver slot #6.';
  DESCRIPTIONS[56] = 'XMSG routing. Routes inter-process messages, possibly across network nodes.';
  DESCRIPTIONS[57] = 'XMSG trace. Diagnostic for tracing inter-process message flow.';
  DESCRIPTIONS[58] = 'XMSG File ID handler. Manages file identification across message boundaries.';
  DESCRIPTIONS[59] = 'Network serial handler. Manages serial communication for NORDNET or Ethernet.';
  DESCRIPTIONS[60] = 'Network name service. Resolves network node names to addresses.';
  DESCRIPTIONS[61] = 'Error Switch/Dispatch. Routes hardware and software exceptions to error handlers.';
  DESCRIPTIONS[62] = 'Program Manager. Manages program loading, linking, and execution lifecycle.';
  DESCRIPTIONS[63] = 'Event Message handler. Handles asynchronous event notification between processes.';
  DESCRIPTIONS[64] = 'BOP Control. Manages HDLC/BOP communication protocol for synchronous serial links.';
  DESCRIPTIONS[65] = 'Magnetic Tape Serial handler. Sequential access to magnetic tape drives.';
  DESCRIPTIONS[66] = 'RT Recovery. Crash recovery and consistency checking for RT program state.';
  DESCRIPTIONS[67] = 'Background Execution System handler. Manages batch and background program scheduling.';

  // =========================================================
  // Background/batch program configuration per version
  // BAKnn = background terminal process (nn = terminal number)
  // BCHnn = batch job slot
  // All background programs share STADR=9ENTO
  // =========================================================

  var BACKGROUND = {
    K: {
      firstBakAddr: 0o066642,  // 9FBPR
      lastBakAddr:  0o073016,  // 9LBPR
      bakCount: 99,
      firstBchAddr: null,
      bchCount: 0
    },
    L: {
      firstBakAddr: 0o023337,  // 9FBPR
      lastBakAddr:  0o030503,  // Last BAK slot (RT#337)
      bakCount: 121,
      firstBchAddr: 0o030531,  // 9LTBP (RT#338, BCH01)
      bchCount: 10
    },
    M: {
      firstBakAddr: null,      // Unverified
      lastBakAddr:  null,
      bakCount: 0,
      firstBchAddr: null,
      bchCount: 0
    }
  };

  // Entry size in words (22 decimal = 0o26)
  var RT_ENTRY_SIZE = 22;

  // =========================================================
  // Name resolution function
  // =========================================================

  /**
   * Resolve an RT program name from its slot number.
   * Uses the discovered RT table base to compute the slot address,
   * then does a reverse lookup in the version-appropriate symbol map.
   *
   * @param {number} rtNum - RT slot index (0-based)
   * @returns {string} Program name (e.g. "DUMMY", "BAK01") or "RT #N" if unknown
   */
  function resolveProcessName(rtNum) {
    var sym = window.sintranSymbols;
    if (!sym) return 'RT #' + rtNum;

    // Get RT table base
    var rtInfo = sym.discoverRtTable();
    if (rtInfo.base === 0) return 'RT #' + rtNum;

    // Compute the RT-Description address for this slot
    var rtAddr = rtInfo.base + rtNum * RT_ENTRY_SIZE;

    // Get version letter to select the right map
    var versionKey = getVersionKey();
    if (!versionKey) return 'RT #' + rtNum;

    // 1. Try named program lookup
    var nameMap = NAME_MAPS[versionKey];
    if (nameMap) {
      var name = nameMap[rtAddr];
      if (name) return name;
    }

    // 2. Try background/batch program naming
    var bg = BACKGROUND[versionKey];
    if (bg && bg.firstBakAddr !== null) {
      // BAKnn range
      if (rtAddr >= bg.firstBakAddr && rtAddr <= bg.lastBakAddr) {
        var bakNum = Math.floor((rtAddr - bg.firstBakAddr) / RT_ENTRY_SIZE) + 1;
        return 'BAK' + padNum(bakNum);
      }
      // BCHnn range
      if (bg.firstBchAddr !== null && bg.bchCount > 0) {
        var lastBchAddr = bg.firstBchAddr + (bg.bchCount - 1) * RT_ENTRY_SIZE;
        if (rtAddr >= bg.firstBchAddr && rtAddr <= lastBchAddr) {
          var bchNum = Math.floor((rtAddr - bg.firstBchAddr) / RT_ENTRY_SIZE) + 1;
          return 'BCH' + padNum(bchNum);
        }
      }
    }

    return 'RT #' + rtNum;
  }

  function resolveProcessDescription(rtNum) {
    if (DESCRIPTIONS[rtNum]) return DESCRIPTIONS[rtNum];
    // Background/batch programs
    var versionKey = getVersionKey();
    if (!versionKey) return null;
    var bg = BACKGROUND[versionKey];
    if (!bg) return null;
    var sym = window.sintranSymbols;
    if (!sym) return null;
    var rtInfo = sym.discoverRtTable();
    if (rtInfo.base === 0) return null;
    var rtAddr = rtInfo.base + rtNum * RT_ENTRY_SIZE;
    if (bg.firstBakAddr !== null && rtAddr >= bg.firstBakAddr && rtAddr <= bg.lastBakAddr) {
      return 'Background user process. One per user terminal, timesliced execution.';
    }
    if (bg.firstBchAddr !== null && bg.bchCount > 0) {
      var lastBchAddr = bg.firstBchAddr + (bg.bchCount - 1) * RT_ENTRY_SIZE;
      if (rtAddr >= bg.firstBchAddr && rtAddr <= lastBchAddr) {
        return 'Batch job slot. Non-interactive batch processing.';
      }
    }
    return null;
  }

  function padNum(n) {
    if (n < 10) return '0' + n;
    return '' + n;
  }

  function getVersionKey() {
    if (typeof sintranState === 'undefined' || !sintranState.versionLetter) return null;
    return sintranState.versionLetter.toUpperCase();
  }

  // Map version letters to lookup tables
  var NAME_MAPS = {
    K: K03,
    L: L07,
    M: M06
  };

  // =========================================================
  // Export
  // =========================================================
  window.resolveProcessName = resolveProcessName;
  window.resolveProcessDescription = resolveProcessDescription;
  window.sintranRtNames = {
    NAME_MAPS: NAME_MAPS,
    BACKGROUND: BACKGROUND,
    DESCRIPTIONS: DESCRIPTIONS,
    resolveProcessName: resolveProcessName,
    resolveProcessDescription: resolveProcessDescription
  };
})();
