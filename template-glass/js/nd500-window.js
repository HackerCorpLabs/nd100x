//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// nd500-window.js - the ND-500 console.
//
// The ND-500 has run inside this module since M2c, but only from the browser
// console: emu.nd500.create(), loadKernel(), boot(), step() by hand. This is
// the window that does it, and the terminal that shows the result.
//
// WHAT IT IS. A kernel, optionally its pre-split segment files, and up to four
// disc images go in; NDIX boots and you get a login prompt. The ND-500 runs on
// its own here, answering its own front-end calls the way nd500x does natively
// (front_end = synthetic). It is NOT connected to the ND-100 in the same page -
// no shared memory, no 3022 bus interface. That is a later milestone, and
// pretending otherwise in the UI would be a lie about what the machine is.
//
// WHERE THE FILES COME FROM. Two places, and both are honest about themselves:
//   - the local disk library (the same OPFS store the HDD manager uses), which
//     is where a catalog download lands;
//   - a file picker, because a kernel is not in the catalog - there is no
//     vmunix on the server, and the one people actually boot is the one they
//     just built.
//
// WHY IT STEPS ON A TIMER. The page has one thread. nd500x's run() does not
// come back until the guest stops, so the window asks for a slice of
// instructions at a time and gives the browser its thread back in between.

