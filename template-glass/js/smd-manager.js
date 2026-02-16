//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// smd-manager.js - SMD Disk Manager window UI
//
// Provides a glass window for managing persistent SMD disk images:
// - Drive configuration (4 units, boot drive selection)
// - Local disk library (list, import, download, delete)
// - Storage quota display

'use strict';

// =========================================================
// Show / Hide
// =========================================================

function smdManagerShow() {
  var win = document.getElementById('smd-manager-window');
  if (!win) return;
  win.style.display = 'flex';
  windowManager.focus('smd-manager-window');
  saveWindowVisibility('smd-manager-window', true);
  smdRefreshAll();
}

function smdManagerHide() {
  var win = document.getElementById('smd-manager-window');
  if (win) win.style.display = 'none';
  saveWindowVisibility('smd-manager-window', false);
}

// =========================================================
// Refresh all sections
// =========================================================

function smdRefreshAll() {
  smdRefreshUnitDisplay();
  smdRefreshInstalledList();
  smdUpdateStorageInfo();
  smdUpdatePersistToggle();
}

// =========================================================
// Persistence toggle state
// =========================================================

function smdUpdatePersistToggle() {
  var toggle = document.getElementById('smd-persist-toggle');
  if (toggle) {
    toggle.checked = isSmdPersistenceEnabled();
  }
  // Show/hide the manager body based on persistence state
  var body = document.getElementById('smd-manager-body');
  var demoMsg = document.getElementById('smd-demo-message');
  if (body && demoMsg) {
    if (isSmdPersistenceEnabled()) {
      body.style.display = 'block';
      demoMsg.style.display = 'none';
    } else {
      body.style.display = 'none';
      demoMsg.style.display = 'block';
    }
  }
}

// =========================================================
// Unit display
// =========================================================

function smdRefreshUnitDisplay() {
  var units = smdStorage.getUnitAssignments();
  var bootUnit = smdStorage.getBootUnit();

  for (var u = 0; u < 4; u++) {
    var nameEl = document.getElementById('smd-unit-' + u + '-name');
    var ejectBtn = document.getElementById('smd-unit-' + u + '-eject');
    var starEl = document.getElementById('smd-unit-' + u + '-star');

    if (!nameEl) continue;

    if (units[u]) {
      nameEl.textContent = units[u];
      nameEl.classList.remove('empty');
      if (ejectBtn) ejectBtn.style.display = '';
    } else {
      nameEl.textContent = 'Not assigned';
      nameEl.classList.add('empty');
      if (ejectBtn) ejectBtn.style.display = 'none';
    }

    if (starEl) {
      starEl.style.display = (u === bootUnit) ? '' : 'none';
    }
  }

  // Update boot unit selector
  var bootSelect = document.getElementById('smd-boot-select');
  if (bootSelect) {
    bootSelect.value = '' + bootUnit;
  }

  // Update Machine Info SMD section
  smdUpdateMachineInfo();
}

function smdUpdateMachineInfo() {
  var nameEl = document.getElementById('smd-info-0-name');
  if (!nameEl) return;

  if (isSmdPersistenceEnabled()) {
    var units = smdStorage.getUnitAssignments();
    nameEl.textContent = units[0] || 'Not assigned';
  } else {
    nameEl.textContent = 'SMD0.IMG (demo)';
  }
}

// =========================================================
// Installed list
// =========================================================

function smdRefreshInstalledList() {
  var container = document.getElementById('smd-installed-list');
  if (!container) return;

  var images = smdStorage.listImages();

  if (images.length === 0) {
    container.innerHTML = '<div class="smd-empty-msg">No disk images stored. Use Download or Import to add images.</div>';
    return;
  }

  var html = '';
  images.forEach(function(img) {
    html += '<div class="smd-image-card">';
    html += '<div class="smd-image-info">';
    html += '<span class="smd-image-name">' + escapeHtml(img.fileName) + '</span>';
    html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; ' + (img.date || '') + '</span>';
    html += '</div>';
    html += '<div class="smd-image-actions">';
    html += '<select class="smd-assign-select" data-file="' + escapeHtml(img.fileName) + '" title="Assign to unit">';
    html += '<option value="">Assign to...</option>';
    for (var u = 0; u < 4; u++) {
      html += '<option value="' + u + '">Unit ' + u + '</option>';
    }
    html += '</select>';
    html += '<button class="smd-delete-btn" data-file="' + escapeHtml(img.fileName) + '" title="Delete">Del</button>';
    html += '</div>';
    html += '</div>';
  });

  container.innerHTML = html;

  // Wire up assign dropdowns
  container.querySelectorAll('.smd-assign-select').forEach(function(sel) {
    sel.addEventListener('change', function() {
      if (this.value !== '') {
        smdAssignToUnit(this.getAttribute('data-file'), parseInt(this.value));
        this.value = '';
      }
    });
  });

  // Wire up delete buttons
  container.querySelectorAll('.smd-delete-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var fileName = this.getAttribute('data-file');
      if (confirm('Delete ' + fileName + ' from persistent storage?')) {
        smdDeleteImage(fileName);
      }
    });
  });
}

