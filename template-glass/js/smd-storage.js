//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// smd-storage.js - OPFS-based persistent storage for SMD disk images
//
// Provides browser-local persistent storage so SMD images survive across sessions.
// Worker mode uses SyncAccessHandle for per-block persistence.
// Direct mode uses async OPFS API with auto-save on stop.
// Falls back gracefully when OPFS is unavailable.

var smdStorage = (function() {
  'use strict';

  // =========================================================
  // State
  // =========================================================
  var _initialized = false;
  var _available = false;        // Is OPFS usable?
  var _opfsRoot = null;          // OPFS root directory handle
  var _imagesDir = null;         // 'smd-images' directory handle
  var _metadata = {};            // fileName -> { name, fileName, size, date, description }
  var _dirty = {};               // unit -> true (Direct mode dirty tracking)

  // localStorage keys
  var UNITS_KEY = 'nd100x-smd-units';
  var BOOT_KEY = 'nd100x-smd-boot-unit';
  var META_KEY = 'nd100x-smd-metadata';  // fallback if OPFS metadata unavailable

  // =========================================================
  // Initialization
  // =========================================================
  async function init() {
    if (_initialized) return _available;

    // Try OPFS
    try {
      if (typeof navigator.storage !== 'undefined' &&
          typeof navigator.storage.getDirectory === 'function') {
        _opfsRoot = await navigator.storage.getDirectory();
        _imagesDir = await _opfsRoot.getDirectoryHandle('smd-images', { create: true });
        _available = true;
        console.log('[SMD Storage] OPFS available');
      }
    } catch (e) {
      console.warn('[SMD Storage] OPFS not available:', e.message);
      _available = false;
    }

    // Load metadata from localStorage (always, as backup)
    try {
      var saved = localStorage.getItem(META_KEY);
      if (saved) _metadata = JSON.parse(saved);
    } catch (e) {}

    _initialized = true;
    return _available;
  }

  function isAvailable() {
    return _available;
  }

  // =========================================================
  // Metadata persistence
  // =========================================================
  function _saveMetadata() {
    try {
      localStorage.setItem(META_KEY, JSON.stringify(_metadata));
    } catch (e) {}
  }

  // =========================================================
  // Image CRUD
  // =========================================================

  // Store an image to OPFS
  // fileName: string, data: Uint8Array, meta: { name, description }
  async function storeImage(fileName, data, meta) {
    if (!_available || !_imagesDir) {
      throw new Error('OPFS not available');
    }

    var fileHandle = await _imagesDir.getFileHandle(fileName, { create: true });
    var writable = await fileHandle.createWritable();
    await writable.write(data);
    await writable.close();

    _metadata[fileName] = {
      name: (meta && meta.name) || fileName,
      fileName: fileName,
      size: data.byteLength,
      date: new Date().toISOString().split('T')[0],
      description: (meta && meta.description) || ''
    };
    _saveMetadata();

    console.log('[SMD Storage] Stored ' + fileName + ' (' + formatSize(data.byteLength) + ')');
    return true;
  }

  // Retrieve an image from OPFS as Uint8Array
  async function retrieveImage(fileName) {
    if (!_available || !_imagesDir) return null;

    try {
      var fileHandle = await _imagesDir.getFileHandle(fileName);
      var file = await fileHandle.getFile();
      var buffer = await file.arrayBuffer();
      return new Uint8Array(buffer);
    } catch (e) {
      console.warn('[SMD Storage] Could not retrieve ' + fileName + ':', e.message);
      return null;
    }
  }

  // Delete an image from OPFS
  async function deleteImage(fileName) {
    if (!_available || !_imagesDir) return false;

    try {
      await _imagesDir.removeEntry(fileName);
    } catch (e) {
      // File may not exist
    }

    delete _metadata[fileName];
    _saveMetadata();

    // Remove from any unit assignments
    var units = getUnitAssignments();
    for (var u = 0; u < 4; u++) {
      if (units[u] === fileName) {
        clearUnitAssignment(u);
      }
    }

    console.log('[SMD Storage] Deleted ' + fileName);
    return true;
  }

  // List all stored images (returns array of metadata objects)
  function listImages() {
    var result = [];
    for (var fn in _metadata) {
      result.push(_metadata[fn]);
    }
    return result;
  }

  // Quick existence check
  function imageExists(fileName) {
    return !!_metadata[fileName];
  }

  // Refresh metadata by scanning OPFS directory
  async function refreshMetadata() {
    if (!_available || !_imagesDir) return;

    var found = {};
    for await (var entry of _imagesDir.values()) {
      if (entry.kind === 'file' && entry.name.endsWith('.IMG')) {
        found[entry.name] = true;
        if (!_metadata[entry.name]) {
          // File exists in OPFS but not in metadata - add basic entry
          try {
            var file = await entry.getFile();
            _metadata[entry.name] = {
              name: entry.name,
              fileName: entry.name,
              size: file.size,
              date: new Date().toISOString().split('T')[0],
              description: ''
            };
          } catch (e) {}
        }
      }
    }

    // Remove metadata for files that no longer exist
    for (var fn in _metadata) {
      if (!found[fn]) {
        delete _metadata[fn];
      }
    }

    _saveMetadata();
  }

  // =========================================================
  // Unit assignments (persisted in localStorage)
  // =========================================================

  function getUnitAssignments() {
    try {
      var saved = localStorage.getItem(UNITS_KEY);
      if (saved) return JSON.parse(saved);
    } catch (e) {}
    return { 0: null, 1: null, 2: null, 3: null };
  }

  function setUnitAssignment(unit, fileName) {
    if (unit < 0 || unit > 3) return;
    var units = getUnitAssignments();
    units[unit] = fileName;
    try {
      localStorage.setItem(UNITS_KEY, JSON.stringify(units));
    } catch (e) {}
  }

  function clearUnitAssignment(unit) {
    setUnitAssignment(unit, null);
  }

  // =========================================================
  // Boot drive (persisted in localStorage)
  // =========================================================

  function getBootUnit() {
    try {
      var saved = localStorage.getItem(BOOT_KEY);
      if (saved !== null) {
        var u = parseInt(saved);
        if (u >= 0 && u <= 3) return u;
      }
    } catch (e) {}
    return 0;  // default: unit 0
  }

  function setBootUnit(unit) {
    if (unit < 0 || unit > 3) return;
    try {
      localStorage.setItem(BOOT_KEY, '' + unit);
    } catch (e) {}
  }

  // =========================================================
  // Dirty tracking (session-scoped, Direct mode auto-save)
  // =========================================================

  function markDirty(unit) {
    _dirty[unit] = true;
  }

  function isDirty(unit) {
    return !!_dirty[unit];
  }

  function clearDirty(unit) {
    delete _dirty[unit];
  }

  function getDirtyUnits() {
    var result = [];
    for (var u in _dirty) {
      if (_dirty[u]) result.push(parseInt(u));
    }
    return result;
  }

  // =========================================================
  // Storage info
  // =========================================================

  async function getStorageEstimate() {
    try {
      if (navigator.storage && navigator.storage.estimate) {
        return await navigator.storage.estimate();
      }
    } catch (e) {}
    return { usage: 0, quota: 0 };
  }

  async function requestPersistence() {
    try {
      if (navigator.storage && navigator.storage.persist) {
        var granted = await navigator.storage.persist();
        console.log('[SMD Storage] Persistence ' + (granted ? 'granted' : 'denied'));
        return granted;
      }
    } catch (e) {}
    return false;
  }

  // =========================================================
  // OPFS file handle access (for Worker SyncAccessHandle)
  // =========================================================

  // Get the raw file handle for a stored image (for Worker to open SyncAccessHandle)
  async function getFileHandle(fileName) {
    if (!_available || !_imagesDir) return null;
    try {
      return await _imagesDir.getFileHandle(fileName);
    } catch (e) {
      return null;
    }
  }

  // Get the images directory handle (for Worker)
  function getImagesDir() {
    return _imagesDir;
  }

  // =========================================================
  // Helpers
  // =========================================================

  function formatSize(bytes) {
    if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + ' MB';
    if (bytes >= 1024) return (bytes / 1024).toFixed(0) + ' KB';
    return bytes + ' B';
  }

  // =========================================================
  // Public API
  // =========================================================

  return {
    init: init,
    isAvailable: isAvailable,

    // Image CRUD
    storeImage: storeImage,
    retrieveImage: retrieveImage,
    deleteImage: deleteImage,
    listImages: listImages,
    imageExists: imageExists,
    refreshMetadata: refreshMetadata,

    // Unit assignments
    getUnitAssignments: getUnitAssignments,
    setUnitAssignment: setUnitAssignment,
    clearUnitAssignment: clearUnitAssignment,

    // Boot drive
    getBootUnit: getBootUnit,
    setBootUnit: setBootUnit,

    // Dirty tracking
    markDirty: markDirty,
    isDirty: isDirty,
    clearDirty: clearDirty,
    getDirtyUnits: getDirtyUnits,

    // Storage info
    getStorageEstimate: getStorageEstimate,
    requestPersistence: requestPersistence,

    // OPFS handles (for Worker integration)
    getFileHandle: getFileHandle,
    getImagesDir: getImagesDir,

    // Helpers
    formatSize: formatSize
  };

})();
