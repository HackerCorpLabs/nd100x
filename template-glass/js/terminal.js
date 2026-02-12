//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs — https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//

// terminal.js - Terminal creation, switching, input handling, font/color settings

// Store all terminals and their containers
let terminalContainers = {};
let activeTerminalId = 1;
const RAW_MODE = true;
let isInitialized = false;

// Track floating terminal windows for cleanup
var floatingTerminalWindows = {};
var floatingTerminalCount = 0;

// Map identCode -> display name (e.g. "5 (TERMINAL 5/ TET12)")
var terminalDisplayNames = {};

// Per-terminal settings: { [identCode]: { fontFamily, colorTheme } }
var terminalSettings = {};

// Defaults for new terminals
var defaultFontFamily = "monospace";
var defaultColorTheme = 'green';

// Color theme map
const colorThemes = {
  green:      { background: 'transparent', foreground: '#00FF00', cursor: '#00FF00' },
  amber:      { background: 'transparent', foreground: '#FFBF00', cursor: '#FFBF00' },
  white:      { background: 'transparent', foreground: '#F8F8F8', cursor: '#F8F8F8' },
  blue:       { background: 'transparent', foreground: '#00BFFF', cursor: '#00BFFF' },
  paperwhite: { background: '#f0f0f5', foreground: '#222222', cursor: '#222222' }
};

// Check if float mode is enabled (default: true for new users)
function isFloatMode() {
  var val = localStorage.getItem('terminal-float-mode');
  if (val === null) return true;
  return val === 'true';
}

// Function to measure the actual character cell size for a given font and size
function measureCharSize(fontFamily, fontSize) {
  const span = document.createElement('span');
  span.textContent = 'M';
  span.style.position = 'absolute';
  span.style.visibility = 'hidden';
  span.style.fontFamily = fontFamily;
  span.style.fontSize = fontSize + 'px';
  document.body.appendChild(span);
  const rect = span.getBoundingClientRect();
  document.body.removeChild(span);
  return { width: rect.width, height: rect.height };
}

// Get settings for a terminal (returns defaults if not set)
function getTerminalSettings(identCode) {
  if (!terminalSettings[identCode]) {
    terminalSettings[identCode] = {
      fontFamily: defaultFontFamily,
      colorTheme: defaultColorTheme
    };
  }
  return terminalSettings[identCode];
}

// Save per-terminal settings to localStorage
function saveTerminalSettings() {
  try {
    localStorage.setItem('terminal-settings', JSON.stringify(terminalSettings));
  } catch(e) {}
}

// Load per-terminal settings from localStorage
function loadTerminalSettings() {
  try {
    var saved = localStorage.getItem('terminal-settings');
    if (saved) {
      terminalSettings = JSON.parse(saved);
    }
  } catch(e) {}
}

// Load settings on startup
loadTerminalSettings();

// Build the font select dropdown HTML for floating terminal headers
function buildFontSelectHTML(identCode) {
  var settings = getTerminalSettings(identCode);
  var opts = [
    { val: "monospace", label: "Monospace" },
    { val: "'VT323', monospace", label: "VT323" },
    { val: "'Fira Mono', monospace", label: "Fira Mono" },
    { val: "'IBM Plex Mono', monospace", label: "IBM Plex" },
    { val: "'Cascadia Mono', monospace", label: "Cascadia" },
    { val: "'Source Code Pro', monospace", label: "Source Code" }
  ];
  var html = '<select class="term-header-select" title="Font">';
  opts.forEach(function(o) {
    var sel = (o.val === settings.fontFamily) ? ' selected' : '';
    html += '<option value="' + o.val + '"' + sel + '>' + o.label + '</option>';
  });
  html += '</select>';
  return html;
}

