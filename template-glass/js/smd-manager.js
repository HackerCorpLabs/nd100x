//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// smd-manager.js - SMD Disk Manager window UI
//
// Provides a glass window for managing persistent SMD disk images:
// - Drive configuration (4 units, boot drive selection)
// - Local disk library (list, import, rename, delete, copy from catalog)
// - Server catalog browsing with "Copy to local disk library" dialog
// - Storage quota display

'use strict';

// =========================================================
// Show / Hide
// =========================================================

function smdManagerShow() {
  if (!isSmdPersistenceEnabled()) return;
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
  // Close window if persistence was disabled while it was open
  if (!isSmdPersistenceEnabled()) {
    smdManagerHide();
    return;
  }
  // Ensure smdStorage is initialized before reading data.
  // init() loads metadata from localStorage; returns immediately if already initialized.
  smdStorage.init().then(function() {
    smdRefreshUnitDisplay();
    smdRefreshInstalledList();
    smdRefreshCatalogList();
    smdUpdateStorageInfo();
  });
}

// =========================================================
// Unit display
// =========================================================

function smdRefreshUnitDisplay() {
  var units = smdStorage.getUnitAssignments();

  for (var u = 0; u < 4; u++) {
    var nameEl = document.getElementById('smd-unit-' + u + '-name');
    var ejectBtn = document.getElementById('smd-unit-' + u + '-eject');

    if (!nameEl) continue;

    // Check registry for current mount state (covers all sources)
    var regEntry = (typeof driveRegistry !== 'undefined') ? driveRegistry.get('smd', u) : null;

    if (regEntry && regEntry.mounted) {
      var displayName = regEntry.name || 'Mounted';
      if (regEntry.source === 'gateway') displayName += ' (Gateway)';
      nameEl.textContent = displayName;
      nameEl.classList.remove('empty');
      if (ejectBtn) ejectBtn.style.display = '';
    } else if (units[u]) {
      // Show the image name from metadata instead of UUID
      var meta = smdStorage.getMetadata(units[u]);
      nameEl.textContent = meta ? meta.name : units[u];
      nameEl.classList.remove('empty');
      if (ejectBtn) ejectBtn.style.display = '';
    } else {
      nameEl.textContent = 'Not assigned';
      nameEl.classList.add('empty');
      if (ejectBtn) ejectBtn.style.display = 'none';
    }
  }
}

// =========================================================
// Installed list (Local Disk Library)
// =========================================================

