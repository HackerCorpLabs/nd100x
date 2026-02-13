//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//
// terminal-core.ts - Single source of truth for terminal creation, theming,
// settings, keyboard handling, and font scaling.
// Used by terminal-manager.ts (main page) and terminal-popout-app.ts (pop-out).
//
// Exports (on window):
//   terminalColorThemes      - color theme map
//   createScaledTerminal     - factory: Terminal + addons + auto-scaling
//   fitTerminalScaled        - resize an existing terminal with font scaling
//   terminalFontOptions      - canonical font option list
//   terminalColorOptions     - canonical color option list
//   getTerminalSettings      - per-terminal settings accessor
//   saveTerminalSettings     - persist settings to localStorage
//   loadTerminalSettings     - restore settings from localStorage
//   setupTerminalKeyHandler  - keyboard handler factory (RAW_MODE)
//   buildFontSelectHTML      - build <select> HTML for font dropdown
//   buildColorSelectHTML     - build <select> HTML for color dropdown
//   getOpaqueTheme           - return theme with opaque background (for pop-out)
(function () {
    'use strict';
    var TARGET_COLS = 80;
    var TARGET_ROWS = 24;
    var MIN_FONT = 8;
    var MAX_FONT = 120;
    // ---- Color themes (ONE definition, used everywhere) ----
    var colorThemes = {
        green: { background: 'transparent', foreground: '#00FF00', cursor: '#00FF00' },
        amber: { background: 'transparent', foreground: '#FFBF00', cursor: '#FFBF00' },
        white: { background: 'transparent', foreground: '#F8F8F8', cursor: '#F8F8F8' },
        blue: { background: 'transparent', foreground: '#00BFFF', cursor: '#00BFFF' },
        paperwhite: { background: '#f0f0f5', foreground: '#222222', cursor: '#222222' }
    };
    // ---- Canonical font and color option lists ----
    var fontOptions = [
        { val: "monospace", label: "Monospace" },
        { val: "'VT323', monospace", label: "VT323" },
        { val: "'Fira Mono', monospace", label: "Fira Mono" },
        { val: "'IBM Plex Mono', monospace", label: "IBM Plex" },
        { val: "'Cascadia Mono', monospace", label: "Cascadia" },
        { val: "'Source Code Pro', monospace", label: "Source Code" }
    ];
    var colorOptions = [
        { val: "green", label: "Green" },
        { val: "amber", label: "Amber" },
        { val: "white", label: "White" },
        { val: "blue", label: "Blue" },
        { val: "paperwhite", label: "Paper" }
    ];
    // ---- Settings management ----
    var defaultFontFamily = "monospace";
    var defaultColorTheme = "green";
    // Per-terminal settings: { [identCode]: { fontFamily, colorTheme } }
    var terminalSettingsMap = {};
    function getTerminalSettings(identCode) {
        if (!terminalSettingsMap[identCode]) {
            terminalSettingsMap[identCode] = {
                fontFamily: defaultFontFamily,
                colorTheme: defaultColorTheme
            };
        }
        return terminalSettingsMap[identCode];
    }
    function saveTerminalSettings() {
        try {
            localStorage.setItem('terminal-settings', JSON.stringify(terminalSettingsMap));
        }
        catch (e) { /* quota or private browsing */ }
    }
    function loadTerminalSettings() {
        try {
            var saved = localStorage.getItem('terminal-settings');
            if (saved) {
                terminalSettingsMap = JSON.parse(saved);
            }
        }
        catch (e) { /* corrupt data */ }
    }
    // Load on script evaluation
    loadTerminalSettings();
    // ---- DOM measurement helpers ----
    var _measureSpan = null;
    function measureCellWidth(fontFamily, fontSize) {
        if (!_measureSpan) {
            _measureSpan = document.createElement('span');
            _measureSpan.style.position = 'absolute';
            _measureSpan.style.visibility = 'hidden';
            _measureSpan.style.whiteSpace = 'pre';
            _measureSpan.style.lineHeight = 'normal';
            _measureSpan.style.fontVariant = 'none';
            _measureSpan.textContent = 'WWWWWWWWWW';
            document.body.appendChild(_measureSpan);
        }
        _measureSpan.style.fontFamily = fontFamily;
        _measureSpan.style.fontSize = fontSize + 'px';
        return _measureSpan.getBoundingClientRect().width / 10;
    }
    function measureLineHeight(fontFamily, fontSize) {
        if (!_measureSpan)
            measureCellWidth(fontFamily, fontSize);
        _measureSpan.style.fontFamily = fontFamily;
        _measureSpan.style.fontSize = fontSize + 'px';
        return _measureSpan.getBoundingClientRect().height;
    }
    // ---- Terminal factory ----
    /**
     * Create an xterm.js Terminal with FitAddon, CanvasAddon, and auto font-scaling.
     *
     * @param container   DOM element to host the terminal
     * @param opts        options
     * @returns { term, fitAddon, resizeTerminal }
     */
    function createScaledTerminal(container, opts) {
        opts = opts || {};
        var fontFamily = opts.fontFamily || 'monospace';
        var theme = colorThemes[opts.colorTheme] || colorThemes.green;
        var sizeDisplay = opts.sizeDisplay || null;
        var term = new Terminal({
            cursorBlink: true,
            fontSize: 16,
            fontFamily: fontFamily,
            rows: TARGET_ROWS,
            cols: TARGET_COLS,
            theme: theme
        });
        var fitAddon = new FitAddon.FitAddon();
        term.loadAddon(fitAddon);
        // Canvas renderer: precise character positioning (no DOM cell gaps at large fonts)
        if (typeof CanvasAddon !== 'undefined' && CanvasAddon.CanvasAddon) {
            try {
                term.loadAddon(new CanvasAddon.CanvasAddon());
            }
            catch (e) { /* DOM fallback */ }
        }
        term.open(container);
        fitAddon.fit();
        function resizeTerminal() {
            fitTerminalScaled(term, fitAddon, sizeDisplay);
        }
        // Defer initial font scaling so xterm has rendered cell metrics
        setTimeout(resizeTerminal, 50);
        // Observe container or parent for size changes
        var observeTarget = opts.observeResize || null;
        if (observeTarget) {
            var observer = new ResizeObserver(function () { resizeTerminal(); });
            observer.observe(observeTarget);
        }
        return { term: term, fitAddon: fitAddon, resizeTerminal: resizeTerminal };
    }
    // ---- Font scaling ----
    /**
     * Scale terminal font so ~80x24 fills the available space, then fit.
     *
     * Phase 1 - Fast binary search using DOM span measurement.
     * Phase 2 - Verify with xterm's fitAddon.proposeDimensions() and correct
     *           downward if xterm's cell metrics disagree.
     * Phase 3 - After fit, verify the rendered screen doesn't overflow the
     *           container (catches subpixel rounding that clips the last row).
     *
     * CSS zoom handling:
     *   Under CSS zoom, xterm's proposeDimensions() mixes unzoomed container
     *   dimensions (from getComputedStyle) with zoomed cell metrics, producing
     *   inflated col/row counts. Phase 2 verification is skipped when zoom is
     *   detected. fitAddon.fit() still fills the space correctly despite the
     *   coordinate mismatch because the font size (from Phase 1) is correct.
     */
    function fitTerminalScaled(term, fitAddon, sizeDisplay) {
        var el = term.element;
        if (!el || !el.parentElement) {
            fitAddon.fit();
            return;
        }
        // getBoundingClientRect returns actual visible (zoomed) pixels.
        // clientWidth/clientHeight returns CSS pixels (ignores zoom).
        var parentRect = el.parentElement.getBoundingClientRect();
        var containerW = parentRect.width;
        var containerH = parentRect.height;
        if (containerW < 20 || containerH < 20) {
            fitAddon.fit();
            return;
        }
        // Detect CSS zoom: ratio of visible pixels to CSS pixels
        var clientW = el.parentElement.clientWidth;
        var zoomRatio = (clientW > 0) ? containerW / clientW : 1;
        var hasZoom = Math.abs(zoomRatio - 1) > 0.01;
        var fontFamily = term.options.fontFamily || 'monospace';
        // Phase 1: Binary search using DOM span measurement (fast estimate).
        var low = MIN_FONT;
        var high = Math.min(MAX_FONT, Math.floor(containerW / 2));
        var bestSize = MIN_FONT;
        while (low <= high) {
            var mid = Math.floor((low + high) / 2);
            var cellW = measureCellWidth(fontFamily, mid);
            var lineH = measureLineHeight(fontFamily, mid);
            var cols = Math.floor(containerW / cellW);
            var rows = Math.floor(containerH / lineH);
            if (cols >= TARGET_COLS && rows >= TARGET_ROWS) {
                bestSize = mid;
                low = mid + 1;
            }
            else {
                high = mid - 1;
            }
        }
        // Phase 2: Verify with xterm's proposeDimensions (only reliable without zoom).
        if (!hasZoom && typeof fitAddon.proposeDimensions === 'function') {
            term.options.fontSize = bestSize;
            void el.offsetHeight;
            var proposed = fitAddon.proposeDimensions();
            var corrections = 0;
            while (proposed &&
                (proposed.cols < TARGET_COLS || proposed.rows < TARGET_ROWS) &&
                bestSize > MIN_FONT && corrections < 20) {
                bestSize--;
                corrections++;
                term.options.fontSize = bestSize;
                void el.offsetHeight;
                proposed = fitAddon.proposeDimensions();
            }
        }
        // Apply final font size
        term.options.fontSize = bestSize;
        if (typeof term.clearTextureAtlas === 'function') {
            term.clearTextureAtlas();
        }
        void el.offsetHeight;
        fitAddon.fit();
        // Phase 3: Post-fit clipping check.
        var screen = el.querySelector('.xterm-screen');
        if (screen) {
            var screenH = screen.getBoundingClientRect().height;
            if (screenH > containerH + 1 && bestSize > MIN_FONT) {
                bestSize--;
                term.options.fontSize = bestSize;
                if (typeof term.clearTextureAtlas === 'function') {
                    term.clearTextureAtlas();
                }
                void el.offsetHeight;
                fitAddon.fit();
            }
        }
        term.refresh(0, term.rows - 1);
        if (sizeDisplay) {
            sizeDisplay.textContent = bestSize + 'px';
        }
    }
    // ---- Keyboard handler factory ----
    /**
     * Set up RAW_MODE keyboard handling on a terminal.
     * The caller provides a sendCallback to route key codes
     * (main page: emu.sendKey wrapper, pop-out: channel.postMessage).
     * Optional isActiveCheck returns false to suppress input (e.g. wrong terminal focused).
     */
    function setupTerminalKeyHandler(term, sendCallback, isActiveCheck) {
        term.onKey(function (ev) {
            var key = ev.key;
            var domEvent = ev.domEvent;
            if (isActiveCheck && !isActiveCheck())
                return;
            var charCode = key.charCodeAt(0);
            // Ctrl+key: convert to control code
            if (domEvent.ctrlKey && charCode >= 65 && charCode <= 90) {
                var ctrlCode = charCode - 64;
                sendCallback(ctrlCode);
            }
            else if (charCode === 10) {
                sendCallback(13);
            }
            else {
                sendCallback(charCode);
            }
            // Local echo for backspace
            if (charCode === 8 || charCode === 127) {
                term.write('\b \b');
            }
        });
    }
    // ---- Select HTML builders ----
    function buildFontSelectHTML(identCode) {
        var settings = getTerminalSettings(identCode);
        var html = '<select class="term-header-select" title="Font">';
        fontOptions.forEach(function (o) {
            var sel = (o.val === settings.fontFamily) ? ' selected' : '';
            html += '<option value="' + o.val + '"' + sel + '>' + o.label + '</option>';
        });
        html += '</select>';
        return html;
    }
    function buildColorSelectHTML(identCode) {
        var settings = getTerminalSettings(identCode);
        var html = '<select class="term-header-select" title="Color">';
        colorOptions.forEach(function (o) {
            var sel = (o.val === settings.colorTheme) ? ' selected' : '';
            html += '<option value="' + o.val + '"' + sel + '>' + o.label + '</option>';
        });
        html += '</select>';
        return html;
    }
    // ---- Opaque theme helper ----
    /**
     * Return a copy of a color theme with opaque background.
     * Pop-out windows need this since there's no glass parent behind them.
     */
    function getOpaqueTheme(themeName) {
        var theme = colorThemes[themeName] || colorThemes.green;
        var copy = { background: theme.background, foreground: theme.foreground, cursor: theme.cursor };
        if (copy.background === 'transparent') {
            copy.background = '#0a0e1c';
        }
        return copy;
    }
    // ---- Exports ----
    window.terminalColorThemes = colorThemes;
    window.createScaledTerminal = createScaledTerminal;
    window.fitTerminalScaled = fitTerminalScaled;
    window.terminalFontOptions = fontOptions;
    window.terminalColorOptions = colorOptions;
    window.getTerminalSettings = getTerminalSettings;
    window.saveTerminalSettings = saveTerminalSettings;
    window.loadTerminalSettings = loadTerminalSettings;
    window.setupTerminalKeyHandler = setupTerminalKeyHandler;
    window.buildFontSelectHTML = buildFontSelectHTML;
    window.buildColorSelectHTML = buildColorSelectHTML;
    window.getOpaqueTheme = getOpaqueTheme;
    // Expose the settings map so terminal-manager can access it
    window.terminalSettings = terminalSettingsMap;
})();
