// machine-setup.js - Machine Setup window for the Glass UI.
//
// Edits the machine configuration as an INI (the same format the native binary
// reads as nd100x.ini). Validation calls the native MachineConfig parser via
// the ValidateMachineINI WASM export, so the browser gets identical, friendly
// error messages. Configurations are kept BY NAME (machine-profiles.js) and can
// be downloaded to run the same machine with the native binary. The selected
// profile is the one toolbar.js hands to emu.init() when the emulator starts.

(function () {
  'use strict';

  function el(id) { return document.getElementById(id); }

  // ---- the profile dropdown ----------------------------------------------
  // Rebuilt from the store on every show and after every change, rather than
  // patched in place: the store is the truth and a list that drifts from it is
  // how you end up saving into the wrong machine.
  function refreshProfiles() {
    var sel = el('machine-setup-profile');
    if (!sel) return;
    var names = machineProfiles.list();
    var active = machineProfiles.activeName();
    sel.innerHTML = '';
    for (var i = 0; i < names.length; i++) {
      var o = document.createElement('option');
      o.value = names[i];
      o.textContent = names[i];
      if (names[i] === active) o.selected = true;
      sel.appendChild(o);
    }
  }

  // Switching machines DISCARDS whatever is in the textarea. Say so, rather
  // than quietly saving it into the machine being left - which is exactly the
  // kind of silent write that loses work.
  function selectProfile() {
    var sel = el('machine-setup-profile');
    if (!sel) return;
    machineProfiles.setActive(sel.value);
    var ta = el('machine-setup-ini');
    if (ta) ta.value = machineProfiles.ini();
    if (window.machineForm) machineForm.load(machineProfiles.ini());
    setResult('Showing "' + sel.value + '". Unsaved edits to the previous machine were discarded.', '');
  }

  function newProfile() {
    var name = prompt('Name for the new machine:', '');
    if (name === null) return;
    // Start from what is on screen: "New" almost always means "like this one,
    // but ...", and starting from the default would throw that away.
    var ta = el('machine-setup-ini');
    var err = machineProfiles.create(name, ta ? ta.value : null);
    if (err) { setResult(err, 'err'); return; }
    refreshProfiles();
    if (ta) ta.value = machineProfiles.ini();
    if (window.machineForm) machineForm.load(machineProfiles.ini());
    setResult('Created "' + name + '".', 'ok');
  }

  function renameProfile() {
    var cur = machineProfiles.activeName();
    var name = prompt('Rename "' + cur + '" to:', cur);
    if (name === null) return;
    var err = machineProfiles.rename(cur, name);
    if (err) { setResult(err, 'err'); return; }
    refreshProfiles();
    setResult('Renamed to "' + name + '".', 'ok');
  }

  function deleteProfile() {
    var cur = machineProfiles.activeName();
    if (!confirm('Delete the machine "' + cur + '"? This cannot be undone.')) return;
    var err = machineProfiles.remove(cur);
    if (err) { setResult(err, 'err'); return; }
    refreshProfiles();
    var ta = el('machine-setup-ini');
    if (ta) ta.value = machineProfiles.ini();
    setResult('Deleted "' + cur + '".', 'ok');
  }

  function machineSetupShow() {
    var win = el('machine-setup-window');
    if (!win) return;
    var ta = el('machine-setup-ini');
    refreshProfiles();
    if (ta) ta.value = machineProfiles.ini();
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
    syncFormToTextarea();
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

  // Which view the user is looking at decides what gets saved. Taking the
  // form's answer while the INI is on screen (or the reverse) would throw away
  // whichever one they had just been editing.
  function syncFormToTextarea() {
    var ta = el('machine-setup-ini');
    if (!ta || !window.machineForm || !machineForm.ready() || iniVisible()) return;
    var out = machineForm.toINI(ta.value);
    if (out !== null) ta.value = out;
  }

  function iniVisible() {
    var adv = el('machine-setup-advanced');
    return !!(adv && adv.style.display !== 'none');
  }

  function toggleAdvanced() {
    var adv = el('machine-setup-advanced');
    var btn = el('machine-setup-advanced-toggle');
    if (!adv) return;
    var showing = adv.style.display !== 'none';
    if (!showing) {
      // Going to the INI view: show what the form currently says, so the two
      // views never disagree about the machine in front of you.
      syncFormToTextarea();
      adv.style.display = '';
      if (btn) btn.textContent = 'Hide INI';
    } else {
      adv.style.display = 'none';
      if (btn) btn.textContent = 'Advanced (INI)';
      // Coming back from the INI view: the text is the truth now.
      var ta = el('machine-setup-ini');
      if (ta && window.machineForm) machineForm.load(ta.value);
    }
  }

  function save() {
    var ta = el('machine-setup-ini');
    if (!ta) return;
    syncFormToTextarea();
    // Only persist a valid config so a broken INI can't wedge the machine.
    validate().then(function (ok) {
      if (!ok) { setResult(el('machine-setup-result').textContent + '  (not saved)', 'err'); return; }
      var name = machineProfiles.activeName();
      if (machineProfiles.write(ta.value, name)) setResult('Saved to "' + name + '".', 'ok');
      else setResult('Save failed (browser storage refused).', 'err');
    });
  }

  function reset() {
    var ta = el('machine-setup-ini');
    if (ta) ta.value = machineProfiles.DEFAULT_INI;
    if (window.machineForm) machineForm.load(machineProfiles.DEFAULT_INI);
    setResult('Reset to default (not yet saved).', '');
  }

  function download() {
    var ta = el('machine-setup-ini');
    if (!ta) return;
    syncFormToTextarea();
    var blob = new Blob([ta.value], { type: 'text/plain' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    // Named after the machine, so a folder of downloads is still readable.
    a.download = machineProfiles.activeName().replace(/[^\w.-]+/g, '_') + '.ini';
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
    var p = el('machine-setup-profile'); if (p) p.addEventListener('change', selectProfile);
    var n = el('machine-setup-new');     if (n) n.addEventListener('click', newProfile);
    var rn = el('machine-setup-rename'); if (rn) rn.addEventListener('click', renameProfile);
    var dl = el('machine-setup-delete'); if (dl) dl.addEventListener('click', deleteProfile);
    var adv = el('machine-setup-advanced-toggle'); if (adv) adv.addEventListener('click', toggleAdvanced);
    if (typeof makeDraggable === 'function') {
      var hdr = el('machine-setup-header');
      if (hdr) makeDraggable(el('machine-setup-window'), hdr, 'machine-setup-pos');
    }
    // Resizable, and the size is remembered - this window holds a machine's
    // whole hardware list, which is more than any fixed height suits. Same
    // shared helper the NDFS viewer, printer and paper tape use.
    if (typeof makeResizable === 'function') {
      var rz = el('machine-setup-resize');
      if (rz) makeResizable(el('machine-setup-window'), rz, 'machine-setup-size', 420, 320);
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