// Build the color select dropdown HTML for floating terminal headers
function buildColorSelectHTML(identCode) {
  var settings = getTerminalSettings(identCode);
  var opts = [
    { val: "green", label: "Green" },
    { val: "amber", label: "Amber" },
    { val: "white", label: "White" },
    { val: "blue", label: "Blue" },
    { val: "paperwhite", label: "Paper" }
  ];
  var html = '<select class="term-header-select" title="Color">';
  opts.forEach(function(o) {
    var sel = (o.val === settings.colorTheme) ? ' selected' : '';
    html += '<option value="' + o.val + '"' + sel + '>' + o.label + '</option>';
  });
  html += '</select>';
  return html;
}

// Create a floating glass window for a terminal
function createFloatingTerminalWindow(identCode, name) {
  var winId = 'float-term-' + identCode;

  // Create the glass window
  var win = document.createElement('div');
  win.id = winId;
  win.className = 'glass-window';
  win.style.display = 'none';
  win.style.width = '700px';
  win.style.height = '460px';
  win.style.minWidth = '400px';
  win.style.minHeight = '250px';
  win.style.flexDirection = 'column';

  // Stagger position using sequential counter (not identCode which can be large)
  var offset = floatingTerminalCount * 30;
  floatingTerminalCount++;
  win.style.top = (120 + offset) + 'px';
  win.style.left = (100 + offset) + 'px';

  // Header
  var header = document.createElement('div');
  header.className = 'glass-window-header';
  header.id = winId + '-header';
  header.innerHTML =
    '<span class="glass-window-title">Terminal ' + name + '</span>' +
    '<div class="float-term-header-controls">' +
      buildFontSelectHTML(identCode) +
      buildColorSelectHTML(identCode) +
      '<button class="glass-window-close" id="' + winId + '-close">' +
        '<svg width="12" height="12" viewBox="0 0 12 12" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round">' +
        '<line x1="2" y1="2" x2="10" y2="10"/><line x1="10" y1="2" x2="2" y2="10"/></svg>' +
      '</button>' +
    '</div>';

  // Resize handle
  var resizeHandle = document.createElement('div');
  resizeHandle.className = 'glass-resize-handle';
  resizeHandle.id = winId + '-resize';
  resizeHandle.innerHTML =
    '<svg class="glass-resize-grip" viewBox="0 0 12 12" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round">' +
    '<line x1="11" y1="1" x2="1" y2="11"/><line x1="11" y1="5" x2="5" y2="11"/><line x1="11" y1="9" x2="9" y2="11"/></svg>';

  // Body
  var body = document.createElement('div');
  body.className = 'float-terminal-body';

  // Terminal container inside the body
  var container = document.createElement('div');
  container.id = 'terminal-container-' + identCode;
  container.className = 'terminal-container active';
  body.appendChild(container);

  win.appendChild(header);
  win.appendChild(resizeHandle);
  win.appendChild(body);
  document.body.appendChild(win);

  // Close button - hide window, set carrier missing, update submenu
  document.getElementById(winId + '-close').addEventListener('click', function() {
    closeWindow(winId);
    // Signal carrier loss to SINTRAN so it disconnects the user
    if (typeof Module !== 'undefined' && Module._SetTerminalCarrier) {
      Module._SetTerminalCarrier(1, identCode);
    }
    updateTerminalSubmenu();
  });

  // Font/color selects in the floating header
  var selects = header.querySelectorAll('.term-header-select');
  var fontSel = selects[0];
  var colorSel = selects[1];

  fontSel.addEventListener('change', function(e) {
    var s = getTerminalSettings(identCode);
    s.fontFamily = e.target.value;
    saveTerminalSettings();
    applySettingsToTerminal(identCode);
  });

  colorSel.addEventListener('change', function(e) {
    var s = getTerminalSettings(identCode);
    s.colorTheme = e.target.value;
    saveTerminalSettings();
    applySettingsToTerminal(identCode);
  });

  // Click to focus - set activeTerminalId
  win.addEventListener('mousedown', function() {
    activeTerminalId = identCode;
    // Update main terminal header selects to reflect this terminal
    var mainFont = document.getElementById('font-family-select');
    var mainColor = document.getElementById('color-theme-select');
    var settings = getTerminalSettings(identCode);
    if (mainFont) mainFont.value = settings.fontFamily;
    if (mainColor) mainColor.value = settings.colorTheme;
  });

  // Make draggable and resizable
  makeDraggable(win, header, 'float-term-' + identCode + '-pos');
  makeResizable(win, resizeHandle, 'float-term-' + identCode + '-size', 400, 250);

  // Register with window manager
  windowManager.register(winId, 'Term ' + name);

  // Restore visibility (default to hidden, only show if explicitly opened)
  try {
    var vis = JSON.parse(localStorage.getItem('window-visibility') || '{}');
    if (vis[winId] === true) {
      win.style.display = 'flex';
    } else {
      win.style.display = 'none';
    }
  } catch(e) {
    win.style.display = 'none';
  }

  // Track for cleanup
  floatingTerminalWindows[identCode] = win;

  return container;
}

