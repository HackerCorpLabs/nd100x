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

  // The INI spells the Winchester "wd"; nobody calls it that out loud. These
  // are for the screen only - every value written to the file is the INI name.
  var PRETTY = {
    floppy: 'Floppy',
    smd:    'SMD disc',
    wd:     'Winchester (ST506)',
    scsi:   'SCSI',
    hdlc:   'HDLC'
  };

  function pretty(type) { return PRETTY[type] || type; }

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

  function esc(s) {
    return String(s == null ? '' : s)
      .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
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
           checkbox(id + '-en', '<b>' + pretty(c.type) + '</b> (thumbwheel ' + c.wheel + ')', c.enabled) +
           '</div>';
      if (c.type === 'hdlc') {
        // NO mode/host/port here, on purpose. A browser cannot open a TCP
        // socket, and the emulator does not pretend otherwise: modem.c builds
        // with MODEM_HAS_NETWORKING undefined under __EMSCRIPTEN__, and
        // wasm_bind_hdlc() in nd100wasm.c puts every HDLC channel on the
        // WebSocket gateway bridge whatever the config asked for. Offering a
        // "server (listen)" mode would be offering something that cannot
        // happen. Enable it or don't - the traffic goes over the gateway.
        // The values themselves are still carried into the written INI, since
        // the native binary reads the same file and there they are real.
        h += '<div class="smd-image-meta" style="opacity:.75;">' +
             'All traffic goes over the <b>gateway</b> (WebSocket). ' +
             'The browser cannot listen on or dial a TCP port itself.';
        if (c.hdlcMode === 'client' && c.hdlcHost)
          h += '<br>Kept for a native run: client ' + esc(c.hdlcHost) + ':' + (c.hdlcPort || 0) + '.';
        else if (c.hdlcPort)
          h += '<br>Kept for a native run: ' + esc(c.hdlcMode || 'server') + ' port ' + c.hdlcPort + '.';
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

    // Everything the machine COULD have and does not. Without this the form
    // can only edit controllers the file already named, so a config that never
    // mentioned Winchester could never grow one - which is most of them, since
    // the default machine is floppy + SMD + SCSI.
    var missing = [];
    for (var t = 0; t < (d.controllerTypes || []).length; t++) {
      var td = d.controllerTypes[t];
      for (var w = td.minWheel; w <= td.maxWheel; w++) {
        var have = false;
        for (var k = 0; k < d.controllers.length; k++)
          if (d.controllers[k].type === td.type && d.controllers[k].wheel === w) have = true;
        if (!have) missing.push({ type: td.type, wheel: w });
      }
    }
    if (missing.length) {
      h += '<div style="display:flex;gap:6px;align-items:center;margin-top:4px;">';
      h += '<select id="mf-add" style="font-size:12px;padding:2px;">';
      for (var m = 0; m < missing.length; m++) {
        var lbl = pretty(missing[m].type);
        // Only say "thumbwheel N" where there is a choice of N to make.
        var multi = false;
        for (var q = 0; q < missing.length; q++)
          if (missing[q].type === missing[m].type && missing[q].wheel !== missing[m].wheel) multi = true;
        if (multi) lbl += ' (thumbwheel ' + missing[m].wheel + ')';
        h += opt(missing[m].type + '.' + missing[m].wheel, lbl, false);
      }
      h += '</select>';
      h += '<button class="smd-action-btn" id="mf-add-btn">Add controller</button>';
      h += '</div>';
    }
    return h;
  }

  // Add the selected controller to the machine being edited, then re-render so
  // it gets its image fields. It goes in ENABLED with empty slots: adding a
  // controller you then have to tick on as well is a step with no meaning, and
  // an empty slot is simply a drive with no disk in it.
  //
  // Nothing is written anywhere yet - Save still generates the INI and the C
  // validator still checks it. That matters for the Winchester in particular:
  // it answers IOX 500-507, the same block as the CDC system disc, so a machine
  // can have one or the other. If this config has a CDC image the validator
  // will say so, in its own words, at Save.
  function addController() {
    if (!current) return;
    var sel = el('mf-add');
    if (!sel || !sel.value) return;
    var parts = sel.value.split('.');
    var type = parts[0], wheel = parseInt(parts[1], 10);

    var td = null;
    for (var i = 0; i < (current.controllerTypes || []).length; i++)
      if (current.controllerTypes[i].type === type) td = current.controllerTypes[i];
    if (!td) return;

    var disks = [];
    for (var j = 0; j < 8; j++) disks.push({ slot: j, present: false, media: 'hdd', image: '' });

    current.controllers.push({
      type: type, wheel: wheel, enabled: true,
      isDisc: td.isDisc, bootable: td.bootable, diskSlots: td.diskSlots,
      disks: disks,
      hdlcMode: 'server', hdlcHost: '', hdlcPort: 5000 + wheel
    });

    // Keep the form's current answers: re-rendering from `current` alone would
    // throw away anything typed since it was loaded.
    var host = el('machine-setup-form');
    var keep = readForm();
    render(current);
    applyForm(keep);
    if (host) host.scrollTop = host.scrollHeight;
  }

  // The form's current values, by element id. Used to survive a re-render.
  function readForm() {
    var host = el('machine-setup-form');
    var out = {};
    if (!host) return out;
    var nodes = host.querySelectorAll('input, select');
    for (var i = 0; i < nodes.length; i++) {
      var n = nodes[i];
      if (!n.id) continue;
      out[n.id] = (n.type === 'checkbox') ? n.checked : n.value;
    }
    return out;
  }

  function applyForm(values) {
    for (var id in values) {
      if (!Object.prototype.hasOwnProperty.call(values, id)) continue;
      var n = el(id);
      if (!n) continue;                       // gone after the re-render
      if (n.type === 'checkbox') n.checked = values[id];
      else n.value = values[id];
    }
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
    // innerHTML replaces the nodes, so the handler is attached here rather than
    // once at startup - there is no button to attach to until now.
    var add = el('mf-add-btn');
    if (add) add.addEventListener('click', addController);
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
        // Straight off the parsed config, not off the screen - the form does
        // not show these (see renderControllers) because they are meaningless
        // in a browser. They are written anyway so the same .ini still works
        // when run natively, where the modem really does open a socket.
        out.push('mode = ' + (c.hdlcMode === 'client' ? 'client' : 'server'));
        if (c.hdlcMode === 'client' && c.hdlcHost) out.push('host = ' + c.hdlcHost);
        if (c.hdlcPort) out.push('port = ' + c.hdlcPort);
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