// =========================================================
// Actions
// =========================================================

function smdAssignToUnit(fileName, unit) {
  if (unit < 0 || unit > 3) return;

  smdStorage.setUnitAssignment(unit, fileName);

  // If emulator is initialized and persistence is on, mount live
  if (emu && emu.isReady && emu.isReady() && isSmdPersistenceEnabled()) {
    if (emu.isWorkerMode()) {
      emu.opfsMountSMD(unit, fileName).then(function(r) {
        console.log('[SMD Manager] Unit ' + unit + ' mounted: ' + fileName + (r.ok ? ' OK' : ' FAILED'));
      });
    } else {
      smdStorage.retrieveImage(fileName).then(function(data) {
        if (data) {
          emu.mountSMDFromBuffer(unit, data);
          console.log('[SMD Manager] Unit ' + unit + ' mounted from buffer: ' + fileName);
        }
      });
    }
  }

  smdRefreshAll();
}

function smdEjectUnit(unit) {
  if (unit < 0 || unit > 3) return;

  // Save dirty buffer first if Direct mode
  if (!emu.isWorkerMode() && smdStorage.isDirty(unit)) {
    smdSaveUnit(unit);
  }

  smdStorage.clearUnitAssignment(unit);

  if (emu && emu.isReady && emu.isReady()) {
    if (emu.isWorkerMode()) {
      emu.opfsUnmountSMD(unit);
    } else {
      emu.unmountSMD(unit);
    }
  }

  smdRefreshAll();
}

function smdSaveUnit(unit) {
  if (emu.isWorkerMode()) return; // Worker mode: SyncAccessHandle persists immediately

  var units = smdStorage.getUnitAssignments();
  var fileName = units[unit];
  if (!fileName) return;

  // Read buffer from WASM heap
  var ptr = emu.getSMDBuffer(unit);
  var size = emu.getSMDBufferSize(unit);
  if (!ptr || !size) return;

  var data = new Uint8Array(Module.HEAPU8.buffer, ptr, size);
  // Copy before async operation (WASM heap may grow)
  var copy = new Uint8Array(data);

  smdStorage.storeImage(fileName, copy, {
    name: fileName,
    description: 'Saved from emulator'
  }).then(function() {
    smdStorage.clearDirty(unit);
    console.log('[SMD Manager] Unit ' + unit + ' saved to OPFS: ' + fileName);
  }).catch(function(err) {
    console.error('[SMD Manager] Save failed for unit ' + unit + ':', err);
  });
}

function smdDeleteImage(fileName) {
  smdStorage.deleteImage(fileName).then(function() {
    smdRefreshAll();
  });
}

function smdImportFromFile() {
  var input = document.createElement('input');
  input.type = 'file';
  input.accept = '.img,.IMG';
  input.addEventListener('change', function() {
    var file = input.files[0];
    if (!file) return;

    var reader = new FileReader();
    reader.onload = function(ev) {
      var data = new Uint8Array(ev.target.result);
      var fileName = file.name.toUpperCase();
      if (!fileName.endsWith('.IMG')) fileName += '.IMG';

      smdStorage.storeImage(fileName, data, {
        name: fileName,
        description: 'Imported from local file'
      }).then(function() {
        smdRefreshAll();
        console.log('[SMD Manager] Imported ' + fileName + ' (' + smdStorage.formatSize(data.byteLength) + ')');
      });
    };
    reader.readAsArrayBuffer(file);
  });
  input.click();
}