// Remove all floating terminal windows from DOM
function cleanupFloatingTerminals() {
  Object.keys(floatingTerminalWindows).forEach(function(id) {
    var win = floatingTerminalWindows[id];
    if (win && win.parentNode) {
      win.parentNode.removeChild(win);
    }
  });
  floatingTerminalWindows = {};
}

// Create a terminal: either as a tab or floating window
function createTerminal(identCode, name) {
  // Store display name for menu use
  terminalDisplayNames[identCode] = name;

  var useFloat = (identCode !== 1) && isFloatMode();
  var container;

  if (useFloat) {
    // Create floating window
    container = createFloatingTerminalWindow(identCode, name);
  } else {
    // Create as tab in the terminal-window
    container = document.createElement('div');
    container.id = 'terminal-container-' + identCode;
    container.className = 'terminal-container';
    var tabs = document.querySelector('.terminal-tabs');
    if (tabs.nextSibling) {
      tabs.parentNode.insertBefore(container, tabs.nextSibling);
    } else {
      tabs.parentNode.appendChild(container);
    }

    var tab = document.createElement('div');
    tab.className = 'terminal-tab';
    tab.dataset.terminal = identCode;
    tab.textContent = name;
    document.querySelector('.terminal-tabs').appendChild(tab);

    tab.addEventListener('click', function() {
      switchTerminal(identCode);
    });

    // Click on the container itself also claims focus (needed in float mode
    // where tabs may be hidden, and for general click-to-focus behavior)
    container.addEventListener('mousedown', function() {
      activeTerminalId = identCode;
    });
  }

  // Get this terminal's settings
  var settings = getTerminalSettings(identCode);

  function getResponsiveFontSize() {
    var width = container.offsetWidth || 860;
    var height = container.offsetHeight || 555;
    // Measure actual character cell size at a reference font size
    var ref = measureCharSize(settings.fontFamily, 16);
    var charWidthRatio = ref.width / 16;   // ~0.6 for monospace
    var charHeightRatio = ref.height / 16; // ~1.17 for monospace
    // Max font size where 80 cols fit in width
    var maxForWidth = Math.floor(width / (80 * charWidthRatio));
    // Max font size where 24 rows fit in height
    var maxForHeight = Math.floor(height / (24 * charHeightRatio));
    return Math.max(10, Math.min(24, Math.min(maxForWidth, maxForHeight)));
  }

  var fontSize = getResponsiveFontSize();
  var term = new Terminal({
    cursorBlink: true,
    fontSize: fontSize,
    fontFamily: settings.fontFamily,
    rows: 24,
    cols: 80,
    theme: colorThemes[settings.colorTheme]
  });

  var fitAddon = new FitAddon.FitAddon();
  term.loadAddon(fitAddon);
  term.open(container);
  fitAddon.fit();

  function resizeTerminal() {
    // Clamp parent window to viewport if it overflows (shrink only, never auto-grow)
    var win = useFloat ? floatingTerminalWindows[identCode]
                       : document.getElementById('terminal-window');
    if (win && !win.classList.contains('maximized')) {
      var maxW = window.innerWidth - 20;
      var maxH = window.innerHeight - 60;
      if (win.offsetWidth > maxW) win.style.width = maxW + 'px';
      if (win.offsetHeight > maxH) win.style.height = maxH + 'px';
    }

    var newFont = getResponsiveFontSize();
    if (term.options.fontSize !== newFont) {
      term.options.fontSize = newFont;
    }
    fitAddon.fit();
    term.refresh(0, term.rows - 1);
  }
  window.addEventListener('resize', resizeTerminal);

  // Observe parent window for size changes (resize handle, zoom CSS vars, etc.)
  var observeWin = useFloat ? floatingTerminalWindows[identCode]
                            : document.getElementById('terminal-window');
  if (observeWin) {
    var observer = new ResizeObserver(function() { resizeTerminal(); });
    observer.observe(observeWin);
  }

  terminals[identCode] = {
    term: term,
    fitAddon: fitAddon,
    container: container,
    resizeTerminal: resizeTerminal
  };

  console.log('Terminal ' + identCode + ' (' + name + ') created' + (useFloat ? ' [floating]' : ' [tab]'));
  terminalContainers[identCode] = container;

  term.onKey(function(ev) {
    var key = ev.key;
    var domEvent = ev.domEvent;
    if (identCode !== activeTerminalId) return;
    var charCode = key.charCodeAt(0);

    if (RAW_MODE) {
      if (domEvent.ctrlKey && charCode >= 65 && charCode <= 90) {
        var ctrlCode = charCode - 64;
        sendKey(ctrlCode);
      } else if (charCode === 10) {
        sendKey(13);
      } else {
        sendKey(charCode);
      }

      if (charCode === 8 || charCode === 127) {
        term.write('\b \b');
      }
    }
  });

  setTimeout(resizeTerminal, 0);
}

