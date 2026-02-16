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

  // Build reverse map: fileName -> unit number (or -1 if not assigned)
  var units = smdStorage.getUnitAssignments();
  var fileToUnit = {};
  for (var u = 0; u < 4; u++) {
    if (units[u]) fileToUnit[units[u]] = u;
  }

  var html = '';
  images.forEach(function(img) {
    var assignedUnit = (fileToUnit[img.fileName] !== undefined) ? fileToUnit[img.fileName] : -1;
    html += '<div class="smd-image-card">';
    html += '<div class="smd-image-info">';
    html += '<span class="smd-image-name">' + escapeHtml(img.fileName) + '</span>';
    html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; ' + (img.date || '');
    if (assignedUnit >= 0) {
      html += ' &middot; Unit ' + assignedUnit;
    }
    html += '</span>';
    html += '</div>';
    html += '<div class="smd-image-actions">';
    html += '<select class="smd-assign-select" data-file="' + escapeHtml(img.fileName) + '" title="Assign to unit">';
    if (assignedUnit >= 0) {
      html += '<option value="">Unit ' + assignedUnit + ' (move...)</option>';
    } else {
      html += '<option value="">Assign to...</option>';
    }
    for (var u2 = 0; u2 < 4; u2++) {
      if (u2 === assignedUnit) continue;  // Skip current assignment
      html += '<option value="' + u2 + '">Unit ' + u2 + '</option>';
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

  // If this file is already assigned to another unit, eject it first
  // (OPFS SyncAccessHandle is exclusive — only one handle per file)
  var currentUnits = smdStorage.getUnitAssignments();
  for (var u = 0; u < 4; u++) {
    if (u !== unit && currentUnits[u] === fileName) {
      console.log('[SMD Manager] Ejecting ' + fileName + ' from unit ' + u + ' before reassigning to unit ' + unit);
      smdEjectUnit(u);
    }
  }

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

// =========================================================
// Remote Images (Gateway)
// =========================================================

var _gatewayDiskList = null;      // { smd: [...], floppy: [...] }
var _gatewayMountedUnits = {};    // 'smd-0' -> true, 'floppy-0' -> true

// Called by proxy when disk-list arrives from gateway sub-worker
window.onDiskList = function(data) {
  _gatewayDiskList = data;
  smdRefreshRemoteList();
};

window.onDiskConnected = function() {
  console.log('[SMD Manager] Gateway disk I/O connected');
};

window.onDiskDisconnected = function() {
  console.log('[SMD Manager] Gateway disk I/O disconnected');
  _gatewayDiskList = null;
  _gatewayMountedUnits = {};
  smdRefreshRemoteList();
};

function smdRefreshRemoteList() {
  var container = document.getElementById('smd-remote-list');
  if (!container) return;

  if (!_gatewayDiskList) {
    container.innerHTML = '<div class="smd-empty-msg">No gateway connection. Connect via Network settings to access remote disk images.</div>';
    return;
  }

  var smdImages = _gatewayDiskList.smd || [];
  var floppyImages = _gatewayDiskList.floppy || [];

  if (smdImages.length === 0 && floppyImages.length === 0) {
    container.innerHTML = '<div class="smd-empty-msg">No disk images configured on gateway server.</div>';
    return;
  }

  var html = '';

  if (smdImages.length > 0) {
    html += '<div class="smd-remote-section-title">SMD Images</div>';
    smdImages.forEach(function(img) {
      var key = 'smd-' + img.unit;
      var mounted = _gatewayMountedUnits[key];
      html += '<div class="smd-image-card">';
      html += '<div class="smd-image-info">';
      html += '<span class="smd-image-name">' + escapeHtml(img.name) + '</span>';
      html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; Gateway unit ' + img.unit + '</span>';
      html += '</div>';
      html += '<div class="smd-image-actions">';
      if (mounted) {
        html += '<span class="smd-badge-gateway">Gateway</span>';
        html += '<button class="smd-eject-remote-btn" data-type="smd" data-unit="' + img.unit + '">Eject</button>';
      } else {
        html += '<select class="smd-assign-remote-select" data-type="smd" data-remote-unit="' + img.unit + '" data-size="' + img.size + '" title="Mount to unit">';
        html += '<option value="">Mount to...</option>';
        for (var u = 0; u < 4; u++) {
          html += '<option value="' + u + '">Unit ' + u + '</option>';
        }
        html += '</select>';
      }
      html += '</div>';
      html += '</div>';
    });
  }

  if (floppyImages.length > 0) {
    html += '<div class="smd-remote-section-title">Floppy Images</div>';
    floppyImages.forEach(function(img) {
      var key = 'floppy-' + img.unit;
      var mounted = _gatewayMountedUnits[key];
      html += '<div class="smd-image-card">';
      html += '<div class="smd-image-info">';
      html += '<span class="smd-image-name">' + escapeHtml(img.name) + '</span>';
      html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; Gateway unit ' + img.unit + '</span>';
      html += '</div>';
      html += '<div class="smd-image-actions">';
      if (mounted) {
        html += '<span class="smd-badge-gateway">Gateway</span>';
        html += '<button class="smd-eject-remote-btn" data-type="floppy" data-unit="' + img.unit + '">Eject</button>';
      } else {
        html += '<button class="smd-mount-remote-btn" data-type="floppy" data-remote-unit="' + img.unit + '" data-size="' + img.size + '">Mount</button>';
      }
      html += '</div>';
      html += '</div>';
    });
  }

  container.innerHTML = html;

  // Wire up mount dropdowns for SMD
  container.querySelectorAll('.smd-assign-remote-select').forEach(function(sel) {
    sel.addEventListener('change', function() {
      if (this.value !== '') {
        var localUnit = parseInt(this.value);
        var remoteUnit = parseInt(this.getAttribute('data-remote-unit'));
        var size = parseInt(this.getAttribute('data-size'));
        var driveType = this.getAttribute('data-type');
        smdMountRemote(driveType, remoteUnit, localUnit, size);
        this.value = '';
      }
    });
  });

  // Wire up mount buttons for floppy
  container.querySelectorAll('.smd-mount-remote-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var remoteUnit = parseInt(this.getAttribute('data-remote-unit'));
      var size = parseInt(this.getAttribute('data-size'));
      smdMountRemote('floppy', remoteUnit, 0, size);
    });
  });

  // Wire up eject buttons
  container.querySelectorAll('.smd-eject-remote-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var driveType = this.getAttribute('data-type');
      var unit = parseInt(this.getAttribute('data-unit'));
      smdEjectRemote(driveType, unit);
    });
  });
}

