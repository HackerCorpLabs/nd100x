//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// sintran-queues.js - Queue Viewer window with 3 tabs
// Shows Execution Queue, Time Queue, and Monitor Queue

(function() {
  'use strict';

  var sym = window.sintranSymbols;
  var FX = sym.FIXED;
  var RT = sym.RT_DESC;
  var DF = sym.IO_DF;

  var refreshTimer = null;
  var autoRefresh = true;
  var activeTab = 'exec';

  // =========================================================
  // Execution Queue (circular, linked via WLINK)
  // =========================================================

  function readExecQueue() {
    var head = sym.readWord(FX.BEXQU);
    if (head === 0 || head === 0xFFFF) return [];

    // Get RT table range for WLINK validation
    var rtInfo = sym.discoverRtTable();

    var entries = [];
    var first = head;
    var addr = head;
    var count = 0;

    do {
      var rtData = sym.readBlock(addr, RT.SIZE);
      if (rtData.length < RT.SIZE) break;

      var rtNum = resolveRtNumber(addr);
      entries.push({
        rtAddr: addr,
        rtNum: rtNum,
        name: resolveNameFromAddr(addr, rtNum),
        prity: rtData[RT.PRITY],
        statu: rtData[RT.STATU]
      });

      addr = rtData[RT.WLINK];
      count++;
      if (count > 50) break;
      // Validate WLINK is within RT table range
      if (rtInfo.base > 0 && (addr < rtInfo.base || addr >= rtInfo.base + rtInfo.count * RT.SIZE)) {
        break;
      }
    } while (addr !== 0 && addr !== 0xFFFF && addr !== first);

    return entries;
  }

  // =========================================================
  // Time Queue (linear, linked via TLINK, stops at 0)
  // =========================================================

  function readTimeQueue() {
    var head = sym.readWord(FX.BTIMQ);
    if (head === 0) return [];

    var entries = [];
    var addr = head;
    var count = 0;
    var visited = {};

    while (addr !== 0 && addr !== 0xFFFF && count < 100 && !visited[addr]) {
      visited[addr] = true;
      var rtData = sym.readBlock(addr, RT.SIZE);
      if (rtData.length < RT.SIZE) break;

      var rtNum = resolveRtNumber(addr);
      var delayTime = (rtData[RT.DTIM1] << 16) | rtData[RT.DTIM2];
      var interval = (rtData[RT.DTIN1] << 16) | rtData[RT.DTIN2];

      entries.push({
        rtAddr: addr,
        rtNum: rtNum,
        name: resolveNameFromAddr(addr, rtNum),
        delayTime: delayTime,
        interval: interval
      });

      addr = rtData[RT.TLINK];
      count++;
    }

    return entries;
  }

  // =========================================================
  // Monitor Queue (linear, linked via MLINK in I/O datafields)
  // =========================================================

  function readMonitorQueue() {
    var head = sym.readWord(FX.MQUEU);
    if (head === 0 || head === 0xFFFF) return [];

    var entries = [];
    var addr = head;
    var count = 0;
    var visited = {};

    while (addr !== 0 && addr !== 0xFFFF && count < 50 && !visited[addr]) {
      visited[addr] = true;

      var rtres = sym.readWord(addr + DF.RTRES);
      var mfunc = sym.readWord(addr + DF.MFUNC);
      var istat = sym.readWord(addr + DF.ISTAT);

      entries.push({
        dfAddr: addr,
        rtres: rtres,
        mfunc: mfunc,
        istat: istat,
        devType: decodeDeviceType(istat)
      });

      addr = sym.readWord(addr + DF.MLINK);
      count++;
    }

    return entries;
  }

  // =========================================================
  // Helpers
  // =========================================================

  function resolveRtNumber(rtAddr) {
    return sym.rtAddrToNumber(rtAddr);
  }

  function resolveNameFromAddr(rtAddr, rtNum) {
    if (rtNum < 0) return 'RT @' + sym.toOctal(rtAddr);
    if (typeof window.resolveProcessName === 'function') {
      return window.resolveProcessName(rtNum);
    }
    return 'RT #' + rtNum;
  }

  function attachDblClickHandlers(container) {
    var rows = container.querySelectorAll('.proc-row[data-rtnum]');
    for (var i = 0; i < rows.length; i++) {
      rows[i].addEventListener('dblclick', function() {
        var rtNum = parseInt(this.getAttribute('data-rtnum'));
        if (rtNum >= 0 && typeof window.procListNavigateToRT === 'function') {
          window.procListNavigateToRT(rtNum);
        }
      });
    }
  }

  function decodeDeviceType(istat) {
    var types = [];
    if (sym.testBit(istat, sym.ISTAT_BITS.TERM)) types.push('Terminal');
    if (sym.testBit(istat, sym.ISTAT_BITS.FLOP)) types.push('Floppy');
    if (sym.testBit(istat, sym.ISTAT_BITS.MT))   types.push('Mag Tape');
    if (sym.testBit(istat, sym.ISTAT_BITS.BAD))  types.push('Bad');
    return types.length > 0 ? types.join(', ') : 'Device';
  }

  // =========================================================
  // Rendering
  // =========================================================

  function renderExecQueue(entries) {
    var container = document.getElementById('queue-exec-content');
    if (!container) return;

    if (entries.length === 0) {
      container.innerHTML = '<div class="queue-empty">Execution queue is empty</div>';
      return;
    }

    var html = '<table class="proc-table"><thead><tr>' +
      '<th>RT#</th><th>Name</th><th>Priority</th><th>Status</th>' +
      '</tr></thead><tbody>';

    for (var i = 0; i < entries.length; i++) {
      var e = entries[i];
      html += '<tr class="proc-row" data-rtnum="' + e.rtNum + '">' +
        '<td>' + e.rtNum + '</td>' +
        '<td class="queue-rt-name">' + e.name + '</td>' +
        '<td>' + sym.toOctal(e.prity) + '</td>' +
        '<td>' + sym.toOctal(e.statu) + '</td>' +
        '</tr>';
    }

    html += '</tbody></table>';
    container.innerHTML = html;
    attachDblClickHandlers(container);
  }

  function renderTimeQueue(entries) {
    var container = document.getElementById('queue-time-content');
    if (!container) return;

    if (entries.length === 0) {
      container.innerHTML = '<div class="queue-empty">Time queue is empty</div>';
      return;
    }

    var html = '<table class="proc-table"><thead><tr>' +
      '<th>RT#</th><th>Name</th><th>Delay Time</th><th>Interval</th>' +
      '</tr></thead><tbody>';

    for (var i = 0; i < entries.length; i++) {
      var e = entries[i];
      html += '<tr class="proc-row" data-rtnum="' + e.rtNum + '">' +
        '<td>' + e.rtNum + '</td>' +
        '<td class="queue-rt-name">' + e.name + '</td>' +
        '<td>' + e.delayTime + '</td>' +
        '<td>' + e.interval + '</td>' +
        '</tr>';
    }

    html += '</tbody></table>';
    container.innerHTML = html;
    attachDblClickHandlers(container);
  }

  function renderMonitorQueue(entries) {
    var container = document.getElementById('queue-mon-content');
    if (!container) return;

    if (entries.length === 0) {
      container.innerHTML = '<div class="queue-empty">Monitor queue is empty</div>';
      return;
    }

    var html = '<table class="proc-table"><thead><tr>' +
      '<th>DF Addr</th><th>Owner RT</th><th>Mon Func</th><th>Device Type</th>' +
      '</tr></thead><tbody>';

    for (var i = 0; i < entries.length; i++) {
      var e = entries[i];
      html += '<tr class="proc-row">' +
        '<td>' + sym.toOctal(e.dfAddr) + '</td>' +
        '<td>' + sym.toOctal(e.rtres) + '</td>' +
        '<td>' + sym.toOctal(e.mfunc) + '</td>' +
        '<td>' + e.devType + '</td>' +
        '</tr>';
    }

    html += '</tbody></table>';
    container.innerHTML = html;
  }

  // =========================================================
  // Tab management
  // =========================================================

  function switchTab(tabName) {
    activeTab = tabName;

    // Update tab buttons
    var tabs = document.querySelectorAll('#queue-viewer-window .queue-tab');
    for (var i = 0; i < tabs.length; i++) {
      if (tabs[i].getAttribute('data-tab') === tabName) {
        tabs[i].classList.add('active');
      } else {
        tabs[i].classList.remove('active');
      }
    }

    // Update tab content
    var contents = document.querySelectorAll('#queue-viewer-window .queue-tab-content');
    for (var j = 0; j < contents.length; j++) {
      if (contents[j].getAttribute('data-tab') === tabName) {
        contents[j].classList.add('active');
      } else {
        contents[j].classList.remove('active');
      }
    }

    refreshQueues();
  }

  // =========================================================
  // Refresh
  // =========================================================

  function refreshQueues() {
    if (!sintranState || !sintranState.detected) return;
    sym.invalidateRtCache();

    if (activeTab === 'exec') {
      renderExecQueue(readExecQueue());
    } else if (activeTab === 'time') {
      renderTimeQueue(readTimeQueue());
    } else if (activeTab === 'mon') {
      renderMonitorQueue(readMonitorQueue());
    }
  }

  function startAutoRefresh() {
    stopAutoRefresh();
    if (autoRefresh) {
      refreshTimer = setInterval(refreshQueues, 2000);
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
    var win = document.getElementById('queue-viewer-window');
    if (win) {
      win.style.display = 'flex';
      refreshQueues();
      startAutoRefresh();
    }
  }

  function hideWindow() {
    var win = document.getElementById('queue-viewer-window');
    if (win) win.style.display = 'none';
    stopAutoRefresh();
  }

  // =========================================================
  // Event handlers
  // =========================================================

  var closeBtn = document.getElementById('queue-viewer-close');
  if (closeBtn) closeBtn.addEventListener('click', hideWindow);

  var refreshBtn = document.getElementById('queue-viewer-refresh');
  if (refreshBtn) refreshBtn.addEventListener('click', refreshQueues);

  // Tab click handlers
  var tabBtns = document.querySelectorAll('#queue-viewer-window .queue-tab');
  for (var t = 0; t < tabBtns.length; t++) {
    tabBtns[t].addEventListener('click', function() {
      switchTab(this.getAttribute('data-tab'));
    });
  }

  var autoRefreshCb = document.getElementById('queue-auto-refresh');
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
  window.queueViewerShowWindow = showWindow;
  window.queueViewerHideWindow = hideWindow;
})();
