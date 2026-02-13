//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// toolbar.js - Toolbar menus, button wiring, window management, draggable helper

// =========================================================
// Reusable drag helper for glass windows
// =========================================================
// Map window element IDs to localStorage keys for position persistence
var windowStorageKeys = {};

// Save a window's current position to localStorage
function saveWindowPosition(winOrId) {
  var el = (typeof winOrId === 'string') ? document.getElementById(winOrId) : winOrId;
  if (!el) return;
  var key = windowStorageKeys[el.id];
  if (!key) return;
  try {
    localStorage.setItem(key, JSON.stringify({
      left: el.style.left,
      top: el.style.top
    }));
  } catch(e) {}
}

// Restore a window's position from localStorage
function restoreWindowPosition(winOrId) {
  var el = (typeof winOrId === 'string') ? document.getElementById(winOrId) : winOrId;
  if (!el) return;
  var key = windowStorageKeys[el.id];
  if (!key) return;
  try {
    var pos = JSON.parse(localStorage.getItem(key));
    if (pos && pos.left && pos.top) {
      var left = Math.max(0, Math.min(parseInt(pos.left) || 0, window.innerWidth - 200));
      var top = Math.max(49, Math.min(parseInt(pos.top) || 0, window.innerHeight - 100));
      el.style.left = left + 'px';
      el.style.top = top + 'px';
      el.style.bottom = 'auto';
      el.style.right = 'auto';
      el.style.transform = 'none';
    }
  } catch(e) {}
}

function makeDraggable(win, header, storageKey) {
  var dragging = false, ox = 0, oy = 0;
  var toolbarH = 49; // toolbar height (48px + 1px border)

  if (storageKey) windowStorageKeys[win.id] = storageKey;

  header.addEventListener('mousedown', function(e) {
    if (e.target.closest('button') || e.target.closest('select')) return;
    if (win.classList.contains('maximized')) return;
    dragging = true;
    ox = e.clientX - win.getBoundingClientRect().left;
    oy = e.clientY - win.getBoundingClientRect().top;
    e.preventDefault();
  });

  document.addEventListener('mousemove', function(e) {
    if (!dragging) return;
    win.style.left = Math.max(0, Math.min(window.innerWidth - 100, e.clientX - ox)) + 'px';
    win.style.top = Math.max(toolbarH, Math.min(window.innerHeight - 60, e.clientY - oy)) + 'px';
    win.style.bottom = 'auto';
    win.style.right = 'auto';
    win.style.transform = 'none';
  });

  document.addEventListener('mouseup', function() {
    if (dragging) {
      dragging = false;
      saveWindowPosition(win);
    }
  });

  // Restore position
  restoreWindowPosition(win);
}

// =========================================================
// Window Manager - click-to-focus and taskbar
// =========================================================
var windowManager = {
  zCounter: 7000,
  windows: {},
  focusedId: null,

  register: function(id, name) {
    var el = document.getElementById(id);
    if (!el) return;
    this.windows[id] = { name: name, element: el };
    el.style.zIndex = this.zCounter;
    var self = this;
    el.addEventListener('mousedown', function() {
      self.focus(id);
    });
  },

  focus: function(id) {
    var entry = this.windows[id];
    if (!entry) return;
    // Cap at 8899 then reset all to base
    if (this.zCounter >= 8899) {
      this.zCounter = 7000;
      for (var wid in this.windows) {
        this.windows[wid].element.style.zIndex = 7000;
      }
    }
    this.zCounter++;
    entry.element.style.zIndex = this.zCounter;
    this.focusedId = id;
    this.updateTaskbarActive();
  },

  updateTaskbarActive: function() {
    var taskbar = document.getElementById('window-taskbar');
    if (!taskbar) return;
    var btns = taskbar.querySelectorAll('.taskbar-btn');
    for (var i = 0; i < btns.length; i++) {
      var btn = btns[i];
      if (btn.getAttribute('data-win-id') === this.focusedId) {
        btn.classList.add('active');
      } else {
        btn.classList.remove('active');
      }
    }
  },

  updateTaskbar: function() {
    var taskbar = document.getElementById('window-taskbar');
    if (!taskbar) return;

    // Find which window currently has highest z-index among visible
    var topZ = -1;
    var topId = null;
    var visibleWindows = [];

    for (var id in this.windows) {
      var entry = this.windows[id];
      var el = entry.element;
      var style = window.getComputedStyle(el);
      if (style.display !== 'none') {
        visibleWindows.push(id);
        var z = parseInt(el.style.zIndex) || 0;
        if (z > topZ) {
          topZ = z;
          topId = id;
        }
      }
    }

    this.focusedId = topId;

    // Rebuild buttons only if window set changed
    var currentIds = [];
    var existingBtns = taskbar.querySelectorAll('.taskbar-btn');
    for (var i = 0; i < existingBtns.length; i++) {
      currentIds.push(existingBtns[i].getAttribute('data-win-id'));
    }

    var same = (currentIds.length === visibleWindows.length);
    if (same) {
      for (var j = 0; j < visibleWindows.length; j++) {
        if (currentIds.indexOf(visibleWindows[j]) === -1) {
          same = false;
          break;
        }
      }
    }

    if (!same) {
      taskbar.innerHTML = '';
      var self = this;
      for (var k = 0; k < visibleWindows.length; k++) {
        (function(wid) {
          var btn = document.createElement('button');
          btn.className = 'taskbar-btn';
          btn.setAttribute('data-win-id', wid);
          btn.textContent = self.windows[wid].name;
          btn.addEventListener('click', function() {
            var el = self.windows[wid].element;
            if (window.getComputedStyle(el).display === 'none') {
              el.style.display = 'flex';
            }
            // If window is outside visible viewport, move it to top-left
            var rect = el.getBoundingClientRect();
            var vw = window.innerWidth;
            var vh = window.innerHeight;
            if (rect.right < 40 || rect.bottom < 40 || rect.left > vw - 40 || rect.top > vh - 40) {
              el.style.left = '20px';
              el.style.top = '60px';
              saveWindowPosition(el);
            }
            self.focus(wid);
          });
          taskbar.appendChild(btn);
        })(visibleWindows[k]);
      }
    }

    this.updateTaskbarActive();
  }
};

