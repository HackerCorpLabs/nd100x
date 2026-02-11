//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// sintran-segments.js - Segment Table Viewer
// Displays SINTRAN III segment table entries with flags and cross-references
// Algorithm from SINTRAN-STRUCTURES.md Section 6.17:
//   1. Read SGMAX, SEGTB, SEGST via DPIT
//   2. Physical base = (SEGTB << 16) + SEGST
//   3. For each segment 0..SGMAX, read 8-word entry at physBase + segNum*8
//   4. Skip all-zero entries
//   5. Cross-reference: RT SEGM1/SEGM2 contain segment NUMBERS (not addresses)

(function() {
  'use strict';

  var sym = window.sintranSymbols;
  var FX = sym.FIXED;
  var SE = sym.SEG_ENTRY;
  var RT = sym.RT_DESC;

  var refreshTimer = null;
  var autoRefresh = true;
  var lastSegments = [];

  // =========================================================
  // Segment FLAG bit names (word offset 5)
  // Source: SINTRAN-STRUCTURES.md Section 6.5
  // =========================================================
  var FLAG_BITS = {
    0: 'OK',        // 5OK - segment valid
    1: 'INHIB',     // 5INHB - inhibited (swapped out / not loaded)
    3: 'PROT',      // PROTECT - write protected
    4: 'SHARED',    // 5SREE - shared/reentrant
    5: 'FIXED',     // 5FIXC - fixed in core (never swapped)
    6: 'DEMAND'     // demand-loaded
  };

  // =========================================================
  // Segment STATUS (SGSTA) bit definitions (word offset 6)
  // Source: SINTRAN-STRUCTURES.md Section 6.6
  // =========================================================
  // Bit 15: WPM  (write permit)
  // Bit 14: RPM  (read permit)
  // Bit 13: FPM  (fetch permit)
  // Bits 10-9: Ring (0-3)
  // Bit 0: NOCLEAR (do not clear on load)

  function decodeSgsta(sgsta) {
    var parts = [];
    if (sgsta & (1 << 15)) parts.push('WPM');
    if (sgsta & (1 << 14)) parts.push('RPM');
    if (sgsta & (1 << 13)) parts.push('FPM');
    var ring = (sgsta >> 9) & 0x03;
    parts.push('R' + ring);
    if (sgsta & 1) parts.push('NOCLEAR');
    return parts.join(' ');
  }

  // =========================================================
  // Data reading - Section 6.17 algorithm
  // =========================================================

  function readSegmentTable() {
    // Step 1: Read root pointers via DPIT
    var sgmax = sym.readWord(FX.SGMAX);
    var segtb = sym.readWord(FX.SEGTB);
    var segst = sym.readWord(FX.SEGST);

    if (sgmax === 0 || sgmax > 4096) return [];

    // Step 2: Compute physical base address
    // SEGTB is the bank number, SEGST is offset within that bank
    // Physical word address = (SEGTB << 16) + SEGST
    var physBase = (segtb << 16) + segst;

    // Step 3: Read all segment entries via physical memory
    var segments = [];
    for (var segNum = 0; segNum <= sgmax; segNum++) {
      var entryAddr = physBase + segNum * SE.SIZE;
      var data = sym.readBlockPhysical(entryAddr, SE.SIZE);
      if (data.length < SE.SIZE) continue;

      // Step 4: Skip all-zero entries (unused segments)
      var allZero = true;
      for (var w = 0; w < SE.SIZE; w++) {
        if (data[w] !== 0) { allZero = false; break; }
      }
      if (allZero) continue;

      segments.push({
        segNum: segNum,
        segli:  data[SE.SEGLI],
        prese:  data[SE.PRESE],
        logad:  data[SE.LOGAD],
        segle:  data[SE.SEGLE],
        madr:   data[SE.MADR],
        flag:   data[SE.FLAG],
        sgsta:  data[SE.SGSTA],
        bpagl:  data[SE.BPAGL],
        usedBy: []
      });
    }

    // Step 5: Cross-reference with RT descriptions
    crossReferenceSegments(segments);

    return segments;
  }

  function crossReferenceSegments(segments) {
    var rtInfo = sym.discoverRtTable();
    if (rtInfo.base === 0 || rtInfo.count === 0) return;

    var rtCount = rtInfo.count;
    var tableData = sym.readBlock(rtInfo.base, rtCount * RT.SIZE);
    if (tableData.length === 0) return;

    // Build a map of segment NUMBER to segment entry for fast lookup
    var segByNum = {};
    for (var s = 0; s < segments.length; s++) {
      segByNum[segments[s].segNum] = segments[s];
    }

    for (var i = 0; i < rtCount; i++) {
      var base = i * RT.SIZE;
      var statu = tableData[base + RT.STATU];
      if (!sym.testBit(statu, sym.STATU_BITS.USED)) continue;

      // RT SEGM1/SEGM2/ACT1S/ACT2S contain segment NUMBERS
      // (not addresses - the segment number indexes into the segment table)
      var segFields = [
        tableData[base + RT.SEGM1],
        tableData[base + RT.SEGM2],
        tableData[base + RT.ACT1S],
        tableData[base + RT.ACT2S]
      ];

      for (var f = 0; f < segFields.length; f++) {
        var segNum = segFields[f];
        if (segNum !== 0 && segByNum[segNum]) {
          var label = (typeof window.resolveProcessName === 'function')
            ? window.resolveProcessName(i) : 'RT #' + i;
          if (segByNum[segNum].usedBy.indexOf(label) === -1) {
            segByNum[segNum].usedBy.push(label);
          }
        }
      }
    }
  }

  // =========================================================
  // Flag decoding
  // =========================================================

  var FLAG_TIPS = {
    0: 'OK - segment is valid and loaded in memory',
    1: 'INHIB - inhibited: segment is swapped out to disk or not yet loaded',
    3: 'PROT - write-protected: any write attempt causes a trap',
    4: 'SHARED - shared/reentrant: multiple RT programs share this segment',
    5: 'FIXED - permanently resident in physical memory, never swapped',
    6: 'DEMAND - demand-loaded: pages loaded from disk only when accessed'
  };

  function decodeFlags(flag) {
    var parts = [];
    for (var bit in FLAG_BITS) {
      if (sym.testBit(flag, parseInt(bit))) {
        parts.push(FLAG_BITS[bit]);
      }
    }
    return parts.length > 0 ? parts.join(', ') : '-';
  }

  function flagsTip(flag) {
    var lines = [];
    for (var bit in FLAG_BITS) {
      var b = parseInt(bit);
      var set = sym.testBit(flag, b);
      lines.push((set ? '[X] ' : '[ ] ') + FLAG_TIPS[bit]);
    }
    return lines.join('\n');
  }

  function sgstaTip(sgsta) {
    var wpm = !!(sgsta & (1 << 15));
    var rpm = !!(sgsta & (1 << 14));
    var fpm = !!(sgsta & (1 << 13));
    var ring = (sgsta >> 9) & 0x03;
    var noclear = !!(sgsta & 1);
    var lines = [
      'Memory protection and access control bits:',
      (wpm ? '[X]' : '[ ]') + ' WPM - write permit: processes can write to this segment',
      (rpm ? '[X]' : '[ ]') + ' RPM - read permit: processes can read this segment',
      (fpm ? '[X]' : '[ ]') + ' FPM - fetch permit: CPU can execute code from this segment',
      'Ring ' + ring + ' - protection ring (0=kernel, 1-2=system, 3=user)',
      (noclear ? '[X]' : '[ ]') + ' NOCLEAR - do not zero-fill pages when loaded from disk'
    ];
    return lines.join('\n');
  }

  // =========================================================
  // Rendering
  // =========================================================

  function escAttr(str) {
    return str.replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  // Column header tooltips
  var COL_TIPS = {
    segNum: 'Unique index in the segment table',
    name:   'System segment name from SINTRAN release documentation.\nS3xxx = active, S3Sxxx = save copy, S3Ixxx = image copy.\nUser segments (136-255) show the RT programs that use them.',
    logad:  'Starting page in virtual address space.\nMultiply by 1024 to get the word address.',
    pages:  'Segment length in pages (octal).\nEach page = 1024 words.',
    madr:   'Disk swap location.\nWhere this segment is stored on disk when swapped out.',
    bpagl:  'First physical page in the core map chain.\nLinks to the physical memory pages allocated to this segment.',
    flags:  'Loading, sharing and protection behavior.\nHover over cells for details on each flag.',
    status: 'Memory protection bits and ring level.\nControls read/write/execute permissions.\nHover over cells for details.',
    usedBy: 'Which RT programs reference this segment.\nShows processes using this as code, data, or active segment.'
  };

  function renderSegmentTable(segments) {
    var body = document.getElementById('segment-table-body');
    if (!body) return;

    if (segments.length === 0) {
      body.innerHTML = '<div style="padding:12px;color:rgba(160,175,210,0.5);font-style:italic;">No segments found</div>';
      return;
    }

    var html = '<table class="proc-table"><thead><tr>' +
      '<th title="' + escAttr(COL_TIPS.segNum) + '">Seg#</th>' +
      '<th title="' + escAttr(COL_TIPS.name) + '">Name</th>' +
      '<th title="' + escAttr(COL_TIPS.logad) + '">Virtual Page</th>' +
      '<th title="' + escAttr(COL_TIPS.pages) + '">Size</th>' +
      '<th title="' + escAttr(COL_TIPS.madr) + '">Disk Location</th>' +
      '<th title="' + escAttr(COL_TIPS.bpagl) + '">Core Map Link</th>' +
      '<th title="' + escAttr(COL_TIPS.flags) + '">Flags</th>' +
      '<th title="' + escAttr(COL_TIPS.status) + '">Protection</th>' +
      '<th title="' + escAttr(COL_TIPS.usedBy) + '">Used By</th>' +
      '</tr></thead><tbody>';

    for (var i = 0; i < segments.length; i++) {
      var s = segments[i];
      var pages = s.segle & 0x3FF;
      var pagesStr = pages.toString(8);
      var pagesTip = pages + ' pages = ' + (pages * 1024) + ' words (' + pages + 'K words)';
      var logadTip = 'Logical page ' + sym.toOctal(s.logad) + ' = word address ' + sym.toOctal(s.logad << 10 & 0xFFFF);
      var usedByStr = s.usedBy.length > 0 ? s.usedBy.join(', ') : '-';

      // Resolve segment name and description
      var segName = (typeof window.resolveSegmentName === 'function')
        ? window.resolveSegmentName(s.segNum) : '';
      var segDesc = (typeof window.resolveSegmentDescription === 'function')
        ? window.resolveSegmentDescription(s.segNum) : null;
      var segCat = (typeof window.resolveSegmentCategory === 'function')
        ? window.resolveSegmentCategory(s.segNum) : null;
      var nameTip = '';
      if (segDesc) {
        nameTip = segDesc;
        if (segCat) nameTip += '\nCategory: ' + segCat;
      } else if (s.usedBy.length > 0) {
        nameTip = 'User segment used by: ' + s.usedBy.join(', ');
      }
      // For unnamed segments, show the RT cross-reference as name
      var nameDisplay = segName || (s.usedBy.length > 0 ? s.usedBy[0] : '');

      var segOct = s.segNum.toString(8);
      html += '<tr class="proc-row">' +
        '<td title="Segment ' + segOct + ' octal (' + s.segNum + ' decimal)">' + segOct + '</td>' +
        '<td title="' + escAttr(nameTip) + '">' + nameDisplay + '</td>' +
        '<td title="' + escAttr(logadTip) + '">' + sym.toOctal(s.logad) + '</td>' +
        '<td title="' + escAttr(pagesTip) + '">' + pagesStr + '</td>' +
        '<td title="Disk swap address (octal)">' + sym.toOctal(s.madr) + '</td>' +
        '<td title="First physical page in core map chain">' + sym.toOctal(s.bpagl) + '</td>' +
        '<td title="' + escAttr(flagsTip(s.flag)) + '">' + decodeFlags(s.flag) + '</td>' +
        '<td title="' + escAttr(sgstaTip(s.sgsta)) + '">' + decodeSgsta(s.sgsta) + '</td>' +
        '<td title="' + escAttr(usedByStr) + '">' + usedByStr + '</td>' +
        '</tr>';
    }

    html += '</tbody></table>';
    body.innerHTML = html;
  }

  // =========================================================
  // Refresh
  // =========================================================

  function refreshSegments() {
    if (!sintranState || !sintranState.detected) return;
    sym.invalidateRtCache();
    lastSegments = readSegmentTable();
    renderSegmentTable(lastSegments);
  }

  function startAutoRefresh() {
    stopAutoRefresh();
    if (autoRefresh) {
      refreshTimer = setInterval(refreshSegments, 2000);
    }
  }

  function stopAutoRefresh() {
    if (refreshTimer) {
      clearInterval(refreshTimer);
      refreshTimer = null;
    }
  }

  // =========================================================
  // Window show/hide
  // =========================================================

  function showWindow() {
    var win = document.getElementById('segment-table-window');
    if (win) {
      win.style.display = 'flex';
      refreshSegments();
      startAutoRefresh();
    }
  }

  function hideWindow() {
    var win = document.getElementById('segment-table-window');
    if (win) win.style.display = 'none';
    stopAutoRefresh();
  }

  // =========================================================
  // Copy to markdown
  // =========================================================

  function copyAsMarkdown() {
    if (lastSegments.length === 0) return;

    var md = '| Seg# | Name | Virtual Page | Size | Disk Location | Core Map Link | Flags | Protection | Used By |\n';
    md += '|------|------|-------------|------|---------------|---------------|-------|------------|---------|\n';

    for (var i = 0; i < lastSegments.length; i++) {
      var s = lastSegments[i];
      var pagesStr = (s.segle & 0x3FF).toString(8);
      var segName = (typeof window.resolveSegmentName === 'function')
        ? window.resolveSegmentName(s.segNum) : '';
      var nameDisplay = segName || (s.usedBy.length > 0 ? s.usedBy[0] : '');
      md += '| ' + s.segNum.toString(8) +
        ' | ' + nameDisplay +
        ' | ' + sym.toOctal(s.logad) +
        ' | ' + pagesStr +
        ' | ' + sym.toOctal(s.madr) +
        ' | ' + sym.toOctal(s.bpagl) +
        ' | ' + decodeFlags(s.flag) +
        ' | ' + decodeSgsta(s.sgsta) +
        ' | ' + (s.usedBy.length > 0 ? s.usedBy.join(', ') : '-') +
        ' |\n';
    }

    navigator.clipboard.writeText(md).then(function() {
      var btn = document.getElementById('segment-table-copy');
      if (btn) {
        btn.classList.add('copied');
        setTimeout(function() { btn.classList.remove('copied'); }, 1200);
      }
    });
  }

  // =========================================================
  // Event handlers
  // =========================================================

  var closeBtn = document.getElementById('segment-table-close');
  if (closeBtn) closeBtn.addEventListener('click', hideWindow);

  var copyBtn = document.getElementById('segment-table-copy');
  if (copyBtn) copyBtn.addEventListener('click', copyAsMarkdown);

  var refreshBtn = document.getElementById('segment-table-refresh');
  if (refreshBtn) refreshBtn.addEventListener('click', refreshSegments);

  var autoRefreshCb = document.getElementById('segment-auto-refresh');
  if (autoRefreshCb) {
    autoRefreshCb.addEventListener('change', function() {
      autoRefresh = this.checked;
      if (autoRefresh) startAutoRefresh();
      else stopAutoRefresh();
    });
  }

  // =========================================================
  // Export
  // =========================================================
  window.segTableShowWindow = showWindow;
  window.segTableHideWindow = hideWindow;
})();