function smdDownloadFromCatalog() {
  var statusEl = document.getElementById('smd-download-status');

  fetch('smd-catalog.json').then(function(r) { return r.json(); }).then(function(catalog) {
    if (!catalog || catalog.length === 0) {
      if (statusEl) statusEl.textContent = 'No images available in catalog';
      return;
    }

    // For now, download each catalog entry that isn't already stored
    var toDownload = catalog.filter(function(entry) {
      return !smdStorage.imageExists(entry.fileName);
    });

    if (toDownload.length === 0) {
      if (statusEl) statusEl.textContent = 'All catalog images already downloaded';
      return;
    }

    // Download sequentially
    var chain = Promise.resolve();
    toDownload.forEach(function(entry) {
      chain = chain.then(function() {
        if (statusEl) statusEl.textContent = 'Downloading ' + entry.name + '...';
        return downloadImageBuffer(entry.url).then(function(buf) {
          return smdStorage.storeImage(entry.fileName, new Uint8Array(buf), {
            name: entry.name,
            description: entry.description
          });
        });
      });
    });

    chain.then(function() {
      if (statusEl) statusEl.textContent = 'Download complete';
      smdRefreshAll();
      // Auto-assign first image to unit 0 if nothing assigned
      var units = smdStorage.getUnitAssignments();
      if (!units[0] && toDownload.length > 0) {
        smdAssignToUnit(toDownload[0].fileName, 0);
      }
    }).catch(function(err) {
      if (statusEl) statusEl.textContent = 'Download failed: ' + err.message;
    });
  }).catch(function(err) {
    if (statusEl) statusEl.textContent = 'Could not load catalog: ' + err.message;
  });
}

// =========================================================
// Storage info
// =========================================================

function smdUpdateStorageInfo() {
  var infoEl = document.getElementById('smd-storage-info');
  if (!infoEl) return;

  smdStorage.getStorageEstimate().then(function(est) {
    if (est.quota > 0) {
      infoEl.textContent = 'Storage: ' + smdStorage.formatSize(est.usage) + ' used of ' + smdStorage.formatSize(est.quota);
    } else {
      var images = smdStorage.listImages();
      var total = 0;
      images.forEach(function(img) { total += img.size || 0; });
      infoEl.textContent = 'Storage: ' + smdStorage.formatSize(total) + ' used';
    }
  });
}

// =========================================================
// Helpers
// =========================================================

function escapeHtml(str) {
  return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

// =========================================================
// Persistence toggle handler
// =========================================================

function smdHandlePersistToggle(enabled) {
  localStorage.setItem(SMD_PERSIST_KEY, enabled ? 'true' : 'false');
  if (enabled) {
    // Turning on persistence - need to reload to switch from MEMFS to OPFS
    if (confirm('Enabling persistent storage requires a page reload.\n\nThe current SMD image will be downloaded and stored in your browser.\nAfter this, disk writes will persist across sessions.\n\nReload now?')) {
      location.reload();
    } else {
      // User cancelled - revert toggle
      localStorage.setItem(SMD_PERSIST_KEY, 'false');
      smdUpdatePersistToggle();
    }
  } else {
    // Turning off persistence - reload to switch back to MEMFS
    if (confirm('Disabling persistent storage requires a page reload.\n\nThe emulator will return to demo mode (SMD image downloaded each visit).\nYour stored images will be kept in browser storage.\n\nReload now?')) {
      location.reload();
    } else {
      localStorage.setItem(SMD_PERSIST_KEY, 'true');
      smdUpdatePersistToggle();
    }
  }
}

// =========================================================
// Auto-save on page unload (Direct mode only)
// =========================================================

window.addEventListener('beforeunload', function(e) {
  if (!isSmdPersistenceEnabled()) return;
  if (emu && emu.isWorkerMode && emu.isWorkerMode()) return; // Worker persists immediately

  var dirtyUnits = smdStorage.getDirtyUnits();
  if (dirtyUnits.length > 0) {
    // Try to save synchronously (best-effort)
    dirtyUnits.forEach(function(unit) {
      smdSaveUnit(unit);
    });
    // Show warning if saves pending
    e.preventDefault();
    e.returnValue = 'Disk changes are being saved...';
  }
});

// Expose for toolbar.js
window.smdManagerShow = smdManagerShow;
window.smdManagerHide = smdManagerHide;