// Function to initialize all available terminals
function initializeTerminals() {
  // Clean up existing floating terminal windows
  cleanupFloatingTerminals();

  if (!Module || !Module._GetTerminalAddress) {
    document.querySelector('.terminal-tabs').innerHTML = '';
    document.querySelectorAll('#terminal-window-body > .terminal-container').forEach(function(el) { el.remove(); });
    createTerminal(1, '1');

    if (terminals[1]) {
      terminals[1].term.focus();
    }

    switchTerminal(1);
    updateTerminalSubmenu();
    return;
  }

  // Remove tab-based containers from main terminal window
  document.querySelectorAll('#terminal-window-body > .terminal-container').forEach(function(el) { el.remove(); });
  document.querySelector('.terminal-tabs').innerHTML = '';

  // Dispose all existing xterm instances
  Object.keys(terminals).forEach(function(identCode) {
    terminals[identCode].term.dispose();
  });
  terminals = {};
  terminalContainers = {};
  terminalDisplayNames = {};
  floatingTerminalCount = 0;

  createTerminal(1, '1');

  if (isInitialized) {
    for (var i = 0; i < 16; i++) {
      var address = Module._GetTerminalAddress(i);
      if (address !== -1) {
        var identCode = Module._GetTerminalIdentCode(i);
        if (identCode !== -1 && identCode !== 1) {
          // Use SINTRAN logical device number as display name
          var logDev = Module._GetTerminalLogicalDevice(i);
          var displayName = (logDev !== -1) ? logDev.toString() : identCode.toString();
          createTerminal(identCode, displayName);
        }
      }
    }
  }

  // In float mode, hide tab bar if only console exists
  var tabs = document.querySelector('.terminal-tabs');
  var tabCount = tabs ? tabs.children.length : 0;
  if (isFloatMode() && tabCount <= 1) {
    tabs.style.display = 'none';
  } else {
    tabs.style.display = '';
  }

  // Ensure clicking anywhere on the main terminal window reclaims focus for console
  var termWin = document.getElementById('terminal-window');
  if (termWin && !termWin._floatFocusWired) {
    termWin.addEventListener('mousedown', function() {
      // Set active to whichever tab-based terminal is currently shown
      var activeTab = document.querySelector('.terminal-tab.active');
      if (activeTab) {
        activeTerminalId = parseInt(activeTab.dataset.terminal);
      } else {
        activeTerminalId = 1;
      }
    });
    termWin._floatFocusWired = true;
  }

  switchTerminal(1);
  updateTerminalSubmenu();
}

