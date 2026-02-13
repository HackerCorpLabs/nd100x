//
// SPDX-License-Identifier: MIT
// Copyright (c) 1985-2026 Ronny Hansen
// HackerCorp Labs -- https://github.com/HackerCorpLabs
// Emulating yesterday's technology with today's code
//
// terminal-bridge.ts - Main-tab side bridge for pop-out terminals.
// Replaces terminal-popout-bridge.js.
// Manages BroadcastChannel communication between main tab and pop-out terminal windows.
(function () {
    'use strict';
    // State: identCode -> { channel, popoutWindow, outputBuffer }
    var poppedOutTerminals = {};
    // Maximum buffer size before forced flush
    var MAX_BUFFER_SIZE = 4096;
    // Poll interval for detecting closed pop-outs
    var CLOSE_POLL_INTERVAL = 2000;
    // Close poll timer
    var closePollTimer = null;
    // MessageChannel for background-tab-immune flush scheduling.
    // setTimeout gets throttled to ~1s in background tabs - MessageChannel does NOT.
    var flushChannel = new MessageChannel();
    var flushScheduled = false;
    flushChannel.port1.onmessage = function () {
        flushScheduled = false;
        Object.keys(poppedOutTerminals).forEach(function (id) {
            var state = poppedOutTerminals[parseInt(id)];
            if (state && state.outputBuffer.length > 0) {
                flushOutput(parseInt(id));
            }
        });
    };
    function scheduleFlush() {
        if (flushScheduled)
            return;
        flushScheduled = true;
        flushChannel.port2.postMessage(null);
    }
    /**
     * Pop out a terminal into its own browser window
     */
    function popOutTerminal(identCode) {
        // Already popped out?
        if (poppedOutTerminals[identCode]) {
            var existing = poppedOutTerminals[identCode].popoutWindow;
            if (existing && !existing.closed) {
                existing.focus();
                return;
            }
            popInTerminal(identCode);
        }
        // Build display name
        var displayName = identCode.toString();
        if (typeof terminalDisplayNames !== 'undefined' && terminalDisplayNames[identCode]) {
            displayName = terminalDisplayNames[identCode];
        }
        // Calculate window size
        var width = 820;
        var height = 520;
        var left = window.screenX + 50;
        var top = window.screenY + 50;
        // Open pop-out window
        var windowName = 'nd100x-term-' + identCode;
        var url = 'terminal-popout.html?id=' + identCode + '&name=' + encodeURIComponent(displayName);
        var features = 'width=' + width + ',height=' + height + ',left=' + left + ',top=' + top +
            ',menubar=no,toolbar=no,location=no,status=no,resizable=yes,scrollbars=no';
        var popoutWindow = window.open(url, windowName, features);
        if (!popoutWindow) {
            alert('Pop-out was blocked by your browser. Please allow popups for this site and try again.');
            return;
        }
        // Create BroadcastChannel
        var channelName = 'nd100x-term-' + identCode;
        var channel = new BroadcastChannel(channelName);
        // Store state
        var state = {
            channel: channel,
            popoutWindow: popoutWindow,
            outputBuffer: [],
            flushTimer: null
        };
        poppedOutTerminals[identCode] = state;
        // Handle messages from pop-out
        channel.onmessage = function (ev) {
            var msg = ev.data;
            if (!msg || !msg.type)
                return;
            switch (msg.type) {
                case 'key':
                    if (typeof emu !== 'undefined' && emu.sendKey) {
                        emu.sendKey(identCode, msg.keyCode);
                    }
                    break;
                case 'popout-ready':
                    var settings = null;
                    if (typeof window.getTerminalSettings === 'function') {
                        settings = window.getTerminalSettings(identCode);
                    }
                    if (settings) {
                        channel.postMessage({
                            type: 'settings-changed',
                            fontFamily: settings.fontFamily,
                            colorTheme: settings.colorTheme
                        });
                    }
                    break;
                case 'settings-changed-from-popout':
                    if (typeof window.getTerminalSettings === 'function') {
                        var s = window.getTerminalSettings(identCode);
                        if (msg.fontFamily)
                            s.fontFamily = msg.fontFamily;
                        if (msg.colorTheme)
                            s.colorTheme = msg.colorTheme;
                    }
                    // Update the floating window's header selects
                    var winId = 'float-term-' + identCode;
                    var win = document.getElementById(winId);
                    if (win) {
                        var selects = win.querySelectorAll('.term-header-select');
                        if (selects[0] && msg.fontFamily)
                            selects[0].value = msg.fontFamily;
                        if (selects[1] && msg.colorTheme)
                            selects[1].value = msg.colorTheme;
                    }
                    break;
                case 'popout-closed':
                    popInTerminal(identCode);
                    break;
            }
        };
        // Hide the floating window in main tab
        hideLocalTerminal(identCode);
        // Start close polling
        startClosePoll();
        // Update terminal submenu
        if (typeof updateTerminalSubmenu === 'function') {
            updateTerminalSubmenu();
        }
    }
    /**
     * Restore a terminal from pop-out back to the main tab
     */
    function popInTerminal(identCode) {
        var state = poppedOutTerminals[identCode];
        if (!state)
            return;
        if (state.channel) {
            state.channel.close();
        }
        if (state.popoutWindow && !state.popoutWindow.closed) {
            state.popoutWindow.close();
        }
        delete poppedOutTerminals[identCode];
        restoreLocalTerminal(identCode);
        if (Object.keys(poppedOutTerminals).length === 0) {
            stopClosePoll();
        }
        if (typeof updateTerminalSubmenu === 'function') {
            updateTerminalSubmenu();
        }
    }
    /**
     * Check if a terminal is currently popped out
     */
    function isPoppedOut(identCode) {
        return !!poppedOutTerminals[identCode];
    }
    /**
     * Buffer output for a popped-out terminal.
     * Uses MessageChannel scheduling (immune to background-tab throttling).
     */
    function bufferPopoutOutput(identCode, charCode) {
        var state = poppedOutTerminals[identCode];
        if (!state)
            return;
        state.outputBuffer.push(charCode);
        if (state.outputBuffer.length >= MAX_BUFFER_SIZE) {
            flushOutput(identCode);
            return;
        }
        scheduleFlush();
    }
    /**
     * Flush buffered output to the pop-out window
     */
    function flushOutput(identCode) {
        var state = poppedOutTerminals[identCode];
        if (!state || state.outputBuffer.length === 0)
            return;
        var chars = state.outputBuffer;
        state.outputBuffer = [];
        state.channel.postMessage({
            type: 'output',
            chars: chars
        });
    }
    /**
     * Broadcast theme change to all pop-out windows
     */
    function broadcastThemeChange(themeName) {
        Object.keys(poppedOutTerminals).forEach(function (id) {
            var state = poppedOutTerminals[parseInt(id)];
            if (state && state.channel) {
                state.channel.postMessage({
                    type: 'theme-changed',
                    theme: themeName
                });
            }
        });
    }
    /**
     * Broadcast settings change to a specific pop-out terminal
     */
    function broadcastSettingsChange(identCode) {
        var state = poppedOutTerminals[identCode];
        if (!state || !state.channel)
            return;
        var settings = null;
        if (typeof window.getTerminalSettings === 'function') {
            settings = window.getTerminalSettings(identCode);
        }
        if (settings) {
            state.channel.postMessage({
                type: 'settings-changed',
                fontFamily: settings.fontFamily,
                colorTheme: settings.colorTheme
            });
        }
    }
    /**
     * Hide the local terminal when popped out.
     */
    function hideLocalTerminal(identCode) {
        var winId = 'float-term-' + identCode;
        var win = document.getElementById(winId);
        if (win) {
            if (typeof closeWindow === 'function') {
                closeWindow(winId);
            }
            else {
                win.style.display = 'none';
            }
            return;
        }
        // Tab-based case
        var container = document.getElementById('terminal-container-' + identCode);
        if (!container)
            return;
        var xterm = container.querySelector('.xterm');
        if (xterm)
            xterm.style.display = 'none';
        var indicator = document.createElement('div');
        indicator.className = 'popout-placeholder';
        indicator.id = 'popout-placeholder-' + identCode;
        indicator.innerHTML =
            '<svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">' +
                '<polyline points="15,3 21,3 21,9"/>' +
                '<line x1="21" y1="3" x2="14" y2="10"/>' +
                '<path d="M21 14v5a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5"/>' +
                '</svg>' +
                '<div class="popout-placeholder-text">Popped out</div>';
        container.appendChild(indicator);
    }
    /**
     * Restore the local terminal when popped back in.
     */
    function restoreLocalTerminal(identCode) {
        var placeholder = document.getElementById('popout-placeholder-' + identCode);
        if (placeholder)
            placeholder.remove();
        var winId = 'float-term-' + identCode;
        var win = document.getElementById(winId);
        if (win) {
            if (typeof openWindow === 'function') {
                openWindow(winId);
            }
            else {
                win.style.display = 'flex';
            }
            if (typeof emu !== 'undefined' && emu.setTerminalCarrier) {
                emu.setTerminalCarrier(0, identCode);
            }
        }
        var container = document.getElementById('terminal-container-' + identCode);
        if (container) {
            var xterm = container.querySelector('.xterm');
            if (xterm)
                xterm.style.display = '';
        }
        if (typeof terminals !== 'undefined' && terminals[identCode]) {
            if (typeof window.applySettingsToTerminal === 'function') {
                window.applySettingsToTerminal(identCode);
            }
            terminals[identCode].resizeTerminal();
            terminals[identCode].term.focus();
        }
        if (typeof windowManager !== 'undefined' && windowManager.updateTaskbar) {
            windowManager.updateTaskbar();
        }
    }
    /**
     * Poll for closed pop-out windows (fallback for missed beforeunload)
     */
    function startClosePoll() {
        if (closePollTimer)
            return;
        closePollTimer = window.setInterval(function () {
            Object.keys(poppedOutTerminals).forEach(function (id) {
                var state = poppedOutTerminals[parseInt(id)];
                if (state && state.popoutWindow && state.popoutWindow.closed) {
                    popInTerminal(parseInt(id));
                }
            });
        }, CLOSE_POLL_INTERVAL);
    }
    function stopClosePoll() {
        if (closePollTimer) {
            clearInterval(closePollTimer);
            closePollTimer = null;
        }
    }
    /**
     * Send main-closing to all pop-outs when main tab unloads
     */
    window.addEventListener('beforeunload', function () {
        Object.keys(poppedOutTerminals).forEach(function (id) {
            var state = poppedOutTerminals[parseInt(id)];
            if (state && state.channel) {
                state.channel.postMessage({ type: 'main-closing' });
            }
        });
    });
    // Expose public API on window
    window.popOutTerminal = popOutTerminal;
    window.popInTerminal = popInTerminal;
    window.isPoppedOut = isPoppedOut;
    window.bufferPopoutOutput = bufferPopoutOutput;
    window.broadcastThemeChange = broadcastThemeChange;
    window.broadcastSettingsChange = broadcastSettingsChange;
})();