function smdRefreshInstalledList() {
  var container = document.getElementById('smd-installed-list');
  if (!container) return;

  var images = smdStorage.listImages();

  if (images.length === 0) {
    container.innerHTML = '<div class="smd-empty-msg">No disk images stored. Use the Server Catalog or Import to add images.</div>';
    return;
  }

  // Build reverse map: uuid -> unit number (or -1 if not assigned)
  var units = smdStorage.getUnitAssignments();
  var uuidToUnit = {};
  for (var u = 0; u < 4; u++) {
    if (units[u]) uuidToUnit[units[u]] = u;
  }

  var html = '';
  images.forEach(function(img) {
    var uuid = img.uuid;
    var assignedUnit = (uuidToUnit[uuid] !== undefined) ? uuidToUnit[uuid] : -1;
    html += '<div class="smd-image-card" data-uuid="' + escapeHtml(uuid) + '">';
    html += '<div class="smd-image-info">';
    html += '<span class="smd-image-name">' + escapeHtml(img.name) + '</span>';
    html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; ' + (img.date || '');
    if (assignedUnit >= 0) {
      html += ' &middot; Unit ' + assignedUnit;
    }
    html += '</span>';
    if (img.sourceName) {
      html += '<span class="smd-source-label">Source: ' + escapeHtml(img.sourceName) + '</span>';
    }
    if (img.description) {
      html += '<span class="smd-image-meta">' + escapeHtml(img.description) + '</span>';
    }
    html += '</div>';
    html += '<div class="smd-image-actions">';
    html += '<select class="smd-assign-select" data-uuid="' + escapeHtml(uuid) + '" title="Assign to unit">';
    if (assignedUnit >= 0) {
      html += '<option value="">Unit ' + assignedUnit + ' (move...)</option>';
    } else {
      html += '<option value="">Assign to...</option>';
    }
    for (var u2 = 0; u2 < 4; u2++) {
      if (u2 === assignedUnit) continue;
      html += '<option value="' + u2 + '">Unit ' + u2 + '</option>';
    }
    html += '</select>';
    html += '<button class="smd-rename-btn" data-uuid="' + escapeHtml(uuid) + '" title="Rename">Rename</button>';
    html += '<button class="smd-export-btn" data-uuid="' + escapeHtml(uuid) + '" title="Export to local file">Export</button>';
    html += '<button class="smd-delete-btn" data-uuid="' + escapeHtml(uuid) + '" title="Delete">Del</button>';
    html += '</div>';
    html += '</div>';
  });

  container.innerHTML = html;

  // Wire up assign dropdowns
  container.querySelectorAll('.smd-assign-select').forEach(function(sel) {
    sel.addEventListener('change', function() {
      if (this.value !== '') {
        smdAssignToUnit(this.getAttribute('data-uuid'), parseInt(this.value));
        this.value = '';
      }
    });
  });

  // Wire up rename buttons
  container.querySelectorAll('.smd-rename-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      smdShowRenameInline(this.getAttribute('data-uuid'));
    });
  });

  // Wire up export buttons
  container.querySelectorAll('.smd-export-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      smdExportImage(this.getAttribute('data-uuid'));
    });
  });

  // Wire up delete buttons
  container.querySelectorAll('.smd-delete-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var uuid = this.getAttribute('data-uuid');
      var meta = smdStorage.getMetadata(uuid);
      var displayName = meta ? meta.name : uuid;
      if (confirm('Delete "' + displayName + '" from persistent storage?')) {
        smdDeleteImage(uuid);
      }
    });
  });
}

// =========================================================
// Inline rename
// =========================================================

function smdShowRenameInline(uuid) {
  var card = document.querySelector('.smd-image-card[data-uuid="' + uuid + '"]');
  if (!card) return;
  var meta = smdStorage.getMetadata(uuid);
  if (!meta) return;

  var infoEl = card.querySelector('.smd-image-info');
  if (!infoEl) return;

  // Replace info content with editable fields
  infoEl.innerHTML =
    '<div class="smd-rename-fields">' +
    '<input type="text" class="smd-rename-input" id="smd-rename-name" value="' + escapeHtml(meta.name) + '" placeholder="Name" maxlength="80">' +
    '</div>' +
    '<div class="smd-rename-fields">' +
    '<input type="text" class="smd-rename-input" id="smd-rename-desc" value="' + escapeHtml(meta.description) + '" placeholder="Description" maxlength="200">' +
    '<button class="smd-rename-save" id="smd-rename-save">Save</button>' +
    '</div>';

  var saveBtn = infoEl.querySelector('#smd-rename-save');
  var nameInput = infoEl.querySelector('#smd-rename-name');
  var descInput = infoEl.querySelector('#smd-rename-desc');

  function doSave() {
    var newName = nameInput.value.trim() || meta.name;
    var newDesc = descInput.value.trim();
    smdStorage.updateMetadata(uuid, newName, newDesc);
    smdRefreshAll();
  }

  saveBtn.addEventListener('click', doSave);
  nameInput.addEventListener('keydown', function(e) { if (e.key === 'Enter') doSave(); });
  descInput.addEventListener('keydown', function(e) { if (e.key === 'Enter') doSave(); });
  nameInput.focus();
  nameInput.select();
}

// =========================================================
// Actions
// =========================================================