function smdMountRemote(driveType, remoteUnit, localUnit, imageSize) {
  if (!emu || !emu.isWorkerMode || !emu.isWorkerMode()) {
    console.warn('[SMD Manager] Gateway disk mounting requires Worker mode');
    return;
  }

  var key = driveType + '-' + remoteUnit;
  var mountFn = (driveType === 'smd') ? emu.gatewayMountSMD : emu.gatewayMountFloppy;

  mountFn(localUnit, imageSize).then(function(r) {
    if (r.ok) {
      _gatewayMountedUnits[key] = true;
      console.log('[SMD Manager] Gateway ' + driveType + ' unit ' + remoteUnit + ' mounted to local unit ' + localUnit);
    } else {
      console.error('[SMD Manager] Gateway mount failed for ' + driveType + ' unit ' + remoteUnit);
    }
    smdRefreshRemoteList();
    smdRefreshUnitDisplay();
  });
}

function smdEjectRemote(driveType, unit) {
  var key = driveType + '-' + unit;
  var unmountFn = (driveType === 'smd') ? emu.gatewayUnmountSMD : emu.gatewayUnmountFloppy;

  unmountFn(unit);
  delete _gatewayMountedUnits[key];
  smdRefreshRemoteList();
  smdRefreshUnitDisplay();
}

// Expose for toolbar.js
window.smdManagerShow = smdManagerShow;
window.smdManagerHide = smdManagerHide;
