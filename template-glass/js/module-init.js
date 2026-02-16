//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// module-init.js - WASM Module configuration, terminal output handler, disk loading
// This must be loaded BEFORE nd100wasm.js

// Define the Module configuration object before loading any Emscripten code
// In Worker mode, Module lives in the Worker - we skip this.
if (typeof USE_WORKER === 'undefined' || !USE_WORKER) {
  // Extract cache-bust version from nd100wasm.js script tag (if present)
  var _wasmCacheBust = (function() {
    var scripts = document.querySelectorAll('script[src*="nd100wasm.js"]');
    if (scripts.length > 0) {
      var m = scripts[0].src.match(/\?v=(\d+)/);
      if (m) return '?v=' + m[1];
    }
    return '';
  })();

  var Module = {
    locateFile: function(path) {
      // Append cache-bust query string to .wasm file
      if (path.endsWith('.wasm')) return path + _wasmCacheBust;
      return path;
    },
    print: function(text) { console.log(text); },
    printErr: function(text) { console.error(text); },
    onRuntimeInitialized: function() {
      console.log("WASM Runtime Initialized");
      console.log("Execution mode: Direct (main thread)");
      // Set up the JavaScript handler for terminal output
      setupTerminalOutputHandler();
      initializeSystem();
    }
  };
}

// Called by emu-proxy-worker.js when Worker WASM is ready
window.onWorkerReady = function() {
  console.log("WASM Runtime Initialized");
  console.log("Execution mode: Web Worker (background thread)");
  setupTerminalOutputHandler();
  initializeSystem();
};

// Global object to store terminal references
var terminals = {};

// Function to handle terminal output from C/WASM
function handleTerminalOutput(identCode, charCode) {
  // Route output to pop-out window if terminal is popped out
  if (typeof window.isPoppedOut === 'function' && window.isPoppedOut(identCode)) {
    window.bufferPopoutOutput(identCode, charCode);

    // Still handle loading overlay hide logic
    if (loadingOverlayVisible && !hasReceivedTerminalOutput) {
      hasReceivedTerminalOutput = true;
      hasEverStartedEmulation = true;
      setTimeout(() => {
        hideLoadingOverlay();
      }, 500);
    }
    return 1;
  }

  if (terminals[identCode] && terminals[identCode].term) {
    terminals[identCode].term.write(String.fromCharCode(charCode));

    // Hide loading overlay when we receive terminal output
    if (loadingOverlayVisible && !hasReceivedTerminalOutput) {
      hasReceivedTerminalOutput = true;
      hasEverStartedEmulation = true;
      setTimeout(() => {
        hideLoadingOverlay();
      }, 500);
    }
  } else {
    console.error(`Terminal with identCode ${identCode} not found or not initialized in handleTerminalOutput`);
  }
  return 1;
}

// Make the terminal output handler available globally for C to call
window.handleTerminalOutputFromC = function(identCode, charCode) {
  return handleTerminalOutput(identCode, charCode);
};

// Setup the terminal output handler by exposing our JS function to C
function setupTerminalOutputHandler() {
  // Prefer ring buffer mode: C writes to a buffer, JS polls after each Step().
  // This eliminates EM_ASM calls and is required for future Web Worker mode.
  if (emu && emu.hasRingBuffer()) {
    if (emu.enableRingBuffer()) {
      console.log("[Phase 1] Terminal ring buffer enabled - C writes to buffer, JS polls after Step()");
      console.log("[Phase 1] EM_ASM terminal callbacks eliminated");
      return true;
    }
  }

  // Fallback: EM_ASM callback (legacy path)
  if (!emu || !emu.hasJSTerminalHandler()) {
    console.warn("[Phase 1] FALLBACK: Using legacy terminal output handler - ring buffer not available");
    return false;
  }

  console.log("[Phase 1] FALLBACK: Using EM_ASM terminal callbacks (ring buffer not available)");
  emu.setJSTerminalOutputHandler(1);
  return true;
}

// Track which disk images loaded successfully
var diskImageStatus = { smd: false };

// Persistence mode: opt-in via Config toggle
var SMD_PERSIST_KEY = 'nd100x-smd-persist';

function isSmdPersistenceEnabled() {
  return localStorage.getItem(SMD_PERSIST_KEY) === 'true';
}

// Expected minimum sizes (sanity check - reject truncated / error pages)
var DISK_MIN_SIZES = {
  'SMD0.IMG':   1024 * 1024,  // SMD must be at least 1MB
  'FLOPPY.IMG': 512           // Floppy must be at least 1 sector
};

// Format byte count for display
function formatBytes(bytes) {
  if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + ' MB';
  if (bytes >= 1024)    return (bytes / 1024).toFixed(0) + ' KB';
  return bytes + ' B';
}