function smdAssignToUnit(uuid, unit) {
  if (unit < 0 || unit > 3) return;

  // If this image is already assigned to another unit, eject it first
  // (OPFS SyncAccessHandle is exclusive -- only one handle per file)
  var currentUnits = smdStorage.getUnitAssignments();
  for (var u = 0; u < 4; u++) {
    if (u !== unit && currentUnits[u] === uuid) {
      console.log('[SMD Manager] Ejecting from unit ' + u + ' before reassigning to unit ' + unit);
      smdEjectUnit(u);
    }
  }

  smdStorage.setUnitAssignment(unit, uuid);

  // If emulator is initialized and persistence is on, mount live
  if (emu && emu.isReady && emu.isReady() && isSmdPersistenceEnabled()) {
    var meta = smdStorage.getMetadata(uuid);
    var displayName = meta ? meta.name : uuid;

    if (emu.isWorkerMode()) {
      emu.opfsMountSMD(unit, uuid).then(function(r) {
        console.log('[SMD Manager] Unit ' + unit + ' mounted: ' + displayName + (r.ok ? ' OK' : ' FAILED'));
        if (r.ok && typeof driveRegistry !== 'undefined') {
          driveRegistry.mount('smd', unit, 'opfs', displayName, uuid, r.size || 0);
        }
      });
    } else {
      smdStorage.retrieveImage(uuid).then(function(data) {
        if (data) {
          emu.mountSMDFromBuffer(unit, data);
          console.log('[SMD Manager] Unit ' + unit + ' mounted from buffer: ' + displayName);
          if (typeof driveRegistry !== 'undefined') {
            driveRegistry.mount('smd', unit, 'opfs', displayName, uuid, data.byteLength);
          }
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

  if (typeof driveRegistry !== 'undefined') {
    driveRegistry.eject('smd', unit);
  }

  smdRefreshAll();
}

function smdSaveUnit(unit) {
  if (emu.isWorkerMode()) return; // Worker mode: SyncAccessHandle persists immediately

  var units = smdStorage.getUnitAssignments();
  var uuid = units[unit];
  if (!uuid) return;

  // Read buffer from WASM heap
  var ptr = emu.getSMDBuffer(unit);
  var size = emu.getSMDBufferSize(unit);
  if (!ptr || !size) return;

  var data = new Uint8Array(Module.HEAPU8.buffer, ptr, size);
  // Copy before async operation (WASM heap may grow)
  var copy = new Uint8Array(data);

  var meta = smdStorage.getMetadata(uuid);
  smdStorage.storeImage(uuid, copy, {
    name: meta ? meta.name : uuid,
    description: meta ? meta.description : 'Saved from emulator',
    sourceName: meta ? meta.sourceName : ''
  }).then(function() {
    smdStorage.clearDirty(unit);
    console.log('[SMD Manager] Unit ' + unit + ' saved to OPFS');
  }).catch(function(err) {
    console.error('[SMD Manager] Save failed for unit ' + unit + ':', err);
  });
}

function smdDeleteImage(uuid) {
  smdStorage.deleteImage(uuid).then(function() {
    smdRefreshAll();
  });
}

function smdExportImage(uuid) {
  var meta = smdStorage.getMetadata(uuid);
  var exportName = meta ? (meta.name.replace(/[^a-zA-Z0-9._-]/g, '_') + '.IMG') : (uuid + '.IMG');

  smdStorage.retrieveImage(uuid).then(function(data) {
    if (!data) {
      console.error('[SMD Manager] Export failed: no data for ' + uuid);
      return;
    }
    var blob = new Blob([data], { type: 'application/octet-stream' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = exportName;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
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
      var uuid = smdStorage.generateUUID();
      var displayName = file.name;

      smdStorage.storeImage(uuid, data, {
        name: displayName,
        description: 'Imported from local file',
        sourceName: file.name
      }).then(function() {
        smdRefreshAll();
        console.log('[SMD Manager] Imported ' + displayName + ' (' + smdStorage.formatSize(data.byteLength) + ')');
      });
    };
    reader.readAsArrayBuffer(file);
  });
  input.click();
}

// =========================================================
// Server Catalog browsing
// =========================================================

var _catalogData = null;

function smdRefreshCatalogList() {
  var container = document.getElementById('smd-catalog-list');
  if (!container) return;

  if (_catalogData) {
    smdRenderCatalog(container, _catalogData);
    return;
  }

  fetch('smd-catalog.json').then(function(r) { return r.json(); }).then(function(catalog) {
    _catalogData = catalog || [];
    smdRenderCatalog(container, _catalogData);
  }).catch(function(err) {
    container.innerHTML = '<div class="smd-empty-msg">Could not load catalog: ' + escapeHtml(err.message) + '</div>';
  });
}

function smdRenderCatalog(container, catalog) {
  if (!catalog || catalog.length === 0) {
    container.innerHTML = '<div class="smd-empty-msg">No images available in server catalog.</div>';
    return;
  }

  var html = '';
  catalog.forEach(function(entry, idx) {
    var isGeneratable = !!entry.generate;
    var isAvailable = entry.available !== false; // static catalog has no 'available' field

    html += '<div class="smd-image-card smd-catalog-card' + (!isAvailable ? ' unavailable' : '') + '">';
    html += '<div class="smd-image-info">';
    html += '<span class="smd-image-name">' + escapeHtml(entry.name) + '</span>';
    html += '<span class="smd-image-meta">' + smdStorage.formatSize(entry.size);
    html += ' &middot; ' + escapeHtml(entry.description);
    html += '</span>';
    html += '</div>';
    html += '<div class="smd-image-actions">';
    if (!isAvailable) {
      html += '<span class="smd-image-meta">Unavailable</span>';
    } else if (isGeneratable) {
      html += '<button class="smd-action-btn smd-catalog-dl-btn" data-idx="' + idx + '" title="Create a blank image in local library">Copy to Library</button>';
    } else {
      html += '<button class="smd-action-btn smd-catalog-dl-btn" data-idx="' + idx + '" title="Copy to local disk library">Copy to Library</button>';
    }
    html += '</div>';
    html += '</div>';
  });

  container.innerHTML = html;

  // Wire up copy buttons
  container.querySelectorAll('.smd-catalog-dl-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var idx = parseInt(this.getAttribute('data-idx'));
      smdShowCopyDialog(catalog[idx]);
    });
  });
}

// =========================================================
// Copy to Library dialog
// =========================================================

function smdShowCopyDialog(entry) {
  var dlg = document.getElementById('smd-copy-dialog');
  if (!dlg) return;

  document.getElementById('smd-copy-name').value = entry.name || '';
  document.getElementById('smd-copy-desc').value = entry.description || '';

  document.getElementById('smd-copy-confirm').onclick = function() {
    var name = document.getElementById('smd-copy-name').value.trim() || entry.name;
    var desc = document.getElementById('smd-copy-desc').value.trim();
    dlg.style.display = 'none';
    smdDoCopy(smdStorage.generateUUID(), name, desc, entry);
  };

  document.getElementById('smd-copy-cancel').onclick = function() {
    dlg.style.display = 'none';
  };

  dlg.style.display = '';
}

function smdDoCopy(uuid, name, description, entry) {
  var statusEl = document.getElementById('smd-download-status');
  if (statusEl) statusEl.textContent = (entry.generate ? 'Creating ' : 'Copying ') + name + '...';

  var dataPromise;
  if (entry.generate) {
    dataPromise = Promise.resolve(new Uint8Array(entry.size));
  } else {
    dataPromise = downloadImageBuffer(entry.url).then(function(buf) {
      return new Uint8Array(buf);
    });
  }

  dataPromise.then(function(data) {
    return smdStorage.storeImage(uuid, data, {
      name: name,
      description: description,
      sourceName: entry.name
    });
  }).then(function() {
    if (statusEl) {
      statusEl.textContent = name + ' added to local library';
      setTimeout(function() {
        if (statusEl && statusEl.textContent.indexOf(name) >= 0) statusEl.textContent = '';
      }, 3000);
    }
    smdRefreshAll();
  }).catch(function(err) {
    if (statusEl) statusEl.textContent = 'Failed: ' + err.message;
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
  if (typeof str !== 'string') str = String(str == null ? '' : str);
  return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

// =========================================================
// Persistence toggle handler
// =========================================================

function smdHandlePersistToggle(enabled) {
  var configToggle = document.getElementById('config-smd-persist');
  localStorage.setItem(SMD_PERSIST_KEY, enabled ? 'true' : 'false');
  if (enabled) {
    // Turning on persistence - need to reload to switch from MEMFS to OPFS
    if (confirm('Enabling persistent storage requires a page reload.\n\nThe current SMD image will be downloaded and stored in your browser.\nAfter this, disk writes will persist across sessions.\n\nReload now?')) {
      location.reload();
    } else {
      // User cancelled - revert toggle
      localStorage.setItem(SMD_PERSIST_KEY, 'false');
      if (configToggle) configToggle.checked = false;
    }
  } else {
    // Turning off persistence - reload to switch back to MEMFS
    if (confirm('Disabling persistent storage requires a page reload.\n\nThe emulator will return to demo mode (SMD image downloaded each visit).\nYour stored images will be kept in browser storage.\n\nReload now?')) {
      location.reload();
    } else {
      localStorage.setItem(SMD_PERSIST_KEY, 'true');
      if (configToggle) configToggle.checked = true;
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
var _gatewayBlockIO = false;      // true if SharedArrayBuffer available for sync block I/O

// Called by proxy when disk-list arrives from gateway sub-worker
window.onDiskList = function(data) {
  _gatewayDiskList = data;
  smdRefreshRemoteList();
  window.dispatchEvent(new CustomEvent('gateway-disk-list', { detail: data }));
};

window.onDiskConnected = function(blockIO) {
  _gatewayBlockIO = !!blockIO;
  console.log('[SMD Manager] Gateway disk I/O connected (block I/O: ' + _gatewayBlockIO + ')');
  smdRefreshRemoteList();
};

window.onDiskDisconnected = function() {
  console.log('[SMD Manager] Gateway disk I/O disconnected');
  _gatewayDiskList = null;
  _gatewayBlockIO = false;
  // Eject all gateway mounts from the registry
  if (typeof driveRegistry !== 'undefined') {
    driveRegistry.getAll().forEach(function(d) {
      if (d.source === 'gateway') driveRegistry.eject(d.type, d.unit);
    });
  }
  smdRefreshRemoteList();
  window.dispatchEvent(new CustomEvent('gateway-disk-disconnected'));
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
    container.innerHTML = '<div class="smd-empty-msg">No disk images configured on gateway server.<br>' +
      '<span style="font-size:0.85em;opacity:0.7">Add image paths to smd.images / floppy.images in gateway.conf.json</span></div>';
    return;
  }

  var html = '';

  if (smdImages.length > 0) {
    html += '<div class="smd-remote-section-title">SMD Images</div>';
    smdImages.forEach(function(img) {
      // Check registry for gateway mount state
      var mounted = false;
      var mountedToLocal = -1;
      if (typeof driveRegistry !== 'undefined') {
        for (var u2 = 0; u2 < 4; u2++) {
          var e = driveRegistry.get('smd', u2);
          if (e.mounted && e.source === 'gateway' && e.name === img.name) {
            mounted = true;
            mountedToLocal = u2;
            break;
          }
        }
      }
      html += '<div class="smd-image-card">';
      html += '<div class="smd-image-info">';
      html += '<span class="smd-image-name">' + escapeHtml(img.name) + '</span>';
      html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; Gateway unit ' + img.unit;
      if (mounted && mountedToLocal >= 0) html += ' &rarr; Local unit ' + mountedToLocal;
      html += '</span>';
      html += '</div>';
      html += '<div class="smd-image-actions">';
      if (mounted) {
        html += '<span class="smd-badge-gateway">Gateway</span>';
        html += '<button class="smd-eject-remote-btn" data-type="smd" data-unit="' + mountedToLocal + '">Eject</button>';
      } else {
        html += '<select class="smd-assign-remote-select" data-type="smd" data-remote-unit="' + img.unit + '" data-size="' + img.size + '" data-name="' + escapeHtml(img.name) + '" title="Mount to unit">';
        html += '<option value="">Mount to...</option>';
        for (var su = 0; su < 4; su++) {
          var occupied = (typeof driveRegistry !== 'undefined') ? driveRegistry.isOccupied('smd', su) : false;
          html += '<option value="' + su + '"' + (occupied ? ' disabled' : '') + '>Unit ' + su + (occupied ? ' (in use)' : '') + '</option>';
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
      // Check registry for gateway floppy mount state
      var fmounted = false;
      var fmountedToLocal = -1;
      if (typeof driveRegistry !== 'undefined') {
        for (var fu = 0; fu < 2; fu++) {
          var fe = driveRegistry.get('floppy', fu);
          if (fe.mounted && fe.source === 'gateway' && fe.name === img.name) {
            fmounted = true;
            fmountedToLocal = fu;
            break;
          }
        }
      }
      html += '<div class="smd-image-card">';
      html += '<div class="smd-image-info">';
      html += '<span class="smd-image-name">' + escapeHtml(img.name) + '</span>';
      html += '<span class="smd-image-meta">' + smdStorage.formatSize(img.size) + ' &middot; Gateway unit ' + img.unit + '</span>';
      html += '</div>';
      html += '<div class="smd-image-actions">';
      if (fmounted) {
        html += '<span class="smd-badge-gateway">Gateway</span>';
        html += '<button class="smd-eject-remote-btn" data-type="floppy" data-unit="' + fmountedToLocal + '">Eject</button>';
      } else {
        html += '<button class="smd-mount-remote-btn" data-type="floppy" data-remote-unit="' + img.unit + '" data-size="' + img.size + '" data-name="' + escapeHtml(img.name) + '">Mount</button>';
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
        var imgName = this.getAttribute('data-name') || '';
        smdMountRemote(driveType, remoteUnit, localUnit, size, imgName);
        this.value = '';
      }
    });
  });

  // Wire up mount buttons for floppy
  container.querySelectorAll('.smd-mount-remote-btn').forEach(function(btn) {
    btn.addEventListener('click', function() {
      var remoteUnit = parseInt(this.getAttribute('data-remote-unit'));
      var size = parseInt(this.getAttribute('data-size'));
      var imgName = this.getAttribute('data-name') || '';
      smdMountRemote('floppy', remoteUnit, 0, size, imgName);
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

function smdMountRemote(driveType, remoteUnit, localUnit, imageSize, imageName) {
  if (!emu || !emu.isWorkerMode || !emu.isWorkerMode()) {
    console.warn('[SMD Manager] Gateway disk mounting requires Worker mode');
    return;
  }

  var mountFn = (driveType === 'smd') ? emu.gatewayMountSMD : emu.gatewayMountFloppy;

  mountFn(localUnit, imageSize).then(function(r) {
    if (r.ok) {
      if (typeof driveRegistry !== 'undefined') {
        driveRegistry.mount(driveType, localUnit, 'gateway', imageName || ('Gateway ' + driveType), null, imageSize);
      }
      console.log('[SMD Manager] Gateway ' + driveType + ' unit ' + remoteUnit + ' mounted to local unit ' + localUnit);
    } else {
      console.error('[SMD Manager] Gateway mount failed for ' + driveType + ' unit ' + remoteUnit);
    }
    smdRefreshRemoteList();
    smdRefreshUnitDisplay();
  });
}

function smdEjectRemote(driveType, unit) {
  var unmountFn = (driveType === 'smd') ? emu.gatewayUnmountSMD : emu.gatewayUnmountFloppy;

  unmountFn(unit);
  if (typeof driveRegistry !== 'undefined') {
    driveRegistry.eject(driveType, unit);
  }
  smdRefreshRemoteList();
  smdRefreshUnitDisplay();
}

// Expose for toolbar.js
window.smdManagerShow = smdManagerShow;
window.smdManagerHide = smdManagerHide;

// Auto-refresh if the window was restored as visible (toolbar.js loads before us)
(function() {
  var win = document.getElementById('smd-manager-window');
  if (win && win.style.display !== 'none') {
    smdRefreshAll();
  }
})();
