// machine-profiles.js - named machine configurations.
//
// The Machine Setup window used to keep ONE configuration, in a single
// localStorage key. You could describe a machine, but not keep two of them -
// so trying an ND-110 with a different disc set meant editing over the config
// you already had and hoping you could type it back.
//
// This is the store behind "multiple machine configs, saved by name". It is
// deliberately just a store: no UI, no emulator calls, no DOM. machine-setup.js
// drives it and toolbar.js reads the active profile when it boots.
//
// SHAPE ON DISK (one key, so a half-written second key cannot desync it):
//
//   nd100x-machine-profiles = {
//     "v": 1,
//     "active": "<name>",
//     "profiles": [ { "name": "<name>", "ini": "<text>" }, ... ]
//   }
//
// An ARRAY, not an object keyed by name: order is what the user sees in the
// dropdown, and object key order is not something to rely on.
//
// The old single key is still WRITTEN with the active profile's INI on every
// save. Nothing else reads it today, but it is the format the native binary
// takes and it costs one line to keep anything that grew up expecting it
// working.

(function () {
  'use strict';

  var KEY     = 'nd100x-machine-profiles';
  var OLD_KEY = 'nd100x-machine-ini';   // the single-config key this replaces

  // Mirrors the shipped nd100x.ini defaults. cpu takes a family number or a
  // model name now (src/cpu/cpu_model.c), which is worth saying here because
  // the comment is the only place a user finds out.
  var DEFAULT_INI =
    '# nd100x machine configuration\n' +
    '# Toggle a device with "enabled = yes|no". Sections are [type.thumbwheel].\n\n' +
    '[machine]\n' +
    'cpu = 100                 ; 100 | 110 | 120, or a model: ND110CX, ND120CX\n\n' +
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

  var DEFAULT_NAME = 'Default';

  // ---- storage ------------------------------------------------------------
  // Every read goes through load(), so a corrupt or absent key produces a
  // usable store rather than an exception halfway up the UI.

  function load() {
    var raw = null;
    try { raw = localStorage.getItem(KEY); } catch (e) {}

    if (raw) {
      try {
        var s = JSON.parse(raw);
        if (s && s.profiles && s.profiles.length) return s;
      } catch (e) {
        // Unparseable: fall through and rebuild. Keeping a broken blob would
        // mean the window never opens again.
      }
    }

    // First run, or a store we could not read. If the old single key is there,
    // that config is the user's real machine - carry it in rather than
    // silently replacing it with the default.
    var old = null;
    try { old = localStorage.getItem(OLD_KEY); } catch (e) {}

    return {
      v: 1,
      active: DEFAULT_NAME,
      profiles: [{ name: DEFAULT_NAME, ini: (old && old.length) ? old : DEFAULT_INI }]
    };
  }

  function save(s) {
    try {
      localStorage.setItem(KEY, JSON.stringify(s));
      // Keep the single-config key pointing at whatever is active.
      var p = find(s, s.active);
      if (p) localStorage.setItem(OLD_KEY, p.ini);
      return true;
    } catch (e) {
      return false;
    }
  }

  function find(s, name) {
    for (var i = 0; i < s.profiles.length; i++)
      if (s.profiles[i].name === name) return s.profiles[i];
    return null;
  }

  // A name has to be usable as a dropdown label and as a thing you type, so
  // trim it and refuse an empty one. Duplicates are refused rather than
  // silently numbered: two profiles called the same thing is a trap.
  function cleanName(name) {
    return (typeof name === 'string') ? name.replace(/^\s+|\s+$/g, '') : '';
  }

  // ---- the API ------------------------------------------------------------

  window.machineProfiles = {

    DEFAULT_INI: DEFAULT_INI,

    /* Names, in display order. */
    list: function () {
      var s = load(), out = [];
      for (var i = 0; i < s.profiles.length; i++) out.push(s.profiles[i].name);
      return out;
    },

    /* The active profile's name. Always one of list(). */
    activeName: function () {
      var s = load();
      return find(s, s.active) ? s.active : s.profiles[0].name;
    },

    /* The INI of <name>, or of the active profile when <name> is omitted.
     * Falls back to the default rather than returning null: a caller about to
     * boot a machine should get a machine. */
    ini: function (name) {
      var s = load();
      var p = find(s, cleanName(name) || s.active) || s.profiles[0];
      return p ? p.ini : DEFAULT_INI;
    },

    /* Switch the active profile. Returns false for an unknown name. */
    setActive: function (name) {
      var s = load(), n = cleanName(name);
      if (!find(s, n)) return false;
      s.active = n;
      return save(s);
    },

    /* Write <ini> into <name> (or the active profile). Returns false only if
     * localStorage refused. */
    write: function (ini, name) {
      var s = load(), n = cleanName(name) || s.active;
      var p = find(s, n);
      if (p) p.ini = ini;
      else s.profiles.push({ name: n, ini: ini });
      s.active = n;
      return save(s);
    },

    /* Create a profile and make it active. Returns an error string, or "". */
    create: function (name, ini) {
      var s = load(), n = cleanName(name);
      if (!n) return 'Give the machine a name.';
      if (find(s, n)) return 'There is already a machine called "' + n + '".';
      s.profiles.push({ name: n, ini: (typeof ini === 'string' && ini.length) ? ini : DEFAULT_INI });
      s.active = n;
      return save(s) ? '' : 'Could not save (browser storage refused).';
    },

    /* Rename, keeping the INI and the active selection. Returns "" or why not. */
    rename: function (oldName, newName) {
      var s = load(), o = cleanName(oldName), n = cleanName(newName);
      if (!n) return 'Give the machine a name.';
      var p = find(s, o);
      if (!p) return 'No machine called "' + o + '".';
      if (n !== o && find(s, n)) return 'There is already a machine called "' + n + '".';
      p.name = n;
      if (s.active === o) s.active = n;
      return save(s) ? '' : 'Could not save (browser storage refused).';
    },

    /* Delete. The last profile is NOT deletable - an empty list would leave
     * the window with nothing to show and the boot with nothing to use. */
    remove: function (name) {
      var s = load(), n = cleanName(name);
      if (s.profiles.length <= 1) return 'This is the only machine - keep at least one.';
      var idx = -1;
      for (var i = 0; i < s.profiles.length; i++) if (s.profiles[i].name === n) idx = i;
      if (idx < 0) return 'No machine called "' + n + '".';
      s.profiles.splice(idx, 1);
      if (s.active === n) s.active = s.profiles[0].name;
      return save(s) ? '' : 'Could not save (browser storage refused).';
    }
  };
})();