// =========================================================
// Toolbar dropdown menus
// =========================================================
(function() {
  'use strict';

  function closeAllMenus() {
    document.querySelectorAll('.toolbar-menu-container').forEach(function(c) {
      c.classList.remove('open');
    });
  }

  // Toggle menu on trigger click
  document.querySelectorAll('.toolbar-menu-trigger').forEach(function(trigger) {
    trigger.addEventListener('click', function(e) {
      e.stopPropagation();
      var container = trigger.closest('.toolbar-menu-container');
      var wasOpen = container.classList.contains('open');
      closeAllMenus();
      if (!wasOpen) container.classList.add('open');
    });
  });

  // Close on click outside
  document.addEventListener('click', function() {
    closeAllMenus();
  });

  // Close on Escape
  document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape') closeAllMenus();
  });

  // Prevent menu clicks from closing
  document.querySelectorAll('.toolbar-menu').forEach(function(menu) {
    menu.addEventListener('click', function(e) {
      e.stopPropagation();
    });
  });
})();

// =========================================================
// Glass window toggle helpers
// =========================================================
function saveWindowVisibility(windowId, visible) {
  try {
    var state = JSON.parse(localStorage.getItem('window-visibility') || '{}');
    state[windowId] = visible;
    localStorage.setItem('window-visibility', JSON.stringify(state));
  } catch(e) {}
}

function closeWindow(windowId) {
  var win = document.getElementById(windowId);
  if (win) win.style.display = 'none';
  saveWindowVisibility(windowId, false);
}

function openWindow(windowId) {
  var win = document.getElementById(windowId);
  if (win) {
    win.style.display = 'flex';
    windowManager.focus(windowId);
  }
  saveWindowVisibility(windowId, true);
}

function toggleGlassWindow(windowId) {
  var win = document.getElementById(windowId);
  if (!win) return;
  if (win.style.display === 'none' || win.style.display === '') {
    openWindow(windowId);
  } else {
    closeWindow(windowId);
  }
}

// =========================================================
// View menu handlers
// =========================================================
document.getElementById('menu-machine-info').addEventListener('click', function() {
  toggleGlassWindow('machine-window');
  document.querySelector('.toolbar-menu-container').classList.remove('open');
});

document.getElementById('menu-cpu-load-graph').addEventListener('click', function() {
  toggleGlassWindow('cpu-load-window');
  document.querySelector('.toolbar-menu-container').classList.remove('open');
});