// Update the View > Terminal submenu
function updateTerminalSubmenu() {
  var submenu = document.getElementById('terminal-submenu');
  if (!submenu) return;

  submenu.innerHTML = '';

  var hasAdditional = false;
  var floatMode = isFloatMode();

  // Collect non-console terminals
  Object.keys(terminals).forEach(function(id) {
    var identCode = parseInt(id);
    if (identCode === 1) return;
    hasAdditional = true;

    var item = document.createElement('button');
    item.className = 'toolbar-menu-item';
    var decName = terminalDisplayNames[identCode] || identCode.toString();

    if (floatMode) {
      // In float mode: show/hide the floating window
      var winId = 'float-term-' + identCode;
      var win = document.getElementById(winId);
      var visible = win && win.style.display !== 'none';
      item.innerHTML = '<span class="submenu-check">' + (visible ? '&#10003;' : '') + '</span>Terminal ' + decName;
      item.addEventListener('mousedown', function(e) { e.stopPropagation(); });
      item.addEventListener('click', (function(ic) {
        return function(e) {
          e.stopPropagation();
          toggleFloatingTerminal(ic);
          document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
        };
      })(identCode));
    } else {
      // In tab mode: switch to that tab
      item.innerHTML = '<span class="submenu-check">' + (activeTerminalId === identCode ? '&#10003;' : '') + '</span>Terminal ' + decName;
      item.addEventListener('click', (function(ic) {
        return function() {
          switchTerminal(ic);
          document.querySelectorAll('.toolbar-menu-container').forEach(function(c) { c.classList.remove('open'); });
        };
      })(identCode));
    }

    submenu.appendChild(item);
  });

  if (!hasAdditional) {
    var empty = document.createElement('div');
    empty.className = 'toolbar-menu-item disabled';
    empty.id = 'terminal-submenu-empty';
    empty.textContent = 'No additional terminals';
    submenu.appendChild(empty);
  }
}

// Toggle a floating terminal window's visibility
function toggleFloatingTerminal(identCode) {
  var winId = 'float-term-' + identCode;
  var win = document.getElementById(winId);
  if (!win) return;

  if (win.style.display === 'none') {
    openWindow(winId);
    // Restore carrier when reopening
    if (typeof Module !== 'undefined' && Module._SetTerminalCarrier) {
      Module._SetTerminalCarrier(0, identCode);
    }
    activeTerminalId = identCode;
    // Defer focus so it runs after menu-click mousedown bubbles through
    // other registered windows (which would steal z-order)
    setTimeout(function() {
      windowManager.focus(winId);
      windowManager.updateTaskbar();
      if (terminals[identCode]) {
        terminals[identCode].term.focus();
        terminals[identCode].resizeTerminal();
      }
    }, 50);
  } else {
    closeWindow(winId);
    // Signal carrier loss when closing
    if (typeof Module !== 'undefined' && Module._SetTerminalCarrier) {
      Module._SetTerminalCarrier(1, identCode);
    }
  }
  updateTerminalSubmenu();
}

// Live switch between float and tab modes
function switchTerminalMode() {
  if (!isInitialized) return;
  initializeTerminals();
  registerTerminalCallbacks();
}

// Make terminals responsive to window resize
window.addEventListener('resize', function() {
  Object.values(terminals).forEach(function(terminal) {
    terminal.resizeTerminal();
  });
});

function switchTerminal(identCode) {
  document.querySelectorAll('.terminal-tab').forEach(function(tab) {
    tab.classList.toggle('active', parseInt(tab.dataset.terminal) === identCode);
  });

  Object.keys(terminalContainers).forEach(function(termId) {
    // Only toggle tab-based containers (inside terminal-window-body)
    var cont = terminalContainers[termId];
    if (cont.closest('#terminal-window-body')) {
      cont.classList.toggle('active', parseInt(termId) === identCode);
    }
  });

  activeTerminalId = identCode;

  // Update the font/color selects to reflect this terminal's settings
  var settings = getTerminalSettings(identCode);
  var fontSelect = document.getElementById('font-family-select');
  var colorSelect = document.getElementById('color-theme-select');
  if (fontSelect) fontSelect.value = settings.fontFamily;
  if (colorSelect) colorSelect.value = settings.colorTheme;
}

