//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//
// terminal-manager.ts - Main page terminal orchestration.
// Replaces terminal.js. Uses terminal-core.ts for all shared logic.
//
// Manages:
//   - Console terminal (identCode=1) in main window
//   - Floating glass windows for additional terminals
//   - Tab switching, active terminal tracking
//   - Float mode toggle
//   - Terminal submenu updates
//
// Globals preserved for other JS modules:
//   terminals, activeTerminalId, terminalContainers, terminalDisplayNames,
//   initializeTerminals, registerTerminalCallbacks, sendKey,
//   getTerminalSettings, saveTerminalSettings, applySettingsToTerminal,
//   switchTerminalMode, updateTerminalSubmenu
// Store all terminals and their containers
var terminalContainers = {};
var activeTerminalId = 1;
var isInitialized = false;
// Track floating terminal windows for cleanup
var floatingTerminalWindows = {};
var floatingTerminalCount = 0;
// Map identCode -> display name
var terminalDisplayNames = {};
// Defaults
var defaultFontFamily = "monospace";
var defaultColorTheme = "green";
// Check if float mode is enabled (default: true — "Group" toggle OFF)
function isFloatMode() {
    var val = localStorage.getItem('terminal-float-mode');
    if (val === null)
        return true;
    return val === 'true';
}
// Check if auto pop-out is enabled (default: false)
function isAutoPopout() {
    return localStorage.getItem('terminal-auto-popout') === 'true';
}
// ---- Floating terminal window creation ----
function createFloatingTerminalWindow(identCode, name) {
    var winId = 'float-term-' + identCode;
    var win = document.createElement('div');
    win.id = winId;
    win.className = 'glass-window';
    win.style.display = 'none';
    win.style.width = '820px';
    win.style.height = '520px';
    win.style.minWidth = '400px';
    win.style.minHeight = '250px';
    win.style.flexDirection = 'column';
    // Stagger position using sequential counter
    var offset = floatingTerminalCount * 30;
    floatingTerminalCount++;
    win.style.top = (120 + offset) + 'px';
    win.style.left = (100 + offset) + 'px';
    // Header
    var header = document.createElement('div');
    header.className = 'glass-window-header';
    header.id = winId + '-header';
    var emuLabel = window.getEmulatorTypeLabel();
    var titleText = 'Terminal ' + name + (emuLabel ? ' (' + emuLabel + ')' : '');
    header.innerHTML =
        '<span class="glass-window-title" id="' + winId + '-title">' + titleText + '</span>' +
            '<div class="float-term-header-controls">' +
            '<span class="term-font-size-badge" id="' + winId + '-fontsize">16px</span>' +
            window.buildFontSelectHTML(identCode) +
            window.buildColorSelectHTML(identCode) +
            '<button class="terminal-window-btn float-vk-toggle" id="float-term-' + identCode + '-vk" title="Virtual Keyboard" style="display:none">' +
            '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
            '<rect x="2" y="4" width="20" height="16" rx="2"/>' +
            '<line x1="6" y1="8" x2="6" y2="8"/><line x1="10" y1="8" x2="10" y2="8"/><line x1="14" y1="8" x2="14" y2="8"/><line x1="18" y1="8" x2="18" y2="8"/>' +
            '<line x1="6" y1="12" x2="6" y2="12"/><line x1="10" y1="12" x2="10" y2="12"/><line x1="14" y1="12" x2="14" y2="12"/><line x1="18" y1="12" x2="18" y2="12"/>' +
            '<line x1="8" y1="16" x2="16" y2="16"/>' +
            '</svg>' +
            '</button>' +
            '<button class="terminal-window-btn" id="' + winId + '-popout" title="Pop out to separate window">' +
            '<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">' +
            '<polyline points="15,3 21,3 21,9"/>' +
            '<line x1="21" y1="3" x2="14" y2="10"/>' +
            '<path d="M21 14v5a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5"/>' +
            '</svg>' +
            '</button>' +
            '<button class="terminal-window-btn terminal-window-btn-close" id="' + winId + '-close">' +
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
    // Terminal container
    var container = document.createElement('div');
    container.id = 'terminal-container-' + identCode;
    container.className = 'terminal-container active';
    body.appendChild(container);
    win.appendChild(header);
    win.appendChild(body);
    win.appendChild(resizeHandle);
    document.body.appendChild(win);
    // VK toggle button (floating window)
    var floatVkBtn = document.getElementById('float-term-' + identCode + '-vk');
    if (floatVkBtn) {
        floatVkBtn.addEventListener('click', function () {
            toggleTerminalVK(identCode);
        });
    }
    // Pop-out button
    document.getElementById(winId + '-popout').addEventListener('click', function () {
        if (typeof window.popOutTerminal === 'function') {
            window.popOutTerminal(identCode);
        }
    });
    // Close button
    document.getElementById(winId + '-close').addEventListener('click', function () {
        closeWindow(winId);
        if (typeof emu !== 'undefined') {
            emu.setTerminalCarrier(1, identCode);
        }
        updateTerminalSubmenu();
    });
    // Font/color selects
    var selects = header.querySelectorAll('.term-header-select');
    var fontSel = selects[0];
    var colorSel = selects[1];
    fontSel.addEventListener('change', function (e) {
        var s = window.getTerminalSettings(identCode);
        s.fontFamily = e.target.value;
        window.saveTerminalSettings();
        applySettingsToTerminal(identCode);
    });
    colorSel.addEventListener('change', function (e) {
        var s = window.getTerminalSettings(identCode);
        s.colorTheme = e.target.value;
        window.saveTerminalSettings();
        applySettingsToTerminal(identCode);
    });
    // Hide font dropdown in RetroTerm mode (bitmap fonts only)
    if (isRetroTermBackend) {
        fontSel.style.display = 'none';
    }
    // Click to focus
    win.addEventListener('mousedown', function () {
        activeTerminalId = identCode;
        var mainFont = document.getElementById('font-family-select');
        var mainColor = document.getElementById('color-theme-select');
        var settings = window.getTerminalSettings(identCode);
        if (mainFont)
            mainFont.value = settings.fontFamily;
        if (mainColor)
            mainColor.value = settings.colorTheme;
    });
    // Make draggable and resizable (aspect-ratio locked via computeHeight callback)
    makeDraggable(win, header, 'float-term-' + identCode + '-pos');
    makeResizable(win, resizeHandle, 'float-term-' + identCode + '-size', 400, 250, function (w) { return computeTerminalHeight(identCode, w); });
    // Register with window manager
    windowManager.register(winId, 'Term ' + name);
    // Restore visibility
    try {
        var vis = JSON.parse(localStorage.getItem('window-visibility') || '{}');
        if (vis[winId] === true) {
            win.style.display = 'flex';
        }
        else {
            win.style.display = 'none';
        }
    }
    catch (e) {
        win.style.display = 'none';
    }
    floatingTerminalWindows[identCode] = win;
    return container;
}
// Remove all floating terminal windows from DOM
function cleanupFloatingTerminals() {
    Object.keys(floatingTerminalWindows).forEach(function (id) {
        var win = floatingTerminalWindows[parseInt(id)];
        if (win && win.parentNode) {
            win.parentNode.removeChild(win);
        }
    });
    floatingTerminalWindows = {};
}
// ---- Terminal creation ----
function createTerminal(identCode, name) {
    terminalDisplayNames[identCode] = name;
    var useFloat = (identCode !== 1) && isFloatMode();
    var container;
    if (useFloat) {
        container = createFloatingTerminalWindow(identCode, name);
    }
    else {
        container = document.createElement('div');
        container.id = 'terminal-container-' + identCode;
        // Start first container as active so it has layout dimensions when the
        // terminal renderer initialises (bitmap font canvas sizing needs non-zero size).
        container.className = 'terminal-container' + (identCode === 1 ? ' active' : '');
        var tabs = document.querySelector('.terminal-tabs');
        if (tabs.nextSibling) {
            tabs.parentNode.insertBefore(container, tabs.nextSibling);
        }
        else {
            tabs.parentNode.appendChild(container);
        }
        var tab = document.createElement('div');
        tab.className = 'terminal-tab';
        tab.dataset.terminal = identCode.toString();
        tab.textContent = name;
        document.querySelector('.terminal-tabs').appendChild(tab);
        tab.addEventListener('click', function () {
            switchTerminal(identCode);
        });
        container.addEventListener('mousedown', function () {
            activeTerminalId = identCode;
        });
    }
    var settings = window.getTerminalSettings(identCode);
    var fontSizeDisplay = useFloat
        ? document.getElementById('float-term-' + identCode + '-fontsize')
        : document.getElementById('console-fontsize');
    var parentWin = useFloat ? floatingTerminalWindows[identCode]
        : document.getElementById('terminal-window');
    // Create a wrapper div inside the container for the canvas.
    // VK sits as a sibling below the wrapper so flex layout gives it space.
    var wrapper = document.createElement('div');
    wrapper.className = 'terminal-canvas-wrapper';
    container.appendChild(wrapper);
    // Create terminal using shared factory from terminal-core
    var inst = window.createScaledTerminal(wrapper, {
        fontFamily: settings.fontFamily,
        colorTheme: settings.colorTheme,
        sizeDisplay: fontSizeDisplay,
        observeResize: parentWin
    });
    var term = inst.term;
    var fitAddon = inst.fitAddon;
    var resizeTerminal = inst.resizeTerminal;
    terminals[identCode] = {
        term: term,
        fitAddon: fitAddon,
        container: container,
        wrapper: wrapper,
        resizeTerminal: resizeTerminal
    };
    // Create per-terminal VK (RetroTerm only)
    createTerminalVK(identCode, container, term);
    console.log('Terminal ' + identCode + ' (' + name + ') created' + (useFloat ? ' [floating]' : ' [tab]'));
    terminalContainers[identCode] = container;
    // Keyboard handler using core factory
    window.setupTerminalKeyHandler(term, function (keyCode) {
        sendKey(keyCode);
    }, function () {
        return identCode === activeTerminalId;
    });
    setTimeout(resizeTerminal, 0);
    // Auto pop-out floating terminals if enabled (skip console terminal)
    if (useFloat && isAutoPopout() && identCode !== 1) {
        setTimeout(function () {
            if (typeof window.popOutTerminal === 'function') {
                window.popOutTerminal(identCode);
            }
        }, 300);
    }
}
// ---- Initialize all available terminals ----
function initializeTerminals() {
    cleanupFloatingTerminals();
    if (!emu || !emu.isReady()) {
        document.querySelector('.terminal-tabs').innerHTML = '';
        document.querySelectorAll('#terminal-window-body > .terminal-container').forEach(function (el) { el.remove(); });
        createTerminal(1, '1');
        if (terminals[1]) {
            terminals[1].term.focus();
        }
        switchTerminal(1);
        updateTerminalSubmenu();
        updateConsoleTitle();
        return;
    }
    // Remove tab-based containers
    document.querySelectorAll('#terminal-window-body > .terminal-container').forEach(function (el) { el.remove(); });
    document.querySelector('.terminal-tabs').innerHTML = '';
    // Dispose all existing xterm instances
    Object.keys(terminals).forEach(function (identCode) {
        terminals[parseInt(identCode)].term.dispose();
    });
    terminals = {};
    terminalContainers = {};
    terminalDisplayNames = {};
    floatingTerminalCount = 0;
    createTerminal(1, '1');
    if (isInitialized) {
        for (var i = 0; i < 16; i++) {
            var address = emu.getTerminalAddress(i);
            if (address !== -1) {
                var identCode = emu.getTerminalIdentCode(i);
                if (identCode !== -1 && identCode !== 1) {
                    var logDev = emu.getTerminalLogicalDevice(i);
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
    }
    else {
        tabs.style.display = '';
    }
    // Click anywhere on main terminal window reclaims focus for console
    var termWin = document.getElementById('terminal-window');
    if (termWin && !termWin._floatFocusWired) {
        termWin.addEventListener('mousedown', function () {
            var activeTab = document.querySelector('.terminal-tab.active');
            if (activeTab) {
                activeTerminalId = parseInt(activeTab.dataset.terminal);
            }
            else {
                activeTerminalId = 1;
            }
        });
        termWin._floatFocusWired = true;
    }
    switchTerminal(1);
    updateTerminalSubmenu();
    updateConsoleTitle();
    fitTerminalWindowHeight();
}
// ---- Terminal submenu ----
function updateTerminalSubmenu() {
    var submenu = document.getElementById('terminal-submenu');
    if (!submenu)
        return;
    submenu.innerHTML = '';
    var hasAdditional = false;
    var floatMode = isFloatMode();
    Object.keys(terminals).forEach(function (id) {
        var identCode = parseInt(id);
        if (identCode === 1)
            return;
        hasAdditional = true;
        var item = document.createElement('button');
        item.className = 'toolbar-menu-item';
        var decName = terminalDisplayNames[identCode] || identCode.toString();
        if (floatMode) {
            var winId = 'float-term-' + identCode;
            var win = document.getElementById(winId);
            var visible = win && win.style.display !== 'none';
            item.innerHTML = '<span class="submenu-check">' + (visible ? '&#10003;' : '') + '</span>Terminal ' + decName;
            item.addEventListener('mousedown', function (e) { e.stopPropagation(); });
            item.addEventListener('click', (function (ic) {
                return function (e) {
                    e.stopPropagation();
                    toggleFloatingTerminal(ic);
                    document.querySelectorAll('.toolbar-menu-container').forEach(function (c) { c.classList.remove('open'); });
                };
            })(identCode));
        }
        else {
            item.innerHTML = '<span class="submenu-check">' + (activeTerminalId === identCode ? '&#10003;' : '') + '</span>Terminal ' + decName;
            item.addEventListener('click', (function (ic) {
                return function () {
                    switchTerminal(ic);
                    document.querySelectorAll('.toolbar-menu-container').forEach(function (c) { c.classList.remove('open'); });
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
// ---- Toggle floating terminal ----
function toggleFloatingTerminal(identCode) {
    var winId = 'float-term-' + identCode;
    var win = document.getElementById(winId);
    if (!win)
        return;
    if (win.style.display === 'none') {
        openWindow(winId);
        if (typeof emu !== 'undefined') {
            emu.setTerminalCarrier(0, identCode);
        }
        activeTerminalId = identCode;
        setTimeout(function () {
            windowManager.focus(winId);
            windowManager.updateTaskbar();
            if (terminals[identCode]) {
                terminals[identCode].term.focus();
                terminals[identCode].resizeTerminal();
            }
        }, 50);
    }
    else {
        closeWindow(winId);
        if (typeof emu !== 'undefined') {
            emu.setTerminalCarrier(1, identCode);
        }
    }
    updateTerminalSubmenu();
}
// ---- Mode switching ----
function switchTerminalMode() {
    if (!isInitialized)
        return;
    initializeTerminals();
    registerTerminalCallbacks();
}
// ---- Window resize ----
window.addEventListener('resize', function () {
    Object.values(terminals).forEach(function (terminal) {
        terminal.resizeTerminal();
    });
});
// ---- Tab switching ----
function switchTerminal(identCode) {
    document.querySelectorAll('.terminal-tab').forEach(function (tab) {
        tab.classList.toggle('active', parseInt(tab.dataset.terminal) === identCode);
    });
    Object.keys(terminalContainers).forEach(function (termId) {
        var cont = terminalContainers[parseInt(termId)];
        if (cont.closest('#terminal-window-body')) {
            cont.classList.toggle('active', parseInt(termId) === identCode);
        }
    });
    activeTerminalId = identCode;
    var settings = window.getTerminalSettings(identCode);
    var fontSelect = document.getElementById('font-family-select');
    var colorSelect = document.getElementById('color-theme-select');
    if (fontSelect)
        fontSelect.value = settings.fontFamily;
    if (colorSelect)
        colorSelect.value = settings.colorTheme;
}
// ---- Register terminal callbacks ----
function registerTerminalCallbacks() {
    if (!emu) {
        console.error("Emu proxy not ready for terminal callback registration");
        return false;
    }
    try {
        console.log("Setting up direct JS terminal callbacks...");
        if (Object.keys(terminals).length === 0) {
            console.error("No terminals initialized");
            return false;
        }
        console.log('Found ' + Object.keys(terminals).length + ' terminals ready for input/output');
        if (emu.hasJSTerminalHandler()) {
            console.log("Using modern direct JS terminal output handler");
            return true;
        }
        console.log("Using legacy terminal callback approach with function pointers");
        if (!emu.hasAddFunction()) {
            console.error("addFunction not available");
            return false;
        }
        var singleCallback;
        try {
            singleCallback = emu.addFunction(function (identCode, charCode) {
                return handleTerminalOutput(identCode, charCode);
            }, 'iii');
            Object.keys(terminals).forEach(function (identCode) {
                console.log('Registering legacy callback for terminal with identCode ' + identCode);
                if (emu.hasSetTerminalOutputCallback()) {
                    emu.setTerminalOutputCallback(parseInt(identCode), singleCallback);
                }
            });
            console.log("Legacy callbacks registered successfully");
            return true;
        }
        catch (e) {
            console.error("Error registering legacy callbacks:", e);
            return false;
        }
    }
    catch (e) {
        console.error("Error in registerTerminalCallbacks:", e);
        return false;
    }
}
// ---- Send key ----
function sendKey(keyCode) {
    if (!emu) {
        console.error("SendKeyToTerminal not available");
        return false;
    }
    try {
        if (!terminals[activeTerminalId]) {
            console.error('Terminal with identCode ' + activeTerminalId + ' not found');
            return false;
        }
        var result = emu.sendKey(activeTerminalId, keyCode);
        if (result !== 1) {
            console.error("Failed to send key to terminal with identCode", activeTerminalId);
            return false;
        }
        return true;
    }
    catch (e) {
        console.error("Error sending key to terminal:", e);
        return false;
    }
}
// ---- Backend detection ----
var isRetroTermBackend = (typeof TERMINAL_BACKEND !== 'undefined' && TERMINAL_BACKEND === 'retroterm');
// ---- Apply settings ----
function applySettingsToTerminal(identCode) {
    // If popped out, broadcast settings to pop-out window
    if (typeof window.isPoppedOut === 'function' && window.isPoppedOut(identCode)) {
        if (typeof window.broadcastSettingsChange === 'function') {
            window.broadcastSettingsChange(identCode);
        }
        return;
    }
    var t = terminals[identCode];
    if (!t)
        return;
    var settings = window.getTerminalSettings(identCode);
    var colorThemes = window.terminalColorThemes;
    // Skip font-related assignments for RetroTerm (bitmap fonts only)
    if (!isRetroTermBackend) {
        t.term.options.fontFamily = settings.fontFamily;
    }
    t.term.options.theme = Object.assign({}, colorThemes[settings.colorTheme], { background: '#0a0e1c' });
    t.term.refresh(0, t.term.rows - 1);
    setTimeout(function () {
        t.term.options.theme = colorThemes[settings.colorTheme];
        t.resizeTerminal();
    }, 50);
}
function applyAllTerminalSettings() {
    Object.keys(terminals).forEach(function (identCode) {
        applySettingsToTerminal(parseInt(identCode));
    });
}
// ---- Per-terminal VirtualKeyboard integration (RetroTerm only) ----
/** Auto-size console window height to fit canvas content.
 *  Computes canvas height from aspect ratio (not current DOM measurement)
 *  to avoid measuring a canvas that flex layout has already squeezed. */
function fitTerminalWindowHeight() {
    var win = document.getElementById('terminal-window');
    if (!win)
        return;
    // Skip when maximized -- flex layout handles everything
    if (win.classList.contains('maximized'))
        return;
    var header = win.querySelector('.terminal-window-header');
    var body = win.querySelector('.terminal-window-body');
    if (!header || !body)
        return;
    var wrapper = body.querySelector('.terminal-canvas-wrapper');
    if (!wrapper)
        return;
    requestAnimationFrame(function () {
        requestAnimationFrame(function () {
            // Compute canvas height from wrapper width + terminal aspect ratio
            // (wrapper width is stable; height may be squeezed by VK flex)
            var contentW = wrapper.clientWidth;
            var canvasRatio = (24 * 16) / (80 * 9); // fallback
            var t = terminals[activeTerminalId];
            if (t && t.term) {
                var renderer = t.term._renderer || t.term.renderer;
                if (renderer) {
                    var cw = renderer.charWidth || 9;
                    var ch = renderer.charHeight || 16;
                    canvasRatio = (24 * ch) / (80 * cw);
                }
            }
            var canvasH = contentW * canvasRatio;
            var headerH = header.getBoundingClientRect().height;
            var tabsEl = body.querySelector('.terminal-tabs');
            var tabsH = tabsEl ? tabsEl.getBoundingClientRect().height : 0;
            // Account for VK if visible (use SVG viewBox ratio, not DOM measurement)
            var vkH = 0;
            var activeContainer = body.querySelector('.terminal-container.active');
            if (activeContainer) {
                var vkEl = activeContainer.querySelector('.terminal-vk-container');
                if (vkEl && vkEl.style.display !== 'none') {
                    var svg = vkEl.querySelector('svg.retroterm-vk');
                    if (svg) {
                        var vb = svg.viewBox.baseVal;
                        if (vb && vb.width > 0 && vb.height > 0) {
                            vkH = contentW * (vb.height / vb.width);
                        }
                    }
                }
            }
            var padding = 20; // resize handle + borders
            win.style.height = Math.ceil(headerH + tabsH + canvasH + vkH + padding) + 'px';
        });
    });
}
/** Create VK container and instance for a single terminal */
function createTerminalVK(identCode, container, term) {
    if (!isRetroTermBackend || typeof RetroTerm === 'undefined')
        return;
    if (typeof RetroTerm.VirtualKeyboard === 'undefined')
        return;
    var vkContainer = document.createElement('div');
    vkContainer.className = 'terminal-vk-container';
    vkContainer.style.display = 'none';
    container.appendChild(vkContainer);
    try {
        var vk = new RetroTerm.VirtualKeyboard(vkContainer);
        var name = terminalDisplayNames[identCode] || identCode.toString();
        vk.attachTerminal(term, 'Terminal ' + name);
        terminals[identCode].vk = vk;
        terminals[identCode].vkContainer = vkContainer;
        // Show VK toggle button for console window
        if (identCode === 1 || !isFloatMode()) {
            var vkBtn = document.getElementById('term-vk-toggle');
            if (vkBtn)
                vkBtn.style.display = '';
        }
        // Show VK toggle button for floating windows
        var floatVkBtn = document.getElementById('float-term-' + identCode + '-vk');
        if (floatVkBtn)
            floatVkBtn.style.display = '';
    }
    catch (e) {
        console.warn('VirtualKeyboard init failed for terminal ' + identCode + ':', e);
        vkContainer.remove();
    }
}
/** Compute correct window height from width, preserving canvas aspect ratio + VK */
function computeTerminalHeight(identCode, windowWidth) {
    var headerH = 39;
    var padding = 20;
    var bodyPadding = 0; // float-terminal-body has no padding
    var contentWidth = windowWidth - bodyPadding;
    // Canvas aspect ratio from RetroTerm renderer
    var t = terminals[identCode];
    var canvasRatio = (24 * 16) / (80 * 9); // fallback ~0.533
    if (t && t.term) {
        var renderer = t.term._renderer || t.term.renderer;
        if (renderer) {
            var cw = renderer.charWidth || 9;
            var ch = renderer.charHeight || 16;
            var cols = 80, rows = 24;
            canvasRatio = (rows * ch) / (cols * cw);
        }
    }
    var canvasH = contentWidth * canvasRatio;
    // VK height from SVG viewBox aspect ratio (if visible)
    var vkH = 0;
    if (t && t.vkContainer && t.vkContainer.style.display !== 'none') {
        var svg = t.vkContainer.querySelector('svg.retroterm-vk');
        if (svg) {
            var vb = svg.viewBox.baseVal;
            if (vb && vb.width > 0 && vb.height > 0) {
                vkH = contentWidth * (vb.height / vb.width);
            }
        }
    }
    return Math.ceil(headerH + canvasH + vkH + padding);
}
/** Toggle VK visibility for a terminal */
function toggleTerminalVK(identCode) {
    var t = terminals[identCode];
    if (!t || !t.vkContainer)
        return;
    var showing = t.vkContainer.style.display === 'none';
    t.vkContainer.style.display = showing ? '' : 'none';
    // Floating terminal: recompute height from aspect ratio
    var floatWin = floatingTerminalWindows[identCode];
    if (floatWin) {
        if (!floatWin.classList.contains('maximized')) {
            var newH = computeTerminalHeight(identCode, floatWin.offsetWidth);
            floatWin.style.height = newH + 'px';
            // Persist
            var storageKey = 'float-term-' + identCode + '-size';
            try {
                localStorage.setItem(storageKey, JSON.stringify({
                    width: floatWin.style.width,
                    height: floatWin.style.height
                }));
            }
            catch (e) { }
        }
        setTimeout(function () { t.resizeTerminal(); }, 0);
        return;
    }
    // Console: maximized -> flex layout + ResizeObserver handles canvas refit;
    // normal -> grow/shrink window to fit canvas + VK
    var consoleWin = document.getElementById('terminal-window');
    if (consoleWin && consoleWin.classList.contains('maximized')) {
        // Double-RAF: let flex reflow settle, then refit canvas
        requestAnimationFrame(function () {
            requestAnimationFrame(function () {
                t.resizeTerminal();
            });
        });
    }
    else {
        fitTerminalWindowHeight();
    }
}
// ---- Update console title based on emulator type ----
function updateConsoleTitle() {
    var titleEl = document.getElementById('terminal-window-title');
    if (!titleEl)
        return;
    var emuLabel = window.getEmulatorTypeLabel();
    if (emuLabel) {
        titleEl.textContent = 'Console (' + emuLabel + ') - Push ESC to wake up SINTRAN';
    }
    // Update floating terminal titles too
    Object.keys(floatingTerminalWindows).forEach(function (id) {
        var identCode = parseInt(id);
        var floatTitleEl = document.getElementById('float-term-' + identCode + '-title');
        if (floatTitleEl) {
            var name = terminalDisplayNames[identCode] || identCode.toString();
            floatTitleEl.textContent = 'Terminal ' + name + (emuLabel ? ' (' + emuLabel + ')' : '');
        }
    });
}
function toggleVirtualKeyboard() {
    toggleTerminalVK(activeTerminalId);
}
// ---- Backend switching ----
function switchTerminalBackend(backend) {
    // Switching backend requires a full page reload which destroys all emulator state.
    // If the emulator is running, warn the user.
    if (typeof hasEverStartedEmulation !== 'undefined' && hasEverStartedEmulation) {
        if (!confirm('Switching terminal backend requires a page reload.\nThe emulator will stop and you will need to reboot.')) {
            // Revert the dropdown to current value
            var sel = document.getElementById('config-terminal-backend');
            if (sel)
                sel.value = isRetroTermBackend ? 'retroterm' : 'xterm';
            return;
        }
    }
    localStorage.setItem('nd100x-terminal-backend', backend);
    location.reload();
}
// ---- Initialize dropdowns ----
function initializeDropdowns() {
    var settings = window.getTerminalSettings(activeTerminalId);
    var fontSelect = document.getElementById('font-family-select');
    var colorSelect = document.getElementById('color-theme-select');
    if (fontSelect) {
        if (isRetroTermBackend) {
            // Hide font dropdown when RetroTerm is active (bitmap fonts only)
            fontSelect.style.display = 'none';
        }
        else {
            fontSelect.value = settings.fontFamily;
        }
    }
    if (colorSelect)
        colorSelect.value = settings.colorTheme;
    var floatToggle = document.getElementById('config-float-terminals');
    if (floatToggle) {
        floatToggle.checked = isFloatMode();
    }
    // Backend config dropdown
    var backendSelect = document.getElementById('config-terminal-backend');
    if (backendSelect) {
        backendSelect.value = isRetroTermBackend ? 'retroterm' : 'xterm';
        backendSelect.addEventListener('change', function () {
            switchTerminalBackend(backendSelect.value);
        });
    }
    // Emulator type dropdown (RetroTerm only)
    var emuTypeRow = document.getElementById('config-emulator-type-row');
    var emuTypeSelect = document.getElementById('config-emulator-type');
    if (emuTypeRow && emuTypeSelect) {
        if (isRetroTermBackend) {
            emuTypeRow.style.display = '';
            var savedType = 'tdv2200';
            try {
                savedType = localStorage.getItem('nd100x-emulator-type') || 'tdv2200';
            }
            catch (e) { }
            emuTypeSelect.value = savedType;
            emuTypeSelect.addEventListener('change', function () {
                localStorage.setItem('nd100x-emulator-type', emuTypeSelect.value);
                location.reload();
            });
        }
    }
    // VK toggle button (console window)
    var vkBtn = document.getElementById('term-vk-toggle');
    if (vkBtn) {
        vkBtn.addEventListener('click', function () {
            toggleTerminalVK(activeTerminalId);
        });
    }
}
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeDropdowns);
}
else {
    initializeDropdowns();
}
// Expose globals for other JS modules
window.activeTerminalId = activeTerminalId;
window.terminalDisplayNames = terminalDisplayNames;
window.applySettingsToTerminal = applySettingsToTerminal;
window.toggleVirtualKeyboard = toggleVirtualKeyboard;
window.switchTerminalBackend = switchTerminalBackend;