document.getElementById('menu-floppy-library').addEventListener('click', function() {
  var floppyModal = document.getElementById('floppy-modal');
  if (floppyModal.style.display === 'none' || floppyModal.style.display === '') {
    openFloppyBrowser();
    windowManager.focus('floppy-modal');
  } else {
    closeFloppyBrowser();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-debugger').addEventListener('click', function() {
  var dbgWin = document.getElementById('debugger-window');
  if (dbgWin.style.display === 'none' || dbgWin.style.display === '') {
    dbgWin.style.display = 'flex';
    if (typeof dbgShowWindow === 'function') dbgShowWindow();
    windowManager.focus('debugger-window');
    saveWindowVisibility('debugger-window', true);
  } else {
    dbgWin.style.display = 'none';
    if (typeof dbgHideWindow === 'function') dbgHideWindow();
    saveWindowVisibility('debugger-window', false);
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-disassembly').addEventListener('click', function() {
  var disasmWin = document.getElementById('disasm-window');
  if (disasmWin.style.display === 'none' || disasmWin.style.display === '') {
    if (typeof disasmShowWindow === 'function') disasmShowWindow();
    windowManager.focus('disasm-window');
  } else {
    if (typeof disasmHideWindow === 'function') disasmHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('disasm-window-close').addEventListener('click', function() {
  if (typeof disasmHideWindow === 'function') disasmHideWindow();
});

document.getElementById('menu-breakpoints').addEventListener('click', function() {
  var bpWin = document.getElementById('breakpoints-window');
  if (bpWin.style.display === 'none' || bpWin.style.display === '') {
    if (typeof bpShowWindow === 'function') bpShowWindow();
    windowManager.focus('breakpoints-window');
  } else {
    if (typeof bpHideWindow === 'function') bpHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('breakpoints-window-close').addEventListener('click', function() {
  if (typeof bpHideWindow === 'function') bpHideWindow();
});

// =========================================================
// Help menu handlers
// =========================================================
document.getElementById('menu-sintran-help').addEventListener('click', function() {
  openHelpWindow();
  windowManager.focus('help-window');
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-about').addEventListener('click', function() {
  openWindow('about-window');
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

// About window close
document.getElementById('about-close').addEventListener('click', function() {
  closeWindow('about-window');
});

// Config window close
document.getElementById('config-window-close').addEventListener('click', function() {
  closeWindow('config-window');
});

// Taskbar cogwheel - toggle Config window (taskbar element is below scripts in DOM)
document.addEventListener('DOMContentLoaded', function() {
  var settingsBtn = document.getElementById('taskbar-settings');
  if (settingsBtn) {
    settingsBtn.addEventListener('click', function() {
      toggleGlassWindow('config-window');
    });
  }
});

// =========================================================
// Config: Float terminals toggle
// =========================================================
(function() {
  var groupToggle = document.getElementById('config-group-terminals');
  var popoutToggle = document.getElementById('config-auto-popout');
  var popoutRow = document.getElementById('config-auto-popout-row');
  if (!groupToggle) return;

  function updatePopoutState() {
    var grouped = groupToggle.checked;
    if (popoutRow) popoutRow.style.opacity = grouped ? '0.4' : '1';
    if (popoutToggle) popoutToggle.disabled = grouped;
  }

  // Initialize state - "Group" ON means float-mode OFF (inverted)
  groupToggle.checked = (localStorage.getItem('terminal-float-mode') === 'false');
  if (popoutToggle) {
    popoutToggle.checked = (localStorage.getItem('terminal-auto-popout') === 'true');
  }
  updatePopoutState();

  groupToggle.addEventListener('change', function() {
    // Inverted: group ON = float OFF
    localStorage.setItem('terminal-float-mode', groupToggle.checked ? 'false' : 'true');
    updatePopoutState();
    if (typeof switchTerminalMode === 'function') {
      switchTerminalMode();
    }
  });

  if (popoutToggle) {
    popoutToggle.addEventListener('change', function() {
      localStorage.setItem('terminal-auto-popout', popoutToggle.checked ? 'true' : 'false');
    });
  }
})();

// =========================================================
// Config: Worker mode toggle
// =========================================================
(function() {
  var toggle = document.getElementById('config-worker-mode');
  if (!toggle) return;

  // Initialize from localStorage (same key used by USE_WORKER detection)
  toggle.checked = (localStorage.getItem('nd100x-worker') === 'true');

  toggle.addEventListener('change', function() {
    localStorage.setItem('nd100x-worker', toggle.checked ? 'true' : 'false');
    // Worker mode requires page reload to take effect
    if (confirm('Worker mode change requires a page reload. Reload now?')) {
      location.reload();
    }
  });
})();

// =========================================================
// SINTRAN menu handlers - debug windows
// =========================================================
document.getElementById('menu-process-list').addEventListener('click', function() {
  var win = document.getElementById('process-list-window');
  if (win.style.display === 'none' || win.style.display === '') {
    if (typeof procListShowWindow === 'function') procListShowWindow();
    windowManager.focus('process-list-window');
  } else {
    if (typeof procListHideWindow === 'function') procListHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-queue-viewer').addEventListener('click', function() {
  var win = document.getElementById('queue-viewer-window');
  if (win.style.display === 'none' || win.style.display === '') {
    if (typeof queueViewerShowWindow === 'function') queueViewerShowWindow();
    windowManager.focus('queue-viewer-window');
  } else {
    if (typeof queueViewerHideWindow === 'function') queueViewerHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-segment-table').addEventListener('click', function() {
  var win = document.getElementById('segment-table-window');
  if (win.style.display === 'none' || win.style.display === '') {
    if (typeof segTableShowWindow === 'function') segTableShowWindow();
    windowManager.focus('segment-table-window');
  } else {
    if (typeof segTableHideWindow === 'function') segTableHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-io-devices').addEventListener('click', function() {
  var win = document.getElementById('io-devices-window');
  if (win.style.display === 'none' || win.style.display === '') {
    if (typeof ioDevicesShowWindow === 'function') ioDevicesShowWindow();
    windowManager.focus('io-devices-window');
  } else {
    if (typeof ioDevicesHideWindow === 'function') ioDevicesHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

document.getElementById('menu-page-tables').addEventListener('click', function() {
  var win = document.getElementById('page-table-window');
  if (win.style.display === 'none' || win.style.display === '') {
    if (typeof pageTableShowWindow === 'function') pageTableShowWindow();
    windowManager.focus('page-table-window');
  } else {
    if (typeof pageTableHideWindow === 'function') pageTableHideWindow();
  }
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
});

// =========================================================
// Copy-to-clipboard button handlers
// =========================================================
document.getElementById('proc-list-copy').addEventListener('click', function() {
  copyTableToClipboard('proc-list-body', 'Process List');
});

document.getElementById('queue-viewer-copy').addEventListener('click', function() {
  // Copy the active tab's table
  var activeContent = document.querySelector('#queue-viewer-window .queue-tab-content.active');
  if (activeContent) {
    var tabName = activeContent.getAttribute('data-tab');
    var titles = { exec: 'Execution Queue', time: 'Time Queue', mon: 'Monitor Queue' };
    copyTableToClipboard(activeContent.id, titles[tabName] || 'Queue');
  }
});

document.getElementById('segment-table-copy').addEventListener('click', function() {
  copyTableToClipboard('segment-table-body', 'Segment Table');
});

document.getElementById('io-devices-copy').addEventListener('click', function() {
  copyTableToClipboard('io-devices-body', 'I/O Devices');
});

document.getElementById('page-table-copy').addEventListener('click', function() {
  var ptSelect = document.getElementById('pt-select');
  var ptLabel = ptSelect ? 'Page Table ' + ptSelect.options[ptSelect.selectedIndex].text : 'Page Table';
  copyTableToClipboard('pt-table-body', ptLabel);
});

document.getElementById('menu-dump-memory').addEventListener('click', function() {
  document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
  if (typeof emu === 'undefined' || !emu.isReady()) {
    alert('Emulator not ready');
    return;
  }
  // C writes 256K words big-endian to /nd100_physmem.bin in MEMFS
  var rc = emu.dumpPhysicalMemory(256 * 1024);
  if (rc !== 0) { alert('Memory dump failed'); return; }
  // Read file from MEMFS and trigger browser download
  var data = emu.fsReadFile('/nd100_physmem.bin');
  var blob = new Blob([data], { type: 'application/octet-stream' });
  var url = URL.createObjectURL(blob);
  var a = document.createElement('a');
  a.href = url;
  a.download = 'nd100_physmem_256k.bin';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
  // Clean up MEMFS
  try { emu.fsUnlink('/nd100_physmem.bin'); } catch(e) {}
});

// =========================================================
// Machine Info window close
// =========================================================
document.getElementById('machine-window-close').addEventListener('click', function() {
  closeWindow('machine-window');
});

// =========================================================
// CPU Load graph window close
// =========================================================
document.getElementById('cpu-load-window-close').addEventListener('click', function() {
  closeWindow('cpu-load-window');
});

// =========================================================
// Power / Boot buttons
// =========================================================
let isInitializedBtn = false;

function completePowerOn(btn) {
  isInitialized = true;
  initializeTerminals();
  if (registerTerminalCallbacks()) {
    console.log("Terminal callbacks registered successfully");
  } else {
    console.error("Failed to register terminal callbacks");
  }

  // Enable boot device selector and boot button
  document.getElementById('boot-select').disabled = false;
  document.getElementById('toolbar-boot').disabled = false;

  // Update machine info status
  var machStatus = document.getElementById('machine-status');
  if (machStatus) machStatus.textContent = 'Initialized';

  // Update execution mode display
  var execMode = document.getElementById('machine-exec-mode');
  if (execMode) execMode.textContent = (typeof USE_WORKER !== 'undefined' && USE_WORKER) ? 'Web Worker' : 'Direct';

  // Update status
  document.getElementById('status').textContent = 'Initialized';

  // Update power button appearance
  btn.classList.add('initialized');
  btn.title = 'Power off';
  isInitializedBtn = true;

  // Start SINTRAN detection polling
  if (typeof sintranStartDetection === 'function') sintranStartDetection();
}

document.getElementById('toolbar-power').addEventListener('click', function() {
  var btn = this;
  if (!isInitializedBtn) {
    // --- POWER ON (hardware init only, no boot) ---
    console.log("Calling Init");

    if (emu.isWorkerMode()) {
      // Worker mode: init is async, terminal info arrives via callback
      emu.onInitialized = function(msg) {
        completePowerOn(btn);
      };
      emu.init();
    } else {
      // Direct mode: synchronous
      emu.init();
      completePowerOn(btn);
    }
  } else {
    // --- POWER OFF (triggers page reload to reset all state) ---
    console.log("Powering off");
    if (typeof emulationRunning !== 'undefined' && emulationRunning) {
      stopEmulation();
    }
    if (emu) emu.stop();

    btn.classList.remove('initialized');
    btn.title = 'Power on';
    isInitializedBtn = false;
    isInitialized = false;

    var machStatus = document.getElementById('machine-status');
    if (machStatus) machStatus.textContent = 'Not initialized';

    var statusEl = document.getElementById('status');
    if (statusEl) statusEl.textContent = 'Restarting...';

    setTimeout(function() {
      window.location.reload();
    }, 500);
  }
});

// --- BOOT button handler ---
// boot_type values: 0=FLOPPY, 1=SMD, 2=BPUN
function performBoot(bootType) {
  var bootBtn = document.getElementById('toolbar-boot');
  var bootNames = ['FLOPPY', 'SMD', 'BPUN'];

  console.log("Booting type " + (bootNames[bootType] || bootType));

  if (emu.isWorkerMode()) {
    // Worker mode: boot is async, result arrives via callback
    emu.onBooted = function(msg) {
      if (msg.result < 0) {
        document.getElementById('status').textContent = 'Boot failed';
        terminals[activeTerminalId].term.writeln(
          '\r\n\x1b[31mBoot failed - could not load from ' +
          (bootNames[bootType] || 'unknown') + '.\x1b[0m' +
          '\r\n\x1b[33mCheck that the image file exists and is valid.\x1b[0m'
        );
        return;
      }

      var pc = emu.getPC();
      console.log("Boot OK - P set to " + pc.toString(8).padStart(6, '0'));

      document.getElementById('boot-select').disabled = true;
      bootBtn.disabled = true;
      startEmulation(bootNames[bootType] || 'unknown');
    };
    emu.boot(bootType);
    return;
  }

  // Direct mode: synchronous
  var result = emu.boot(bootType);

  // Boot() returns PC on success, -1 on failure
  if (result < 0) {
    document.getElementById('status').textContent = 'Boot failed';
    terminals[activeTerminalId].term.writeln(
      '\r\n\x1b[31mBoot failed - could not load from ' +
      (bootNames[bootType] || 'unknown') + '.\x1b[0m' +
      '\r\n\x1b[33mCheck that the image file exists and is valid.\x1b[0m'
    );
    return;  // Leave boot button enabled for retry
  }

  var pc = emu.getPC();
  console.log("Boot OK - P set to " + pc.toString(8).padStart(6, '0'));

  // Disable boot controls (no re-boot without power cycle)
  document.getElementById('boot-select').disabled = true;
  bootBtn.disabled = true;

  startEmulation(bootNames[bootType] || 'unknown');
}

document.getElementById('toolbar-boot').addEventListener('click', function() {
  var bootDevice = document.getElementById('boot-select').value;

  if (bootDevice === 'bpun') {
    // Trigger file upload dialog for BPUN
    document.getElementById('bpun-file-input').click();
    return;
  }

  // 0=FLOPPY, 1=SMD
  var bootType = (bootDevice === 'smd') ? 1 : 0;

  // Check SMD image was loaded
  if (bootType === 1 && typeof diskImageStatus !== 'undefined' && !diskImageStatus.smd) {
    document.getElementById('status').textContent = 'Boot failed - SMD0.IMG not loaded';
    terminals[activeTerminalId].term.writeln(
      '\r\n\x1b[31mCannot boot: SMD0.IMG was not loaded.\x1b[0m\r\n' +
      '\x1b[33mEnsure the disk image is served alongside the WASM files.\x1b[0m');
    return;
  }

  // Check floppy is mounted (via Floppy Library)
  if (bootType === 0) {
    var driveEl = document.getElementById('floppy-drive-1-name');
    if (!driveEl || driveEl.textContent === 'Empty') {
      document.getElementById('status').textContent = 'No floppy mounted';
      terminals[activeTerminalId].term.writeln(
        '\r\n\x1b[31mCannot boot: No floppy image mounted in Unit 0.\x1b[0m\r\n' +
        '\x1b[33mUse View \x1b[1m\x1b[37m>\x1b[0m\x1b[33m Floppy Library to mount a floppy image first.\x1b[0m');
      return;
    }
  }

  performBoot(bootType);
});

// Handle BPUN file upload
document.getElementById('bpun-file-input').addEventListener('change', function(e) {
  var file = e.target.files[0];
  if (!file) return;

  var reader = new FileReader();
  reader.onload = function(ev) {
    var data = new Uint8Array(ev.target.result);
    // Write uploaded BPUN to MEMFS
    if (emu.isWorkerMode()) {
      emu.workerLoadDisk('/BPUN_UPLOAD.IMG', ev.target.result);
    } else {
      emu.fsWriteFile('/BPUN_UPLOAD.IMG', data);
    }
    console.log("BPUN file uploaded: " + file.name + " (" + data.length + " bytes)");

    // Small delay in Worker mode to let the file transfer complete
    if (emu.isWorkerMode()) {
      setTimeout(function() { performBoot(2); }, 100);
    } else {
      performBoot(2);
    }
  };
  reader.readAsArrayBuffer(file);

  // Reset input so same file can be re-selected
  this.value = '';
});

// =========================================================
// Terminal font/color select handlers (now in terminal header)
// =========================================================
document.getElementById('font-family-select').addEventListener('change', function(e) {
  var settings = getTerminalSettings(activeTerminalId);
  settings.fontFamily = e.target.value;
  saveTerminalSettings();
  applySettingsToTerminal(activeTerminalId);
});

document.getElementById('color-theme-select').addEventListener('change', function(e) {
  var settings = getTerminalSettings(activeTerminalId);
  settings.colorTheme = e.target.value;
  saveTerminalSettings();
  applySettingsToTerminal(activeTerminalId);
});

// =========================================================
// Terminal window management (drag, maximize)
// =========================================================
(function() {
  var termWin = document.getElementById('terminal-window');
  var termHeader = document.getElementById('terminal-window-header');
  var termMaxBtn = document.getElementById('term-maximize');
  var termMaxIcon = document.getElementById('term-maximize-icon');

  var expandSVG = '<polyline points="9,1 13,1 13,5"/><line x1="13" y1="1" x2="8" y2="6"/><polyline points="5,13 1,13 1,9"/><line x1="1" y1="13" x2="6" y2="8"/>';
  var collapseSVG = '<polyline points="5,1 1,1 1,5"/><line x1="1" y1="1" x2="6" y2="6"/><polyline points="9,13 13,13 13,9"/><line x1="13" y1="13" x2="8" y2="8"/>';

  function resizeAllTerminals() {
    Object.values(terminals).forEach(function(t) { t.resizeTerminal(); });
  }

  function isMaximized() {
    return termWin.classList.contains('maximized');
  }

  function saveTermPos() {
    try {
      localStorage.setItem('term-pos', JSON.stringify({
        left: termWin.style.left,
        top: termWin.style.top,
        width: termWin.style.width,
        height: termWin.style.height
      }));
    } catch(e) {}
  }

  function restoreTermPos() {
    try {
      var pos = JSON.parse(localStorage.getItem('term-pos'));
      if (pos) {
        if (pos.left) {
          var left = Math.max(0, Math.min(parseInt(pos.left) || 0, window.innerWidth - 200));
          var top = Math.max(49, Math.min(parseInt(pos.top) || 0, window.innerHeight - 100));
          termWin.style.left = left + 'px';
          termWin.style.top = top + 'px';
          termWin.style.bottom = 'auto';
          termWin.style.transform = 'none';
        }
        if (pos.width) {
          var w = parseInt(pos.width) || 860;
          termWin.style.width = Math.min(w, window.innerWidth - 20) + 'px';
        }
        if (pos.height) termWin.style.height = pos.height;
      }
    } catch(e) {}
  }

  // Pop-out button for main terminal (pops out the active tab-based terminal)
  var termPopoutBtn = document.getElementById('term-popout');
  if (termPopoutBtn) {
    termPopoutBtn.addEventListener('click', function() {
      if (typeof window.popOutTerminal === 'function' && typeof activeTerminalId !== 'undefined') {
        window.popOutTerminal(activeTerminalId);
      }
    });
  }

  // Use makeDraggable for terminal (but we handle maximize separately)
  makeDraggable(termWin, termHeader, 'term-pos');

  // Maximize/minimize toggle
  termMaxBtn.addEventListener('click', function() {
    if (isMaximized()) {
      termWin.classList.remove('maximized');
      restoreTermPos();
      termMaxIcon.innerHTML = expandSVG;
      termMaxBtn.title = 'Maximize';
    } else {
      saveTermPos();
      termWin.classList.add('maximized');
      termMaxIcon.innerHTML = collapseSVG;
      termMaxBtn.title = 'Restore';
    }
    try {
      localStorage.setItem('term-maximized', isMaximized() ? '1' : '0');
    } catch(e) {}
    setTimeout(resizeAllTerminals, 50);
  });

  // Restore state on load
  restoreTermPos();
  try {
    if (localStorage.getItem('term-maximized') === '1') {
      termWin.classList.add('maximized');
      termMaxIcon.innerHTML = collapseSVG;
      termMaxBtn.title = 'Restore';
    }
  } catch(e) {}
  setTimeout(resizeAllTerminals, 100);
})();

// =========================================================
// Make Machine Info and Floppy Drives windows draggable
// =========================================================
(function() {
  var machWin = document.getElementById('machine-window');
  var machHeader = document.getElementById('machine-window-header');
  if (machWin && machHeader) makeDraggable(machWin, machHeader, 'machine-pos');

  var cpuLoadWin = document.getElementById('cpu-load-window');
  var cpuLoadHeader = document.getElementById('cpu-load-window-header');
  if (cpuLoadWin && cpuLoadHeader) makeDraggable(cpuLoadWin, cpuLoadHeader, 'cpu-load-pos');

  // Floppy Drives window removed - drives shown in Machine Info

  var helpWin = document.getElementById('help-window');
  var helpHeader = document.getElementById('help-window-header');
  if (helpWin && helpHeader) makeDraggable(helpWin, helpHeader, 'help-window-pos');

  var floppyBrowserWin = document.getElementById('floppy-modal');
  var floppyBrowserHeader = document.getElementById('floppy-modal-header');
  if (floppyBrowserWin && floppyBrowserHeader) makeDraggable(floppyBrowserWin, floppyBrowserHeader, 'floppy-browser-pos');

  var disasmWin = document.getElementById('disasm-window');
  var disasmHeader = document.getElementById('disasm-window-header');
  if (disasmWin && disasmHeader) makeDraggable(disasmWin, disasmHeader, 'disasm-pos');

  var bpWin = document.getElementById('breakpoints-window');
  var bpHeader = document.getElementById('breakpoints-window-header');
  if (bpWin && bpHeader) makeDraggable(bpWin, bpHeader, 'breakpoints-pos');

  var sysinfoWin = document.getElementById('sysinfo-window');
  var sysinfoHeader = document.getElementById('sysinfo-window-header');
  if (sysinfoWin && sysinfoHeader) makeDraggable(sysinfoWin, sysinfoHeader, 'sysinfo-pos');

  var procWin = document.getElementById('process-list-window');
  var procHeader = document.getElementById('process-list-header');
  if (procWin && procHeader) makeDraggable(procWin, procHeader, 'proc-list-pos');

  var queueWin = document.getElementById('queue-viewer-window');
  var queueHeader = document.getElementById('queue-viewer-header');
  if (queueWin && queueHeader) makeDraggable(queueWin, queueHeader, 'queue-viewer-pos');

  var segWin = document.getElementById('segment-table-window');
  var segHeader = document.getElementById('segment-table-header');
  if (segWin && segHeader) makeDraggable(segWin, segHeader, 'segment-table-pos');

  var ioWin = document.getElementById('io-devices-window');
  var ioHeader = document.getElementById('io-devices-header');
  if (ioWin && ioHeader) makeDraggable(ioWin, ioHeader, 'io-devices-pos');

  var ptWin = document.getElementById('page-table-window');
  var ptHeader = document.getElementById('page-table-header');
  if (ptWin && ptHeader) makeDraggable(ptWin, ptHeader, 'page-table-pos');

  var configWin = document.getElementById('config-window');
  var configHeader = document.getElementById('config-window-header');
  if (configWin && configHeader) makeDraggable(configWin, configHeader, 'config-pos');

  // Debugger has its own drag logic but register its storage key for the global helpers
  windowStorageKeys['debugger-window'] = 'dbg-pos';
})();

// =========================================================
// Reusable resize handle helper
// =========================================================
function makeResizable(win, handle, storageKey, minW, minH) {
  if (!win || !handle) return;

  var resizing = false;
  var startX, startY, startW, startH;

  handle.addEventListener('mousedown', function(e) {
    e.preventDefault();
    e.stopPropagation();
    resizing = true;
    startX = e.clientX;
    startY = e.clientY;
    startW = win.offsetWidth;
    startH = win.offsetHeight;
    // Clear centering transform on first resize
    win.style.transform = 'none';
    if (!win.style.left || win.style.left === '') {
      var rect = win.getBoundingClientRect();
      win.style.left = rect.left + 'px';
      win.style.top = rect.top + 'px';
    }
  });

  document.addEventListener('mousemove', function(e) {
    if (!resizing) return;
    win.style.width = Math.max(minW, startW + (e.clientX - startX)) + 'px';
    win.style.height = Math.max(minH, startH + (e.clientY - startY)) + 'px';
  });

  document.addEventListener('mouseup', function() {
    if (resizing) {
      resizing = false;
      if (storageKey) {
        try {
          localStorage.setItem(storageKey, JSON.stringify({
            width: win.style.width,
            height: win.style.height
          }));
        } catch(e) {}
      }
    }
  });

  // Restore saved size
  if (storageKey) {
    try {
      var saved = JSON.parse(localStorage.getItem(storageKey));
      if (saved && saved.width && saved.height) {
        win.style.width = saved.width;
        win.style.height = saved.height;
      }
    } catch(e) {}
  }
}

// =========================================================
// Copy table as Markdown to clipboard
// =========================================================
function copyTableToClipboard(containerId, windowTitle) {
  var container = document.getElementById(containerId);
  if (!container) return;

  var table = container.querySelector('table');
  if (!table) return;

  // Extract headers
  var headers = [];
  var ths = table.querySelectorAll('thead th');
  for (var i = 0; i < ths.length; i++) {
    headers.push(ths[i].textContent.trim());
  }
  if (headers.length === 0) return;

  // Extract rows
  var rows = [];
  var trs = table.querySelectorAll('tbody tr');
  for (var r = 0; r < trs.length; r++) {
    var cells = [];
    var tds = trs[r].querySelectorAll('td');
    for (var c = 0; c < tds.length; c++) {
      // Replace pipe chars in cell text to avoid breaking markdown
      cells.push(tds[c].textContent.trim().replace(/\|/g, '/'));
    }
    rows.push(cells);
  }

  // Build markdown
  var md = '# ' + windowTitle + '\n\n';
  md += '| ' + headers.join(' | ') + ' |\n';
  md += '|';
  for (var h = 0; h < headers.length; h++) md += ' --- |';
  md += '\n';
  for (var j = 0; j < rows.length; j++) {
    md += '| ' + rows[j].join(' | ') + ' |\n';
  }

  navigator.clipboard.writeText(md).then(function() {
    // Brief visual feedback on the copy button
    var btn = container.closest('.glass-window').querySelector('.copy-table-btn');
    if (btn) {
      btn.classList.add('copied');
      setTimeout(function() { btn.classList.remove('copied'); }, 1200);
    }
  });
}

// Apply resize to terminal, floppy library, and help windows
makeResizable(
  document.getElementById('terminal-window'),
  document.getElementById('terminal-window-resize'),
  'term-size', 500, 350
);
makeResizable(
  document.getElementById('floppy-modal'),
  document.getElementById('floppy-modal-resize'),
  'floppy-browser-size', 500, 350
);
makeResizable(
  document.getElementById('help-window'),
  document.getElementById('help-window-resize'),
  'help-window-size', 400, 300
);
makeResizable(
  document.getElementById('disasm-window'),
  document.getElementById('disasm-window-resize'),
  'disasm-size', 350, 250
);
makeResizable(
  document.getElementById('breakpoints-window'),
  document.getElementById('breakpoints-window-resize'),
  'breakpoints-size', 320, 250
);
makeResizable(
  document.getElementById('process-list-window'),
  document.getElementById('process-list-resize'),
  'proc-list-size', 600, 300
);
makeResizable(
  document.getElementById('queue-viewer-window'),
  document.getElementById('queue-viewer-resize'),
  'queue-viewer-size', 500, 300
);
makeResizable(
  document.getElementById('segment-table-window'),
  document.getElementById('segment-table-resize'),
  'segment-table-size', 550, 300
);
makeResizable(
  document.getElementById('io-devices-window'),
  document.getElementById('io-devices-resize'),
  'io-devices-size', 500, 300
);
makeResizable(
  document.getElementById('page-table-window'),
  document.getElementById('page-table-resize'),
  'page-table-size', 550, 350
);
makeResizable(
  document.getElementById('cpu-load-window'),
  document.getElementById('cpu-load-window-resize'),
  'cpu-load-size', 240, 140
);

// =========================================================
// Register all windows with the window manager
// =========================================================
windowManager.register('machine-window', 'Machine Info');
windowManager.register('cpu-load-window', 'CPU Load');
windowManager.register('about-window', 'About');
windowManager.register('floppy-modal', 'Floppy Library');
windowManager.register('help-window', 'SINTRAN Help');
windowManager.register('terminal-window', 'Console');
windowManager.register('debugger-window', 'Debugger');
windowManager.register('disasm-window', 'Disassembly');
windowManager.register('breakpoints-window', 'Breakpoints');
windowManager.register('sysinfo-window', 'System Info');
windowManager.register('process-list-window', 'RT Descriptions');
windowManager.register('queue-viewer-window', 'Queues');
windowManager.register('segment-table-window', 'Segments');
windowManager.register('io-devices-window', 'I/O Devices');
windowManager.register('page-table-window', 'Page Tables');
windowManager.register('config-window', 'Config');

// Restore window visibility from localStorage
(function() {
  try {
    var state = JSON.parse(localStorage.getItem('window-visibility') || '{}');
    for (var id in state) {
      var win = document.getElementById(id);
      if (!win) continue;
      if (state[id]) {
        win.style.display = 'flex';
      } else {
        win.style.display = 'none';
      }
    }
  } catch(e) {}
  // Remove early-restore style tag - toolbar.js now owns all positions/visibility
  var earlyStyle = document.getElementById('early-restore');
  if (earlyStyle) earlyStyle.remove();
})();

// Periodically update taskbar to reflect visible windows
setInterval(function() { windowManager.updateTaskbar(); }, 500);
// Initial update
windowManager.updateTaskbar();
