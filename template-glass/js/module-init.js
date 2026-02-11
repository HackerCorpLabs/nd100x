// module-init.js - WASM Module configuration, terminal output handler, disk loading
// This must be loaded BEFORE nd100wasm.js

// Define the Module configuration object before loading any Emscripten code
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
    // Set up the JavaScript handler for terminal output
    setupTerminalOutputHandler();
    initializeSystem();
  }
};

// Global object to store terminal references
var terminals = {};

// Function to handle terminal output from C/WASM
function handleTerminalOutput(identCode, charCode) {
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
  if (typeof Module._TerminalOutputToJS === 'undefined' ||
      typeof Module._SetJSTerminalOutputHandler === 'undefined') {
    console.warn("Using legacy terminal output handler - new JS handler functions not found in WASM module");
    return false;
  }

  console.log("Setting up JavaScript terminal output handler...");
  Module._SetJSTerminalOutputHandler(1);
  console.log("JavaScript terminal output handler registered successfully");
  return true;
}

// Track which disk images loaded successfully
var diskImageStatus = { smd: false };

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

      // Write to MEMFS
      try {
        Module.FS.writeFile(memfsPath, new Uint8Array(buf));
      } catch (e) {
        console.error(url + ': FS.writeFile failed: ' + e.message);
        resolve(false);
        return;
      }

      // Set permissions
      try { Module.FS.chmod(memfsPath, 0o666); } catch (e) { /* ignore */ }

      // Verify readback
      try {
        var stat = Module.FS.stat(memfsPath);
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

// Function to load the disk images
function loadDiskImages() {
  if (typeof Module.FS === 'undefined') {
    console.error("ERROR: Module.FS is not available!");
    var statusElement = document.getElementById('status');
    if (statusElement) {
      statusElement.textContent = 'Error: FS API not available. Check console.';
    }
    return Promise.resolve();
  }

  // Load SMD disk image only. Floppy images are loaded on demand via Floppy Library.
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
