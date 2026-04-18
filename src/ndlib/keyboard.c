/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100x project and the RetroCore project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the nd100em
 * distribution in the file COPYING); if not, see <http://www.gnu.org/licenses/>.
 */

#include "keyboard.h"
#include <string.h>

#ifdef _WIN32

// =============================================================================
// Windows implementation.
//
// Uses ReadConsoleInputW() to read native console key events. Classification
// is driven by virtual-key codes and control-key modifiers — no synthetic
// xterm escape sequences are generated. Non-key events (mouse, focus, buffer
// resize) and key-release events are discarded.
// =============================================================================

#include <windows.h>

static HANDLE get_stdin_handle(void)
{
    // Cache the handle on first use. Caching is safe: the console handle
    // associated with STD_INPUT_HANDLE does not change for the lifetime of
    // the process.
    static HANDLE hIn = NULL;
    if (!hIn) {
        hIn = GetStdHandle(STD_INPUT_HANDLE);
    }
    return hIn;
}

KeyEvent read_key_event(void)
{
    KeyEvent evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = KEY_NONE;

    HANDLE hIn = get_stdin_handle();
    if (!hIn || hIn == INVALID_HANDLE_VALUE) return evt;

    // Non-blocking: ask how many input records are pending.
    DWORD pending = 0;
    if (!GetNumberOfConsoleInputEvents(hIn, &pending) || pending == 0) {
        return evt;
    }

    // Drain events one at a time until we find a key-press we classify, or
    // run out of pending events. We intentionally keep unrelated events
    // (mouse, focus, buffer size) out of the stream by consuming them here.
    while (pending > 0) {
        INPUT_RECORD rec;
        DWORD read = 0;
        if (!ReadConsoleInputW(hIn, &rec, 1, &read) || read == 0) {
            return evt;
        }
        pending--;

        if (rec.EventType != KEY_EVENT) continue;
        if (!rec.Event.KeyEvent.bKeyDown) continue;

        const KEY_EVENT_RECORD *ke = &rec.Event.KeyEvent;
        WORD  vk  = ke->wVirtualKeyCode;
        DWORD cks = ke->dwControlKeyState;
        bool  alt = (cks & (LEFT_ALT_PRESSED  | RIGHT_ALT_PRESSED))  != 0;
        bool  ctl = (cks & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;

        // Alt+1..9 — digit pressed with only Alt held.
        if (alt && !ctl && vk >= '1' && vk <= '9') {
            evt.type      = KEY_ALT_DIGIT;
            evt.ch        = (char)vk;
            evt.seq[0]    = (char)vk;
            evt.seqLen    = 1;
            return evt;
        }

        // F12 — dedicated virtual-key code.
        if (vk == VK_F12) {
            evt.type   = KEY_F12;
            evt.seqLen = 0;
            return evt;
        }

        // Bare Escape.
        if (vk == VK_ESCAPE) {
            evt.type   = KEY_ESCAPE;
            evt.seq[0] = 0x1B;
            evt.seqLen = 1;
            return evt;
        }

        // Backspace — ReadConsoleInputW normally sets UnicodeChar to 0x08,
        // but emit it explicitly so any console-mode quirk can't swallow it.
        if (vk == VK_BACK) {
            evt.type   = KEY_CHAR;
            evt.ch     = 0x08;
            evt.seq[0] = 0x08;
            evt.seqLen = 1;
            return evt;
        }

        // Delete — ND-100/SINTRAN uses 0x1F (Unit Separator / "delete") as
        // the rubout character rather than ASCII DEL (0x7F).
        if (vk == VK_DELETE) {
            evt.type   = KEY_CHAR;
            evt.ch     = 0x1F;
            evt.seq[0] = 0x1F;
            evt.seqLen = 1;
            return evt;
        }

        // Ordinary typed character. ReadConsoleInputW has already applied
        // keyboard-layout translation and dead-key composition, so uChar
        // contains the exact character the user typed.
        wchar_t wc = ke->uChar.UnicodeChar;
        if (wc != 0 && wc < 0x80) {
            // ASCII fast path — fits in a single byte.
            evt.type   = KEY_CHAR;
            evt.ch     = (char)wc;
            evt.seq[0] = (char)wc;
            evt.seqLen = 1;
            return evt;
        }
        if (wc != 0) {
            // Non-ASCII: pass through as UTF-8 into seq. This preserves
            // international characters for passthrough to the emulated
            // terminal without forcing a specific codepage here.
            char utf8[8];
            int  n = WideCharToMultiByte(CP_UTF8, 0, &wc, 1,
                                         utf8, (int)sizeof(utf8),
                                         NULL, NULL);
            if (n > 0 && n <= (int)sizeof(evt.seq)) {
                evt.type   = KEY_CHAR;
                evt.ch     = utf8[0];
                memcpy(evt.seq, utf8, (size_t)n);
                evt.seqLen = n;
                return evt;
            }
        }
        // Otherwise (function keys we don't classify, arrow keys, etc.):
        // swallow and keep draining.
    }

    return evt;
}

#else

// =============================================================================
// POSIX implementation.
//
// Non-blocking poll() on stdin. After receiving an ESC we wait up to 50ms
// for trailing bytes so xterm-style multi-byte escape sequences arrive
// intact, then classify the buffer into a KeyEvent.
// =============================================================================

#include <poll.h>
#include <unistd.h>

static int read_raw_sequence(char *buf, size_t bufsize)
{
    if (!buf || bufsize == 0) return 0;

    int keylen = 0;
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };

    while (keylen < (int)bufsize - 1) {
        // First byte: non-blocking.
        // After ESC: wait up to 50ms for the rest of the escape sequence.
        int timeout_ms = (keylen > 0 && buf[0] == 27) ? 50 : 0;
        if (poll(&pfd, 1, timeout_ms) <= 0) break;

        char ch;
        int n = read(STDIN_FILENO, &ch, 1);
        if (n <= 0) break;

        buf[keylen++] = ch;

        if (keylen == 1 && ch != 27) break;        // Single char (not ESC)
        if (keylen == 2 && ch != '[') break;       // ESC + non-[ → complete
        if (keylen >= 3 && (ch == '~' || (ch >= 'A' && ch <= 'Z')))
            break;                                  // End of escape sequence
    }

    buf[keylen] = '\0';
    return keylen;
}

