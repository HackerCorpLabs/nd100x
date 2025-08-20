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
#include <poll.h>
#include <unistd.h>
#include <string.h>

// Read a complete key sequence (non-blocking)
int read_key_sequence(char *buf, size_t bufsize) {
    if (!buf || bufsize == 0) return 0;
    
    int keylen = 0;
    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
    
    while (keylen < (int)bufsize - 1) {
        if (poll(&pfd, 1, 0) <= 0) break;  // No more data available
        
        char ch;
        int n = read(STDIN_FILENO, &ch, 1);
        if (n <= 0) break;
        
        buf[keylen++] = ch;
        
        // Determine if we have a complete sequence
        if (keylen == 1 && ch != 27) break;        // Single character (not ESC)
        if (keylen == 2 && ch != '[') break;       // ESC followed by non-[ = ESC key
        if (keylen >= 3 && (ch == '~' || (ch >= 'A' && ch <= 'Z')))
            break;                                  // Likely end of escape sequence
    }
    
    buf[keylen] = '\0';
    return keylen;
}

// Check if the key sequence is F12
int is_f12_key(const char *keybuf) {
    return (keybuf && strcmp(keybuf, "\x1B[24~") == 0);
}

// Check if the key sequence is a single ESC key
int is_escape_key(const char *keybuf) {
    return (keybuf && strlen(keybuf) == 1 && keybuf[0] == 27);
} 