// machine-setup.js - Machine Setup window for the Glass UI.
//
// Edits the machine configuration as an INI (the same format the native binary
// reads as nd100x.ini). Validation calls the native MachineConfig parser via
// the ValidateMachineINI WASM export, so the browser gets identical, friendly
// error messages. The INI is persisted in localStorage and can be downloaded to
// run the same machine with the native binary.

(function () {
  'use strict';

  var STORAGE_KEY = 'nd100x-machine-ini';

  // Default machine INI - mirrors the shipped nd100x.ini defaults.
  var DEFAULT_INI =
    '# nd100x machine configuration\n' +
    '# Toggle a device with "enabled = yes|no". Sections are [type.thumbwheel].\n\n' +
    '[machine]\n' +
    'cpu = 100                 ; 100 | 110 | 120\n\n' +
    '[controller.floppy.0]\n' +
    'enabled = yes\n' +
    'disk0 = FLOPPY.IMG\n\n' +
    '[controller.smd.0]\n' +
    'enabled = yes\n' +
    'disk0 = SMD0.IMG\n\n' +
    '[controller.scsi.0]\n' +
    'enabled = yes\n' +
    'disk0 = hdd:SCSI0.IMG     ; SCSI ID 0 (media hdd)\n\n' +
    '[terminals]\n' +
    'enabled = 5, 6, 7, 8, 9, 10, 11\n\n' +
    '[boot]\n' +
    'device = smd.0.0          ; <type>.<wheel>.<unit>\n';

  function el(id) { return document.getElementById(id); }

  function currentINI() {
    var stored = null;
    try { stored = localStorage.getItem(STORAGE_KEY); } catch (e) {}
    return (stored && stored.length) ? stored : DEFAULT_INI;
  }

  function machineSetupShow() {
    var win = el('machine-setup-window');
    if (!win) return;
    var ta = el('machine-setup-ini');
    if (ta) ta.value = currentINI();
    setResult('', '');
    win.style.display = 'flex';
    if (typeof windowManager !== 'undefined') windowManager.focus('machine-setup-window');
  }

  function machineSetupHide() {
    var win = el('machine-setup-window');
    if (win) win.style.display = 'none';
  }

  function setResult(text, kind) {
    var r = el('machine-setup-result');
    if (!r) return;
    r.textContent = text;
    r.style.color = (kind === 'ok') ? '#7CFC7C' : (kind === 'err') ? '#FF8A8A' : '';
  }

  // Validate via the native parser (through the emu proxy). Returns a Promise.
  function validate() {
    var ta = el('machine-setup-ini');
    if (!ta) return Promise.resolve(false);
    if (typeof emu === 'undefined' || !emu.validateMachineINI) {
      setResult('Validation unavailable (emulator not ready).', 'err');
      return Promise.resolve(false);
    }
    setResult('Validating...', '');
    return Promise.resolve(emu.validateMachineINI(ta.value)).then(function (msg) {
      if (!msg) { setResult('Configuration is valid.', 'ok'); return true; }
      setResult(msg, 'err');
      return false;
    }).catch(function (e) {
      setResult('Validation error: ' + (e && e.message ? e.message : e), 'err');
      return false;
    });
  }

  function save() {
    var ta = el('machine-setup-ini');
    if (!ta) return;
    // Only persist a valid config so a broken INI can't wedge the machine.
    validate().then(function (ok) {
      if (!ok) { setResult(el('machine-setup-result').textContent + '  (not saved)', 'err'); return; }
      try { localStorage.setItem(STORAGE_KEY, ta.value); setResult('Saved.', 'ok'); }
      catch (e) { setResult('Save failed: ' + e.message, 'err'); }
    });
  }

  function reset() {
    var ta = el('machine-setup-ini');
    if (ta) ta.value = DEFAULT_INI;
    setResult('Reset to default (not yet saved).', '');
  }

  function download() {
    var ta = el('machine-setup-ini');
    if (!ta) return;
    var blob = new Blob([ta.value], { type: 'text/plain' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = 'nd100x.ini';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function () { URL.revokeObjectURL(url); }, 1000);
  }

  function wire() {
    var menu = el('menu-machine-setup');
    if (menu) menu.addEventListener('click', machineSetupShow);
    var close = el('machine-setup-close');
    if (close) close.addEventListener('click', machineSetupHide);
    var v = el('machine-setup-validate'); if (v) v.addEventListener('click', validate);
    var s = el('machine-setup-save');     if (s) s.addEventListener('click', save);
    var r = el('machine-setup-reset');    if (r) r.addEventListener('click', reset);
    var d = el('machine-setup-download'); if (d) d.addEventListener('click', download);
    if (typeof makeDraggable === 'function') {
      var hdr = el('machine-setup-header');
      if (hdr) makeDraggable(el('machine-setup-window'), hdr, 'machine-setup-pos');
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', wire);
  } else {
    wire();
  }

  // Expose for other modules / tests.
  window.machineSetupShow = machineSetupShow;
  window.machineSetupHide = machineSetupHide;
  window.machineSetupValidate = validate;
})();
