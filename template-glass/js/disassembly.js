// disassembly.js - Standalone Disassembly Glass Window
// Renders the same disassembly data as the debugger's Disasm tab
// in a separate, draggable/resizable glass window.

(function() {
  'use strict';

  var visible = false;
  var refreshTimer = null;
  var view = document.getElementById('disasm-standalone-view');

  /* --- Utilities (local copies to avoid cross-module deps) --- */

  function oct(val) {
    return ('000000' + ((val >>> 0) & 0xFFFF).toString(8)).slice(-6);
  }

  function escapeHtml(str) {
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  function wasmReady() {
    return typeof Module !== 'undefined' && Module && Module.calledRun && Module._Dbg_GetPC;
  }

  function wasmCall(fnName) {
    return typeof Module !== 'undefined' && Module && typeof Module[fnName] === 'function';
  }

  /* --- Window show / hide --- */

  function show() {
    visible = true;
    document.getElementById('disasm-window').style.display = 'flex';
    renderDisasm();
    startRefreshTimer();
  }

  function hide() {
    visible = false;
    document.getElementById('disasm-window').style.display = 'none';
    stopRefreshTimer();
  }

  /* --- Auto-refresh timer --- */

  function startRefreshTimer() {
    if (refreshTimer) return;
    refreshTimer = setInterval(function() {
      if (visible) renderDisasm();
    }, 250);
  }

  function stopRefreshTimer() {
    if (refreshTimer) { clearInterval(refreshTimer); refreshTimer = null; }
  }

  /* --- Render disassembly --- */

  function renderDisasm() {
    if (!view || !wasmReady() || !wasmCall('_Dbg_Disassemble')) return;
    if (!Module._IsInitialized || !Module._IsInitialized()) return;

    var breakpoints = window.dbgBreakpoints || new Set();
    var pc = Module._Dbg_GetPC();
    var ptr = Module._Dbg_Disassemble((pc - 10) & 0xFFFF, 30);
    var lines = Module.UTF8ToString(ptr).trim().split('\n');

    var html = '';
    lines.forEach(function(line) {
      var parts = line.split(/\s+/);
      if (parts.length < 3) return;
      var addr = parseInt(parts[0], 8);
      var isPC = (addr === pc);
      var hasBP = breakpoints.has(addr);

      html += '<div class="disasm-line' + (isPC ? ' current-pc' : '') + '" data-addr="' + addr + '">';
      html += '<div class="disasm-gutter">';
      html += '<span class="disasm-gutter-bp"><span class="disasm-bp-dot' + (hasBP ? ' active' : '') + '"></span></span>';
      html += '<span class="disasm-gutter-pc">' + (isPC ? '<span class="disasm-pc-arrow">&#9654;</span>' : '') + '</span>';
      html += '</div>';
      html += '<span class="disasm-addr">' + parts[0] + '</span>';
      html += '<span class="disasm-word">' + parts[1] + '</span>';
      html += '<span class="disasm-mnemonic">' + escapeHtml(parts.slice(2).join(' ')) + '</span>';
      html += '</div>';
    });
    view.innerHTML = html;

    var currentLine = view.querySelector('.current-pc');
    if (currentLine) currentLine.scrollIntoView({ block: 'center', behavior: 'auto' });

    /* Breakpoint click handler */
    view.querySelectorAll('.disasm-line').forEach(function(el) {
      el.addEventListener('click', function() {
        var addr = parseInt(el.getAttribute('data-addr'));
        if (isNaN(addr)) return;
        if (breakpoints.has(addr)) {
          breakpoints.delete(addr);
          if (Module._Dbg_RemoveBreakpoint) Module._Dbg_RemoveBreakpoint(addr);
        } else {
          breakpoints.add(addr);
          if (Module._Dbg_AddBreakpoint) Module._Dbg_AddBreakpoint(addr);
        }
        renderDisasm();
        /* Keep debugger tab in sync */
        if (typeof window.dbgRefreshDisasm === 'function') {
          try { window.dbgRefreshDisasm(); } catch(e) {}
        }
      });
    });
  }

  /* --- Expose for toolbar.js --- */
  window.disasmShowWindow = show;
  window.disasmHideWindow = hide;

})();