// Function to register terminal callbacks
function registerTerminalCallbacks() {
  if (!Module) {
    console.error("Module not ready for terminal callback registration");
    return false;
  }

  try {
    console.log("Setting up direct JS terminal callbacks...");

    if (Object.keys(terminals).length === 0) {
      console.error("No terminals initialized");
      return false;
    }

    console.log('Found ' + Object.keys(terminals).length + ' terminals ready for input/output');

    if (typeof Module._TerminalOutputToJS !== 'undefined' &&
        typeof Module._SetJSTerminalOutputHandler !== 'undefined') {
      console.log("Using modern direct JS terminal output handler");
      return true;
    }

    console.log("Using legacy terminal callback approach with function pointers");

    if (typeof Module.addFunction === 'undefined') {
      console.error("addFunction not available");
      return false;
    }

    var singleCallback;
    try {
      singleCallback = Module.addFunction(function(identCode, charCode) {
        return handleTerminalOutput(identCode, charCode);
      }, 'iii');

      Object.keys(terminals).forEach(function(identCode) {
        console.log('Registering legacy callback for terminal with identCode ' + identCode);
        if (Module._SetTerminalOutputCallback) {
          Module._SetTerminalOutputCallback(parseInt(identCode), singleCallback);
        }
      });

      console.log("Legacy callbacks registered successfully");
      return true;
    } catch (e) {
      console.error("Error registering legacy callbacks:", e);
      return false;
    }
  } catch (e) {
    console.error("Error in registerTerminalCallbacks:", e);
    return false;
  }
}

// Function to send a key to the currently active terminal
function sendKey(keyCode) {
  if (!Module || !Module._SendKeyToTerminal) {
    console.error("SendKeyToTerminal not available");
    return false;
  }

  try {
    if (!terminals[activeTerminalId]) {
      console.error('Terminal with identCode ' + activeTerminalId + ' not found');
      return false;
    }

    var result = Module._SendKeyToTerminal(activeTerminalId, keyCode);
    if (result !== 1) {
      console.error("Failed to send key to terminal with identCode", activeTerminalId);
      return false;
    }
    return true;
  } catch (e) {
    console.error("Error sending key to terminal:", e);
    return false;
  }
}

// Apply font and color settings to a single terminal with xterm.js rendering fix
function applySettingsToTerminal(identCode) {
  var t = terminals[identCode];
  if (!t) return;
  var settings = getTerminalSettings(identCode);

  t.term.options.fontFamily = settings.fontFamily;
  t.term.options.theme = { ...colorThemes[settings.colorTheme], background: '#0a0e1c' };
  t.term.refresh(0, t.term.rows - 1);

  setTimeout(function() {
    t.term.options.theme = colorThemes[settings.colorTheme];
    t.resizeTerminal();
  }, 50);
}

// Apply settings to all terminals (used during init)
function applyAllTerminalSettings() {
  Object.keys(terminals).forEach(function(identCode) {
    applySettingsToTerminal(parseInt(identCode));
  });
}

// Initialize dropdown values to match active terminal on page load
function initializeDropdowns() {
  var settings = getTerminalSettings(activeTerminalId);
  var fontSelect = document.getElementById('font-family-select');
  var colorSelect = document.getElementById('color-theme-select');

  if (fontSelect) fontSelect.value = settings.fontFamily;
  if (colorSelect) colorSelect.value = settings.colorTheme;

  // Initialize the float terminals config toggle
  var floatToggle = document.getElementById('config-float-terminals');
  if (floatToggle) {
    floatToggle.checked = isFloatMode();
  }
}

if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', initializeDropdowns);
} else {
  initializeDropdowns();
}