KeyEvent read_key_event(void)
{
    KeyEvent evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = KEY_NONE;

    char buf[8];
    int len = read_raw_sequence(buf, sizeof(buf));
    if (len <= 0) return evt;

    // Copy raw bytes into the event for passthrough / KEY_UNKNOWN consumers.
    int copyLen = (len < (int)sizeof(evt.seq)) ? len : (int)sizeof(evt.seq);
    memcpy(evt.seq, buf, (size_t)copyLen);
    evt.seqLen = copyLen;

    // Classify.
    if (len == 1) {
        if (buf[0] == 27) {
            evt.type = KEY_ESCAPE;
        } else {
            evt.type = KEY_CHAR;
            evt.ch   = buf[0];
        }
        return evt;
    }

    // ESC + digit → Alt+digit (xterm's meta-sends-escape convention).
    if (len == 2 && buf[0] == 27 && buf[1] >= '1' && buf[1] <= '9') {
        evt.type = KEY_ALT_DIGIT;
        evt.ch   = buf[1];
        return evt;
    }

    // F12 — standard "\x1B[24~" and some terminals "\x1B[6~".
    if ((len == 5 && memcmp(buf, "\x1B[24~", 5) == 0) ||
        (len == 4 && memcmp(buf, "\x1B[6~",  4) == 0)) {
        evt.type = KEY_F12;
        return evt;
    }

    // Unknown multi-byte sequence — caller still has the raw bytes in seq.
    evt.type = KEY_UNKNOWN;
    return evt;
}

#endif
