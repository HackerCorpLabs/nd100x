// machine-form.js - the Machine Setup form.
//
// The window used to be a raw INI textarea. Fine if you already know the key
// names; not much use if you just want an ND-110 with two discs.
//
// TWO RULES SHAPE THIS FILE.
//
// 1. READING GOES THROUGH THE C PARSER. The form never interprets INI itself.
//    It calls DescribeMachineINI (src/frontend/nd100wasm), which runs the same
//    MachineConfig_LoadFile the native binary runs and hands back JSON. A
//    second parser written in JavaScript would drift from the first, and then
//    the form would show one machine while the emulator built another.
//
// 2. WRITING PRESERVES WHAT IT DOES NOT UNDERSTAND. The form regenerates only
//    the sections it owns - [machine], [controller.*], [terminals],
//    [peripheral.*], [boot] - and carries every other section over verbatim.
//    [runtime] alone holds the telnet port, throttle, charset, print and tape
//    directories, the drum and CDC images and the memory size. A form that
//    regenerated the file from scratch would delete all of it the first time
//    somebody clicked Save, and nothing would say so.
//
// The generated text still goes through the C validator before it is saved, so
// a form that produces nonsense is caught by the same check a hand-typed file
// gets.

(function () {
  'use strict';

  function el(id) { return document.getElementById(id); }

  // ---- INI section surgery ------------------------------------------------
  // Deliberately NOT a parser: it splits on section headers and nothing else.
  // Understanding keys is the C code's job (rule 1); all this needs to know is
  // where one section stops and the next begins.

  var OWNED = /^\[(machine|controller\.|terminals|peripheral\.|boot)/i;

  function foreignSections(ini) {
    var lines = (ini || '').split('\n');
    var out = [], keep = false, cur = [];
    for (var i = 0; i < lines.length; i++) {
      var m = /^\s*\[([^\]]+)\]/.exec(lines[i]);
      if (m) {
        if (keep && cur.length) out.push(cur.join('\n'));
        cur = [];
        keep = !OWNED.test('[' + m[1]);
      }
      if (keep) cur.push(lines[i]);
    }
    if (keep && cur.length) out.push(cur.join('\n'));
    return out;
  }

  // ---- rendering ----------------------------------------------------------

  var current = null;   // the last JSON description, for regenerating

  function opt(value, label, selected) {
    return '<option value="' + value + '"' + (selected ? ' selected' : '') + '>' +
           (label || value) + '</option>';
  }

  function checkbox(id, label, checked) {
    return '<label style="display:inline-flex;align-items:center;gap:4px;margin-right:12px;">' +
           '<input type="checkbox" id="' + id + '"' + (checked ? ' checked' : '') + '> ' +
           label + '</label>';
  }

  function textInput(id, value, width) {
    return '<input type="text" id="' + id + '" value="' + (value || '').replace(/"/g, '&quot;') +
           '" style="width:' + (width || '150px') + ';font-size:12px;padding:2px;">';
  }

  function renderMachine(d) {
    var h = '<div class="smd-section-title">CPU and clock</div><div style="margin-bottom:10px;">';
    h += '<label style="margin-right:12px;">Model ';
    h += '<select id="mf-cpu" style="font-size:12px;padding:2px;">';
    // Straight from the CPU's own table (cpu_model.c), so the list cannot go
    // stale when a model is added to the enum.
    for (var i = 0; i < d.cpuModels.length; i++)
      h += opt(d.cpuModels[i], d.cpuModels[i], d.cpuModels[i] === d.machine.cpu);
    h += '</select></label>';
    h += '<label style="margin-right:12px;">FPP <select id="mf-fpp" style="font-size:12px;padding:2px;">' +
         opt('48', '48-bit', d.machine.fpp === 48) + opt('32', '32-bit', d.machine.fpp === 32) +
         '</select></label>';
    h += '<label>RTC <select id="mf-rtc" style="font-size:12px;padding:2px;">' +
         opt('ticks', 'instruction ticks', d.machine.rtc === 'ticks') +
         opt('wall', 'wall clock 20 ms', d.machine.rtc === 'wall') +
         '</select></label>';
    h += '</div>';
    return h;
  }

  function renderControllers(d) {
    var h = '<div class="smd-section-title">Controllers</div>';
    if (!d.controllers.length)
      h += '<div class="smd-image-meta" style="opacity:.7;">None in this configuration.</div>';
    for (var i = 0; i < d.controllers.length; i++) {
      var c = d.controllers[i];
      var id = 'mf-c' + i;
      h += '<div style="border:1px solid rgba(255,255,255,.12);border-radius:4px;padding:6px;margin-bottom:6px;">';
      h += '<div style="margin-bottom:4px;">' +
           checkbox(id + '-en', '<b>' + c.type + '</b> (thumbwheel ' + c.wheel + ')', c.enabled) +
           '</div>';
      if (c.type === 'hdlc') {
        h += '<div style="display:flex;gap:8px;flex-wrap:wrap;align-items:center;">';
        h += '<label>Mode <select id="' + id + '-mode" style="font-size:12px;padding:2px;">' +
             opt('server', 'server (listen)', c.hdlcMode === 'server') +
             opt('client', 'client', c.hdlcMode === 'client') + '</select></label>';
        h += '<label>Host ' + textInput(id + '-host', c.hdlcHost, '130px') + '</label>';
        h += '<label>Port ' + textInput(id + '-port', String(c.hdlcPort || ''), '70px') + '</label>';
        h += '</div>';
      } else if (c.diskSlots > 0) {
        for (var j = 0; j < c.diskSlots && j < c.disks.length; j++) {
          var k = c.disks[j];
          h += '<div style="display:flex;gap:6px;align-items:center;margin-top:3px;">';
          h += '<span style="opacity:.7;width:52px;">disk' + j + '</span>';
          h += textInput(id + '-d' + j, k.present ? k.image : '', '190px');
          if (c.type === 'scsi') {
            h += '<select id="' + id + '-m' + j + '" style="font-size:12px;padding:2px;">' +
                 opt('hdd', 'hdd', k.media === 'hdd') +
                 opt('cdrom', 'cdrom', k.media === 'cdrom') +
                 opt('tape', 'tape', k.media === 'tape') +
                 opt('floppy', 'floppy', k.media === 'floppy') + '</select>';
          }
          h += '</div>';
        }
        h += '<div class="smd-image-meta" style="opacity:.6;margin-top:3px;">Leave a slot empty for no disk.</div>';
      }
      h += '</div>';
    }
    return h;
  }

  function renderRest(d) {
    var h = '<div class="smd-section-title">Terminals</div>';
    h += '<div style="margin-bottom:10px;">' +
         '<label>Thumbwheels ' + textInput('mf-terminals', d.terminals.join(', '), '220px') + '</label>' +
         '<div class="smd-image-meta" style="opacity:.6;">The console is always there; these are the extra lines.</div>' +
         '</div>';

    h += '<div class="smd-section-title">Peripherals</div><div style="margin-bottom:10px;">';
    h += checkbox('mf-ptr', 'Paper tape reader', d.peripherals.papertapeReader);
    h += checkbox('mf-ptp', 'Paper tape punch', d.peripherals.papertapePunch);
    h += checkbox('mf-lpt', 'Line printer', d.peripherals.linePrinter);
    h += '</div>';

    h += '<div class="smd-section-title">Boot from</div><div style="margin-bottom:6px;">';
    h += '<select id="mf-boot" style="font-size:12px;padding:2px;min-width:180px;">';
    // Only bootable disc controllers that are actually in this machine: a boot
    // device naming a controller the config does not have is rejected by the
    // validator, so there is no point offering it.
    var found = false;
    for (var i = 0; i < d.controllers.length; i++) {
      var c = d.controllers[i];
      if (!c.bootable || !c.isDisc) continue;
      for (var j = 0; j < c.diskSlots && j < c.disks.length; j++) {
        var v = c.type + '.' + c.wheel + '.' + j;
        var sel = d.boot.isDisc && d.boot.type === c.type &&
                  d.boot.wheel === c.wheel && d.boot.unit === j;
        if (sel) found = true;
        h += opt(v, v + (c.disks[j].present ? '  (' + c.disks[j].image + ')' : '  (empty)'), sel);
      }
    }
    if (!found && d.boot.isDisc) {
      var v0 = d.boot.type + '.' + d.boot.wheel + '.' + d.boot.unit;
      h = h.replace('<select id="mf-boot"', '<select id="mf-boot"');
      h += opt(v0, v0 + '  (not in this machine)', true);
    }
    h += '</select></div>';
    return h;
  }

  function render(d) {
    var host = el('machine-setup-form');
    if (!host) return;
    if (d.error) {
      host.innerHTML = '<div class="smd-image-meta" style="color:#FF8A8A;">' +
        'This configuration cannot be shown as a form: ' + d.error +
        '<br>Fix it in Advanced (INI) below.</div>';
      current = null;
      return;
    }
    current = d;
    host.innerHTML = renderMachine(d) + renderControllers(d) + renderRest(d);
  }

  // ---- form -> INI --------------------------------------------------------

  function val(id, dflt) { var e = el(id); return e ? e.value : (dflt || ''); }
  function chk(id) { var e = el(id); return !!(e && e.checked); }

  function toINI(previousIni) {
    if (!current) return null;
    var d = current, out = [];

    out.push('# nd100x machine configuration');
    out.push('# Written by the Machine Setup form. Sections are [type.thumbwheel].');
    out.push('');
    out.push('[machine]');
    out.push('cpu = ' + val('mf-cpu', 'ND100'));
    out.push('fpp = ' + val('mf-fpp', '48'));
    out.push('rtc = ' + val('mf-rtc', 'ticks'));
    out.push('');

    for (var i = 0; i < d.controllers.length; i++) {
      var c = d.controllers[i], id = 'mf-c' + i;
      out.push('[controller.' + c.type + '.' + c.wheel + ']');
      out.push('enabled = ' + (chk(id + '-en') ? 'yes' : 'no'));
      if (c.type === 'hdlc') {
        out.push('mode = ' + val(id + '-mode', 'server'));
        var host = val(id + '-host', '');
        if (host) out.push('host = ' + host);
        var port = val(id + '-port', '');
        if (port) out.push('port = ' + port);
      } else {
        for (var j = 0; j < c.diskSlots && j < c.disks.length; j++) {
          var img = val(id + '-d' + j, '');
          if (!img) continue;
          // SCSI slots carry a media type; everything else is a plain path.
          if (c.type === 'scsi') out.push('disk' + j + ' = ' + val(id + '-m' + j, 'hdd') + ':' + img);
          else                   out.push('disk' + j + ' = ' + img);
        }
      }
      out.push('');
    }

    out.push('[terminals]');
    out.push('enabled = ' + val('mf-terminals', ''));
    out.push('');

    out.push('[peripheral.papertape-reader]');
    out.push('enabled = ' + (chk('mf-ptr') ? 'yes' : 'no'));
    out.push('');
    out.push('[peripheral.papertape-punch]');
    out.push('enabled = ' + (chk('mf-ptp') ? 'yes' : 'no'));
    out.push('');
    out.push('[peripheral.lineprinter]');
    out.push('enabled = ' + (chk('mf-lpt') ? 'yes' : 'no'));
    out.push('');

    out.push('[boot]');
    out.push('device = ' + val('mf-boot', ''));
    out.push('');

    // Rule 2: everything the form does not own comes across untouched.
    var carried = foreignSections(previousIni);
    if (carried.length) {
      out.push('# Sections below are not edited by the form and are kept as they were.');
      out.push('');
      for (var k = 0; k < carried.length; k++) { out.push(carried[k]); out.push(''); }
    }

    return out.join('\n');
  }

  // ---- public -------------------------------------------------------------

  window.machineForm = {

    /* Show <ini> as a form. Asks the C parser what it means; on a parse error
     * says so and leaves the user with the INI view, which is where a broken
     * config has to be fixed anyway. */
    load: function (ini) {
      if (typeof emu === 'undefined' || !emu.describeMachineINI) {
        var host = el('machine-setup-form');
        if (host) host.innerHTML = '<div class="smd-image-meta" style="opacity:.7;">' +
          'The form needs the emulator module (not loaded yet). Use Advanced (INI) below.</div>';
        current = null;
        return Promise.resolve(false);
      }
      return Promise.resolve(emu.describeMachineINI(ini)).then(function (json) {
        var d;
        try { d = JSON.parse(json); }
        catch (e) { d = { error: 'could not read the machine description' }; }
        render(d);
        return !d.error;
      });
    },

    /* The form as INI, with unowned sections carried over from <previousIni>.
     * null when there is nothing rendered - the caller keeps its own text. */
    toINI: toINI,

    /* Is there a rendered form to read? */
    ready: function () { return !!current; },

    /* Exposed for tests/test_machine_form.js. This is the one piece of INI
     * handling that lives in JavaScript, and getting it wrong deletes
     * [runtime] - the telnet port, throttle, charset, drum and CDC images and
     * the memory size - the first time somebody presses Save. It is worth a
     * test of its own. */
    _foreignSections: foreignSections
  };
})();