// Load a single disk image into MEMFS using XHR (supports progress).
// Returns a promise that resolves to true on success, false on failure.
function loadDiskImage(url, memfsPath) {
  var minSize = DISK_MIN_SIZES[url] || 512;

  return new Promise(function(resolve) {
    var statusEl = document.getElementById('status');
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';

    xhr.onprogress = function(e) {
      if (e.lengthComputable && statusEl) {
        var pct = Math.round((e.loaded / e.total) * 100);
        statusEl.textContent = 'Loading ' + url + '... ' + pct + '%';
      } else if (statusEl) {
        statusEl.textContent = 'Loading ' + url + '... ' + formatBytes(e.loaded);
      }
    };

    xhr.onload = function() {
      if (xhr.status < 200 || xhr.status >= 300) {
        console.error(url + ': HTTP ' + xhr.status + ' ' + xhr.statusText);
        resolve(false);
        return;
      }

      var buf = xhr.response;
      if (!buf) {
        console.error(url + ': empty response body');
        resolve(false);
        return;
      }

      if (buf.byteLength < minSize) {
        console.error(url + ': file too small (' + formatBytes(buf.byteLength) +
          ', expected at least ' + formatBytes(minSize) + ') - not a valid disk image');
        resolve(false);
        return;
      }

      // Check for HTML error pages served as the image
      var header = new Uint8Array(buf, 0, Math.min(16, buf.byteLength));
      var headerStr = String.fromCharCode.apply(null, header);
      if (headerStr.indexOf('<!DOC') === 0 || headerStr.indexOf('<html') === 0 ||
          headerStr.indexOf('<HTML') === 0) {
        console.error(url + ': received HTML instead of a disk image (server returned an error page)');
        resolve(false);
        return;
      }

      // In Worker mode, transfer ArrayBuffer to Worker
      if (typeof USE_WORKER !== 'undefined' && USE_WORKER) {
        emu.workerLoadDisk(memfsPath, buf);
        console.log(url + ' -> ' + memfsPath + ' (' + formatBytes(buf.byteLength) + ') OK [Worker]');
        resolve(true);
        return;
      }

      // Write to MEMFS (direct mode)
      try {
        emu.fsWriteFile(memfsPath, new Uint8Array(buf));
      } catch (e) {
        console.error(url + ': FS.writeFile failed: ' + e.message);
        resolve(false);
        return;
      }

      // Set permissions
      try { emu.fsChmod(memfsPath, 0o666); } catch (e) { /* ignore */ }

      // Verify readback
      try {
        var stat = emu.fsStat(memfsPath);
        if (stat.size !== buf.byteLength) {
          console.error(url + ': MEMFS size mismatch (wrote ' + buf.byteLength +
            ', got ' + stat.size + ')');
          resolve(false);
          return;
        }
      } catch (e) {
        console.error(url + ': FS.stat failed after write: ' + e.message);
        resolve(false);
        return;
      }

      console.log(url + ' -> ' + memfsPath + ' (' + formatBytes(buf.byteLength) + ') OK');
      resolve(true);
    };

    xhr.onerror = function() {
      console.error(url + ': network error (server unreachable or CORS blocked)');
      resolve(false);
    };

    xhr.ontimeout = function() {
      console.error(url + ': request timed out');
      resolve(false);
    };

    xhr.timeout = 120000; // 2 minute timeout for large images
    xhr.send();
  });
}

// Download an image with XHR and return the ArrayBuffer (for OPFS import)
function downloadImageBuffer(url) {
  return new Promise(function(resolve, reject) {
    var statusEl = document.getElementById('status');
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, true);
    xhr.responseType = 'arraybuffer';

    xhr.onprogress = function(e) {
      if (e.lengthComputable && statusEl) {
        var pct = Math.round((e.loaded / e.total) * 100);
        statusEl.textContent = 'Downloading ' + url + '... ' + pct + '%';
      } else if (statusEl) {
        statusEl.textContent = 'Downloading ' + url + '... ' + formatBytes(e.loaded);
      }
    };

    xhr.onload = function() {
      if (xhr.status >= 200 && xhr.status < 300 && xhr.response) {
        resolve(xhr.response);
      } else {
        reject(new Error('HTTP ' + xhr.status));
      }
    };
    xhr.onerror = function() { reject(new Error('Network error')); };
    xhr.timeout = 120000;
    xhr.send();
  });
}

// Load disk images - respects persistence mode
// Demo mode: XHR to MEMFS (current behavior)
// Persistent mode: mount from OPFS (no XHR if images exist)
function loadDiskImages() {
  if (!emu || !emu.fsAvailable()) {
    console.error("ERROR: Module.FS is not available!");
    var statusElement = document.getElementById('status');
    if (statusElement) {
      statusElement.textContent = 'Error: FS API not available. Check console.';
    }
    return Promise.resolve();
  }

  // Check if persistence is enabled
  if (isSmdPersistenceEnabled()) {
    return loadDiskImagesPersistent();
  }

  // Demo mode: load SMD from server into MEMFS (original behavior)
  return loadDiskImage('SMD0.IMG', '/SMD0.IMG').then(function(smdOk) {
    diskImageStatus.smd = smdOk;

    var statusElement = document.getElementById('status');
    if (!statusElement) return;

    if (diskImageStatus.smd) {
      statusElement.textContent = 'WebAssembly module loaded!';
    } else {
      statusElement.textContent = 'Warning: Failed to load SMD0.IMG. Check console.';
      console.warn('SMD0.IMG not found. Ensure disk image is served alongside the WASM files.');
    }
  });
}