(function () {
  'use strict';

  // How much CPU to spend per tick, and how often. 300k instructions at ~4.5M/s
  // measured under node is ~65 ms of work - too long for one frame, so the
  // interval is deliberately longer than a frame and the page stays responsive
  // while the guest runs at a useful speed. Both are adjustable on screen.
  var SLICE_DEFAULT = 300000;
  var TICK_MS = 40;

  var MAX_LINES = 1000;   // console scrollback

  var win, header, closeBtn, resizeHandle;
  var consoleEl, statusEl, bootBtn, stopBtn, sliceInput;
  var kernelSel, kernelFile, psegFile, dsegFile, diskSel, diskFile, memSel;
  var writableBox;

  var timer = null;
  var booted = false;
  var pendingKernel = null;    // Uint8Array
  var pendingPseg = null, pendingDseg = null;
  var pendingDisk = null;
  var lineBuf = '';

  function el(id) { return document.getElementById(id); }

  function setStatus(text, kind) {
    if (!statusEl) return;
    statusEl.textContent = text;
    statusEl.style.color = (kind === 'err') ? '#FF8A8A'
                         : (kind === 'ok') ? '#7CFC7C' : '';
  }

  // ---- console ------------------------------------------------------------

  // Guest output arrives as bytes, not lines. Appending to a text node and
  // trimming by line is enough for a login prompt and a shell; this is not a
  // VT100 and does not claim to be - cursor addressing would need the real
  // terminal core, and that one is bound to the ND-100's device layer.
  function write(text) {
    if (!consoleEl) return;
    lineBuf += text;
    var lines = lineBuf.split('\n');
    if (lines.length > MAX_LINES) {
      lines = lines.slice(lines.length - MAX_LINES);
      lineBuf = lines.join('\n');
    }
    consoleEl.textContent = lineBuf;
    consoleEl.scrollTop = consoleEl.scrollHeight;
  }

  function drain() {
    if (!window.emu || !emu.nd500) return;
    var chunks = emu.nd500.pollConsole();
    for (var i = 0; i < chunks.length; i++) {
      var c = chunks[i];
      // Unit 255 is the emulator's own boot log, not the guest talking. It is
      // shown, because when a boot dies the last line printed is how you know
      // which step it died in - but marked, so it is never mistaken for NDIX.
      if (c.unit === 255) write(c.text.replace(/^/gm, '| '));
      else write(c.text);
    }
  }

  // ---- the run loop -------------------------------------------------------

  function tick() {
    if (!booted) return;
    var slice = parseInt(sliceInput && sliceInput.value, 10) || SLICE_DEFAULT;
    try {
      emu.nd500.step(slice);
    } catch (e) {
      stop();
      setStatus('stopped: ' + (e && e.message ? e.message : e), 'err');
      return;
    }
    drain();
    // The run FLAG, not the stop reason. NDIX takes page faults constantly -
    // that is what demand paging is - and each one leaves a stop reason behind
    // while the machine carries on perfectly happily. Only the flag going away
    // means the CPU actually stopped.
    if (!emu.nd500.isRunning()) {
      stop();
      setStatus('stopped: ' + emu.nd500.stopReason(), 'err');
    }
  }

  function start() {
    if (timer) return;
    timer = setInterval(tick, TICK_MS);
  }

  function stop() {
    if (timer) { clearInterval(timer); timer = null; }
    if (stopBtn) stopBtn.textContent = 'Run';
  }

  // ---- loading ------------------------------------------------------------

  function readFile(input) {
    return new Promise(function (resolve, reject) {
      var f = input && input.files && input.files[0];
      if (!f) { resolve(null); return; }
      var r = new FileReader();
      r.onload = function () { resolve(new Uint8Array(r.result)); };
      r.onerror = function () { reject(r.error); };
      r.readAsArrayBuffer(f);
    });
  }

  // Images the local library holds that make sense for an ND-500. The catalog
  // tags an NDIX root image diskType "nd500"; anything untagged is an ND-100
  // disc and is not offered here, because mounting an SMD image as an NDIX root
  // produces a boot that fails in a confusing way rather than an obvious one.
  function refreshLibrary() {
    if (!diskSel) return;
    diskSel.innerHTML = '<option value="">(none - or choose a file below)</option>';
    if (typeof smdStorage === 'undefined' || !smdStorage.isAvailable()) return;
    var imgs = smdStorage.listImages();
    for (var i = 0; i < imgs.length; i++) {
      if ((imgs[i].diskType || '') !== 'nd500') continue;
      var o = document.createElement('option');
      o.value = imgs[i].uuid;
      o.textContent = imgs[i].name + ' (' + smdStorage.formatSize(imgs[i].size) + ')';
      diskSel.appendChild(o);
    }
  }

  async function boot() {
    if (!window.emu || !emu.nd500 || !emu.nd500.available()) {
      setStatus('This build has no ND-500.', 'err');
      return;
    }
    if (emu.isWorkerMode && emu.isWorkerMode()) {
      setStatus('The ND-500 needs direct mode; Worker mode does not forward it yet.', 'err');
      return;
    }
    if (booted) {
      setStatus('Already booted. Reload the page to boot a different kernel.', 'err');
      return;
    }

    try {
      setStatus('reading files...');
      pendingKernel = await readFile(kernelFile);
      if (!pendingKernel) {
        setStatus('Choose a kernel first (an NDIX vmunix a.out).', 'err');
        return;
      }
      pendingPseg = await readFile(psegFile);
      pendingDseg = await readFile(dsegFile);

      pendingDisk = await readFile(diskFile);
      if (!pendingDisk && diskSel && diskSel.value) {
        setStatus('reading the root disc out of the library...');
        pendingDisk = await smdStorage.retrieveImage(diskSel.value);
      }

      setStatus('creating the machine...');
      var mb = parseInt(memSel && memSel.value, 10) || 16;
      if (emu.nd500.create(mb * 1024 * 1024) !== 0) {
        setStatus('could not create the ND-500', 'err');
        return;
      }

      if (emu.nd500.loadKernel(pendingKernel) !== 0) {
        setStatus('could not stage the kernel', 'err');
        return;
      }
      // BOTH or NEITHER. With an incomplete pair the library derives the sizes
      // from the a.out header instead, which is what a kernel taken out of a
      // disc image has to do anyway.
      if (pendingPseg && pendingDseg) emu.nd500.loadSegments(pendingPseg, pendingDseg);

      if (pendingDisk) {
        var wr = !!(writableBox && writableBox.checked);
        emu.nd500.mountDisk(0, pendingDisk, wr);
        write('| root disc: ' + pendingDisk.length + ' bytes, ' +
              (wr ? 'WRITABLE' : 'read-only') + '\n');
      } else {
        write('| no root disc - the kernel will boot and then have nothing to mount\n');
      }

      setStatus('booting...');
      var rc = emu.nd500.boot();
      drain();
      if (rc !== 0) { setStatus('boot failed - see the log above', 'err'); return; }

      booted = true;
      if (bootBtn) bootBtn.disabled = true;
      if (stopBtn) { stopBtn.disabled = false; stopBtn.textContent = 'Pause'; }
      setStatus('running', 'ok');
      start();
    } catch (e) {
      setStatus('error: ' + (e && e.message ? e.message : e), 'err');
    }
  }

  // ---- keyboard -----------------------------------------------------------

  // Typing goes to guest tty unit 0 (the console). Only when the window has
  // focus, or every keystroke meant for the ND-100 terminal would be copied
  // here as well.
  function onKey(e) {
    if (!booted || !window.emu || !emu.nd500) return;
    var text = null;
    if (e.key === 'Enter') text = '\r';
    else if (e.key === 'Backspace') text = '\b';
    else if (e.key === 'Tab') text = '\t';
    else if (e.key === 'Escape') text = '\x1b';
    else if (e.key.length === 1) {
      text = e.ctrlKey ? String.fromCharCode(e.key.toUpperCase().charCodeAt(0) & 0x1F)
                       : e.key;
    }
    if (text === null) return;
    e.preventDefault();
    emu.nd500.sendInput(0, text);
  }

  // ---- wiring -------------------------------------------------------------

  function init() {
    win          = el('nd500-window');
    if (!win) return;
    header       = el('nd500-header');
    closeBtn     = el('nd500-close');
    resizeHandle = el('nd500-resize');
    consoleEl    = el('nd500-console');
    statusEl     = el('nd500-status');
    bootBtn      = el('nd500-boot');
    stopBtn      = el('nd500-stop');
    sliceInput   = el('nd500-slice');
    kernelFile   = el('nd500-kernel-file');
    psegFile     = el('nd500-pseg-file');
    dsegFile     = el('nd500-dseg-file');
    diskSel      = el('nd500-disk-select');
    diskFile     = el('nd500-disk-file');
    memSel       = el('nd500-memory');
    writableBox  = el('nd500-writable');

    if (typeof makeDraggable === 'function' && header) makeDraggable(win, header, 'nd500-pos');
    if (typeof makeResizable === 'function' && resizeHandle)
      makeResizable(win, resizeHandle, 'nd500-size', 520, 340);

    if (closeBtn) closeBtn.addEventListener('click', function () {
      // Closing hides the window; it does NOT stop the machine. A guest that
      // vanishes because a window was closed is not what anyone means by close.
      if (typeof closeWindow === 'function') closeWindow('nd500-window');
    });

    var menu = el('menu-nd500');
    if (menu) menu.addEventListener('click', function () {
      if (typeof openWindow === 'function') openWindow('nd500-window');
      refreshLibrary();
      report();
    });

    if (bootBtn) bootBtn.addEventListener('click', boot);
    if (stopBtn) stopBtn.addEventListener('click', function () {
      if (timer) { stop(); setStatus('paused'); }
      else if (booted) { start(); stopBtn.textContent = 'Pause'; setStatus('running', 'ok'); }
    });

    if (consoleEl) {
      consoleEl.setAttribute('tabindex', '0');
      consoleEl.addEventListener('keydown', onKey);
      consoleEl.addEventListener('click', function () { consoleEl.focus(); });
    }
  }

  // Say up front whether this build even has an ND-500, rather than letting
  // Boot fail with it later.
  function report() {
    if (!window.emu || !emu.nd500) { setStatus('the emulator is not ready yet'); return; }
    if (!emu.nd500.available()) {
      setStatus('This build has no ND-500 (built without an nd500x checkout).', 'err');
      if (bootBtn) bootBtn.disabled = true;
      return;
    }
    if (emu.isWorkerMode && emu.isWorkerMode()) {
      setStatus('Worker mode does not forward the ND-500 yet - switch to direct mode.', 'err');
      if (bootBtn) bootBtn.disabled = true;
      return;
    }
    setStatus('ready - choose a kernel and press Boot');
  }

  if (document.readyState === 'loading')
    document.addEventListener('DOMContentLoaded', init);
  else
    init();

  window.nd500Window = { refreshLibrary: refreshLibrary, report: report };
})();
