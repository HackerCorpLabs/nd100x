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

// Function to create a terminal container and tab
function createTerminal(identCode, name) {
  const container = document.createElement('div');
  container.id = `terminal-container-${identCode}`;
  container.className = 'terminal-container';
  const tabs = document.querySelector('.terminal-tabs');
  if (tabs.nextSibling) {
    tabs.parentNode.insertBefore(container, tabs.nextSibling);
  } else {
    tabs.parentNode.appendChild(container);
  }

  const tab = document.createElement('div');
  tab.className = 'terminal-tab';
  tab.dataset.terminal = identCode;
  tab.textContent = name;
  document.querySelector('.terminal-tabs').appendChild(tab);

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

  let fontSize = getResponsiveFontSize();
  const term = new Terminal({
    cursorBlink: true,
    fontSize: fontSize,
    fontFamily: settings.fontFamily,
    rows: 24,
    cols: 80,
    theme: colorThemes[settings.colorTheme]
  });

  const fitAddon = new FitAddon.FitAddon();
  term.loadAddon(fitAddon);
  term.open(container);
  fitAddon.fit();

  function resizeTerminal() {
    var newFont = getResponsiveFontSize();
    if (term.options.fontSize !== newFont) {
      term.options.fontSize = newFont;
    }
    fitAddon.fit();
    term.refresh(0, term.rows - 1);
  }
  window.addEventListener('resize', resizeTerminal);

  terminals[identCode] = {
    term,
    fitAddon,
    container,
    resizeTerminal
  };

  console.log(`Terminal ${identCode} (${name}) created.`);
  terminalContainers[identCode] = container;

  tab.addEventListener('click', () => {
    switchTerminal(identCode);
  });

  term.onKey(({ key, domEvent }) => {
    if (identCode !== activeTerminalId) return;
    const charCode = key.charCodeAt(0);

    if (RAW_MODE) {
      if (domEvent.ctrlKey && charCode >= 65 && charCode <= 90) {
        const ctrlCode = charCode - 64;
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

  term.writeln(`ND100X Emulator - ${name}`);
  term.writeln('----------------------------------');

  setTimeout(resizeTerminal, 0);
}

// Function to initialize all available terminals
function initializeTerminals() {
  if (!Module || !Module._GetTerminalAddress) {
    document.querySelector('.terminal-tabs').innerHTML = '';
    document.querySelectorAll('.terminal-container').forEach(el => el.remove());
    createTerminal(1, '1');

    if (terminals[1]) {
      terminals[1].term.focus();
    }

    switchTerminal(1);
    return;
  }

  document.querySelectorAll('.terminal-container').forEach(el => el.remove());
  document.querySelector('.terminal-tabs').innerHTML = '';
  Object.keys(terminals).forEach(identCode => {
    terminals[identCode].term.dispose();
  });
  terminals = {};
  terminalContainers = {};

  createTerminal(1, '1');

  if (isInitialized) {
    for (let i = 0; i < 16; i++) {
      const address = Module._GetTerminalAddress(i);
      if (address !== -1) {
        const identCode = Module._GetTerminalIdentCode(i);
        if (identCode !== -1 && identCode !== 1) {
          createTerminal(identCode, identCode.toString(8));
        }
      }
    }
  }

  switchTerminal(1);
}

// Make terminals responsive to window resize
window.addEventListener('resize', () => {
  Object.values(terminals).forEach(terminal => {
    terminal.resizeTerminal();
  });
});

function switchTerminal(identCode) {
  document.querySelectorAll('.terminal-tab').forEach(tab => {
    tab.classList.toggle('active', parseInt(tab.dataset.terminal) === identCode);
  });

  Object.keys(terminalContainers).forEach(termId => {
    terminalContainers[termId].classList.toggle('active', parseInt(termId) === identCode);
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

    console.log(`Found ${Object.keys(terminals).length} terminals ready for input/output`);

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

      Object.keys(terminals).forEach(identCode => {
        console.log(`Registering legacy callback for terminal with identCode ${identCode}`);
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
      console.error(`Terminal with identCode ${activeTerminalId} not found`);
      return false;
    }

    const result = Module._SendKeyToTerminal(activeTerminalId, keyCode);
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

  setTimeout(() => {
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
}

if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', initializeDropdowns);
} else {
  initializeDropdowns();
}