// Persistent mode: mount SMD units from OPFS
function loadDiskImagesPersistent() {
  var statusEl = document.getElementById('status');

  return smdStorage.init().then(function(available) {
    if (!available) {
      console.warn('[Persist] OPFS not available, falling back to demo mode');
      if (statusEl) statusEl.textContent = 'OPFS unavailable - using demo mode';
      return loadDiskImage('SMD0.IMG', '/SMD0.IMG').then(function(ok) {
        diskImageStatus.smd = ok;
      });
    }

    return smdStorage.refreshMetadata().then(function() {
      var units = smdStorage.getUnitAssignments();
      var hasAnyUnit = false;
      for (var u = 0; u < 4; u++) {
        if (units[u]) { hasAnyUnit = true; break; }
      }

      if (!hasAnyUnit) {
        // No images stored yet - download default to OPFS, then mount
        if (statusEl) statusEl.textContent = 'First-time setup: downloading SMD0.IMG...';
        return downloadImageBuffer('SMD0.IMG').then(function(buf) {
          var data = new Uint8Array(buf);
          return smdStorage.storeImage('SMD0.IMG', data, {
            name: 'SINTRAN III/VSE K03',
            description: 'Default system disk'
          }).then(function() {
            smdStorage.setUnitAssignment(0, 'SMD0.IMG');
            smdStorage.setBootUnit(0);
            return smdStorage.requestPersistence();
          }).then(function() {
            return mountPersistentUnits();
          });
        }).catch(function(err) {
          console.error('[Persist] Download failed:', err);
          if (statusEl) statusEl.textContent = 'Download failed - using demo mode';
          return loadDiskImage('SMD0.IMG', '/SMD0.IMG').then(function(ok) {
            diskImageStatus.smd = ok;
          });
        });
      }

      // Images exist in OPFS - mount them
      return mountPersistentUnits();
    });
  });
}

// Mount all assigned units from OPFS
function mountPersistentUnits() {
  var units = smdStorage.getUnitAssignments();
  var promises = [];

  for (var u = 0; u < 4; u++) {
    if (!units[u]) continue;

    (function(unit, fileName) {
      if (emu.isWorkerMode()) {
        // Worker mode: use SyncAccessHandle (true per-block persistence)
        promises.push(
          emu.opfsMountSMD(unit, fileName).then(function(result) {
            if (result.ok) {
              console.log('[Persist] Unit ' + unit + ' mounted from OPFS: ' + fileName);
              if (unit === 0) diskImageStatus.smd = true;
              return true;
            }
            console.error('[Persist] Failed to mount unit ' + unit);
            return false;
          }).catch(function(err) {
            console.error('[Persist] Mount error unit ' + unit + ':', err);
            return false;
          })
        );
      } else {
        // Direct mode: load entire image into buffer
        promises.push(
          smdStorage.retrieveImage(fileName).then(function(data) {
            if (!data) {
              console.error('[Persist] Image not found in OPFS: ' + fileName);
              return false;
            }
            var rc = emu.mountSMDFromBuffer(unit, data);
            if (rc === 0) {
              console.log('[Persist] Unit ' + unit + ' mounted from buffer: ' + fileName + ' (' + formatBytes(data.byteLength) + ')');
              if (unit === 0) diskImageStatus.smd = true;
              return true;
            }
            console.error('[Persist] mountSMDFromBuffer failed for unit ' + unit);
            return false;
          })
        );
      }
    })(u, units[u]);
  }

  return Promise.all(promises).then(function(results) {
    var statusEl = document.getElementById('status');
    var mounted = results.filter(function(r) { return r; }).length;
    if (statusEl) {
      if (mounted > 0) {
        statusEl.textContent = 'Persistent storage: ' + mounted + ' drive(s) mounted';
      } else {
        statusEl.textContent = 'No drives mounted from persistent storage';
      }
    }
  });
}

// Main initialization function called from Module.onRuntimeInitialized
function initializeSystem() {
  console.log("System initialization starting...");
  document.getElementById('toolbar-power').disabled = false;

  loadDiskImages()
    .then(() => {
      return document.fonts.ready;
    })
    .then(() => {
      initializeTerminals();
      console.log("Terminals initialized");
    })
    .catch(error => {
      console.error("Error during initialization:", error);
      document.getElementById('status').textContent = 'Error during initialization: ' + error.message;
    });
}
