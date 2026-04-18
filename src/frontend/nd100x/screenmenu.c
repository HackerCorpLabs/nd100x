/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdatomic.h>

#include "keyboard.h"
#include "vscreen.h"
#include "screenmenu.h"
#include "../../devices/devices_types.h"
#include "../../devices/devices_protos.h"
#include "../../devices/hdlc/deviceHDLC.h"

// Forward declaration for floppy menu (conditionally available).
// The floppy-DB browser in menu.c depends on ncurses + libcurl and is not
// compiled on RISC-V or Windows builds, so gate the extern the same way.
#if !defined(PLATFORM_RISCV) && !defined(PLATFORM_WINDOWS) && !defined(_WIN32)
extern int show_floppy_menu(void);
#endif

// =========================================================
// Internal: set mode and draw the appropriate screen
// =========================================================

static void draw_f12(void);
static void draw_screen_select(MenuState *state, void *telnetServer);
static void draw_release_prompt(MenuState *state);
static void draw_hdlc_status(void);
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
static void draw_pending_list(void *telnetServer);
#endif

// Human-readable byte count (e.g. "1.2 KB", "3.4 MB")
static void format_bytes(uint64_t bytes, char *buf, int buflen)
{
    if (bytes < 1024)
        snprintf(buf, buflen, "%" PRIu64 " B", bytes);
    else if (bytes < 1024 * 1024)
        snprintf(buf, buflen, "%.1f KB", (double)bytes / 1024.0);
    else
        snprintf(buf, buflen, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
}

static void menu_set_mode(MenuState *state, MenuMode mode, void *telnetServer)
{
    state->mode = mode;
    switch (mode) {
    case MENU_NONE:
        VScreen_Redraw(&state->screens[*state->activeScreen]);
        break;
    case MENU_F12:
        draw_f12();
        break;
    case MENU_SCREEN_SELECT:
        draw_screen_select(state, telnetServer);
        break;
    case MENU_SCREEN_RELEASE:
        draw_release_prompt(state);
        break;
    case MENU_HDLC_STATUS:
        state->lastRefresh = time(NULL);
        draw_hdlc_status();
        break;
    case MENU_PENDING_LIST:
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
        state->lastRefresh = time(NULL);
        draw_pending_list(telnetServer);
#endif
        break;
    case MENU_MESSAGE:
        break;
    default:
        break;
    }
}

static void menu_show_message(MenuState *state, const char *msg, MenuMode returnTo)
{
    printf("\n%s\n", msg);
    fflush(stdout);
    state->mode = MENU_MESSAGE;
    state->returnTo = returnTo;
    state->messageExpiry = time(NULL) + 1;
}

// =========================================================
// Draw functions
// =========================================================

static void draw_f12(void)
{
    printf("\033[2J\033[H");
    printf("=== ND100X Menu ===\n\n");
    printf("  [1] Floppy Database Browser\n");
    printf("  [2] Virtual Screen Selector\n");
    printf("  [3] HDLC Status\n");
    printf("\nPress 1-3 to select, ESC to cancel: ");
    fflush(stdout);
}

static void draw_hdlc_status(void)
{
    // Home cursor and clear entire screen (repaint in place)
    printf("\033[H\033[J");
    printf("=== HDLC Device Status ===\n\n");

    int count = DeviceManager_GetDeviceCount();
    int found = 0;

    printf("  %-6s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %s\n",
           "Dev#", "I/O Addr", "Bytes TX", "Bytes RX",
           "Frames TX", "Frames RX", "CRC Errs", "RX Frame", "RX Queue", "Connected");
    printf("  %-6s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s  %s\n",
           "----", "--------", "--------", "--------",
           "---------", "---------", "--------", "--------", "--------", "---------");

    for (int i = 0; i < count; i++) {
        Device *dev = DeviceManager_GetDeviceByIndex(i);
        if (!dev || dev->type != DEVICE_TYPE_HDLC) continue;

        HDLCData *data = (HDLCData *)dev->deviceData;
        if (!data) continue;

        found++;

        char txBytesStr[16], rxBytesStr[16];
        format_bytes(data->modem ? data->modem->bytesTx : 0, txBytesStr, sizeof(txBytesStr));
        format_bytes(data->modem ? data->modem->bytesRx : 0, rxBytesStr, sizeof(rxBytesStr));

        bool connected = data->modem ? atomic_load(&data->modem->connected) : false;

        // RX frame assembly state and queue depth
        char rxFrameBuf[24];
        char rxQueueBuf[16];
        const char *rxFrame = "Idle";
        const char *rxQueue = "-";

        HDLCRxFrameStatus rxStatus;
        if (HDLC_GetRxFrameStatus(data, &rxStatus)) {
            if (!rxStatus.rxDmaEnabled) {
                rxFrame = "DMA off";
            } else if (!rxStatus.rxDcbReady) {
                rxFrame = "No DCB";
            } else {
                switch (rxStatus.state) {
                case 1: // HDLC_STATE_RECEIVING
                case 2: // HDLC_STATE_ESCAPE
                    snprintf(rxFrameBuf, sizeof(rxFrameBuf), "Rx %d B", rxStatus.frameLength);
                    rxFrame = rxFrameBuf;
                    break;
                case 3: // HDLC_STATE_ERROR
                    rxFrame = "Error";
                    break;
                }
            }
            format_bytes(rxStatus.tcpQueueUsed, rxQueueBuf, sizeof(rxQueueBuf));
            rxQueue = rxQueueBuf;
        }

        char addrStr[16];
        snprintf(addrStr, sizeof(addrStr), "%04o-%04o",
                 dev->startAddress, dev->endAddress);

        printf("  %-6d  %-10s  %-10s  %-10s  %-10" PRIu64 "  %-10" PRIu64 "  %-10" PRIu64 "  %-10s  %-10s  %s\n",
               data->thumbwheel,
               addrStr,
               txBytesStr, rxBytesStr,
               data->framesTx, data->framesRx, data->framesRxErrors,
               rxFrame, rxQueue,
               connected ? "Yes" : "No");
    }

    if (found == 0) {
        printf("\n  No HDLC devices configured.\n");
        printf("  Use --hdlc=N:PORT (server) or --hdlc=N:HOST:PORT (client)\n");
    }

    printf("\n  Refreshes every second. Press ESC to return.\n");
    fflush(stdout);
}

static void draw_screen_select(MenuState *state, void *telnetServer)
{
    bool hasTelnet = (telnetServer != NULL);

    printf("\033[2J\033[H");

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (hasTelnet) {
        TelnetServer *ts = (TelnetServer *)telnetServer;
        int pending = TelnetServer_GetPendingCount(ts);
        if (pending > 0) {
            printf("=== Virtual Screens (telnet port %d, %d pending) ===\n\n",
                   TelnetServer_GetPort(ts), pending);
        } else {
            printf("=== Virtual Screens (telnet port %d) ===\n\n",
                   TelnetServer_GetPort(ts));
        }
    } else
#endif
    {
        printf("=== Virtual Screen Selector ===\n\n");
    }

    for (int i = 0; i < state->screenCount; i++) {
        char statusBuf[128];
        const char *status = "";

        if (i == *state->activeScreen) {
            status = " *";
        } else if (!state->screens[i].isInputCapable) {
            status = " (output only)";
        }
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
        else if (hasTelnet) {
            TelnetServer *ts = (TelnetServer *)telnetServer;
            if (TelnetServer_IsDeviceConnected(ts, state->screens[i].device)) {
                const char *addr = TelnetServer_GetDeviceClientAddr(ts, state->screens[i].device);

                // Find terminal index in server for byte stats
                uint64_t rx = 0, tx = 0;
                int tcount = TelnetServer_GetTerminalCount(ts);
                for (int t = 0; t < tcount; t++) {
                    const char *tname = NULL;
                    bool conn = false;
                    TelnetServer_GetTerminalStatus(ts, t, &tname, NULL, &conn, NULL, NULL, 0);
                    if (conn && tname && strcmp(tname, state->screens[i].name) == 0) {
                        TelnetServer_GetTerminalStats(ts, t, &rx, &tx);
                        break;
                    }
                }

                char rxStr[16], txStr[16];
                format_bytes(rx, rxStr, sizeof(rxStr));
                format_bytes(tx, txStr, sizeof(txStr));

                if (addr && addr[0]) {
                    snprintf(statusBuf, sizeof(statusBuf),
                             " [Telnet %s] rx:%s tx:%s", addr, rxStr, txStr);
                } else {
                    snprintf(statusBuf, sizeof(statusBuf),
                             " [Telnet] rx:%s tx:%s", rxStr, txStr);
                }
                status = statusBuf;
            } else if (!state->screens[i].localActive) {
                status = " [Inactive]";
            } else if (state->screens[i].isInputCapable && i > 0) {
                status = " [Virtual]";
            }
        }
#else
        else if (!state->screens[i].localActive) {
            status = " [Inactive]";
        }
        (void)telnetServer;
#endif

        if (i < 9)
            printf("  [%d] %-24s%s\n", i + 1, state->screens[i].name, status);
        else
            printf("  [%c] %-24s%s\n", 'a' + (i - 9), state->screens[i].name, status);
    }

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (hasTelnet) {
        printf("\n  [R] Release terminal (virtual->inactive, or disconnect telnet)");
        printf("\n  [P] Pending connections (live view)");
        printf("\n\nPress 1-%d/a to switch, R/P for options, ESC to cancel: ",
               state->screenCount > 9 ? 9 : state->screenCount);
    } else
#endif
    {
        printf("\nPress 1-%d/a to switch, ESC to cancel: ",
               state->screenCount > 9 ? 9 : state->screenCount);
    }
    fflush(stdout);
}

static void draw_release_prompt(MenuState *state)
{
    printf("\nEnter terminal to release (2-%d): ",
           state->screenCount > 9 ? 9 : state->screenCount);
    fflush(stdout);
}

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
static void draw_pending_list(void *telnetServer)
{
    TelnetServer *ts = (TelnetServer *)telnetServer;
    int count = TelnetServer_GetPendingCount(ts);

    printf("\033[2J\033[H");
    printf("=== Pending Telnet Connections (live, port %d) ===\n\n",
           TelnetServer_GetPort(ts));

    if (count == 0) {
        printf("  No pending connections.\n");
    } else {
        printf("  #  %-24s  %-10s  %-10s  %s\n", "Address", "RX", "TX", "Age");
        printf("  -  %-24s  %-10s  %-10s  %s\n", "-------", "--", "--", "---");
        for (int i = 0; i < count; i++) {
            char addr[48];
            int age = 0;
            uint64_t rx = 0, tx = 0;
            if (TelnetServer_GetPendingInfo(ts, i, addr, sizeof(addr), &age, &rx, &tx)) {
                char rxStr[16], txStr[16];
                format_bytes(rx, rxStr, sizeof(rxStr));
                format_bytes(tx, txStr, sizeof(txStr));
                printf("  %d) %-24s  %-10s  %-10s  %ds / 60s\n",
                       i + 1, addr, rxStr, txStr, age);
            }
        }
    }

    printf("\n  [D] Drop connection  [A] Drop all  [ESC] Back\n");
    printf("\nRefreshes every 2 seconds. Press key: ");
    fflush(stdout);
}
#endif

// =========================================================
// Public API
// =========================================================

void menu_init(MenuState *state, VScreen *screens, int screenCount, int *activeScreen)
{
    memset(state, 0, sizeof(MenuState));
    state->screens = screens;
    state->screenCount = screenCount;
    state->activeScreen = activeScreen;
}

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
void menu_enter(MenuState *state, TelnetServer *telnetServer)
#else
void menu_enter(MenuState *state, void *telnetServer)
#endif
{
    menu_set_mode(state, MENU_F12, telnetServer);
}

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
void menu_tick(MenuState *state, TelnetServer *telnetServer)
#else
void menu_tick(MenuState *state, void *telnetServer)
#endif
{
    if (state->mode == MENU_MESSAGE && time(NULL) >= state->messageExpiry) {
        menu_set_mode(state, state->returnTo, telnetServer);
    }
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    // Live refresh for pending list view (every 2 seconds)
    if (state->mode == MENU_PENDING_LIST && telnetServer) {
        time_t now = time(NULL);
        if (now - state->lastRefresh >= 2) {
            state->lastRefresh = now;
            draw_pending_list(telnetServer);
        }
    }
#endif
    // Live refresh for HDLC status view (every 1 second)
    if (state->mode == MENU_HDLC_STATUS) {
        time_t now = time(NULL);
        if (now - state->lastRefresh >= 1) {
            state->lastRefresh = now;
            draw_hdlc_status();
        }
    }
}

// =========================================================
// Key handler - processes one keypress per call
// =========================================================

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
void menu_process_key(MenuState *state, const KeyEvent *key, TelnetServer *telnetServer)
#else
void menu_process_key(MenuState *state, const KeyEvent *key, void *telnetServer)
#endif
{
    if (!key || key->type == KEY_NONE) return;

    // The menu only cares about ESC or typed characters. Function keys and
    // multi-byte unknown sequences are ignored.
    bool is_esc = (key->type == KEY_ESCAPE);
    char ch     = (key->type == KEY_CHAR) ? key->ch : '\0';

    switch (state->mode) {

    // ----- F12 top-level menu -----
    case MENU_F12:
        if (is_esc) {
            menu_set_mode(state, MENU_NONE, telnetServer);
        } else if (ch == '1') {
#if defined(PLATFORM_RISCV) || defined(PLATFORM_WINDOWS) || defined(_WIN32)
            printf("\nFloppy menu not available on this build\n");
            fflush(stdout);
#else
            int ret = show_floppy_menu();
            if (ret == -1) printf("Failed to show floppy menu\n");
#endif
            menu_set_mode(state, MENU_NONE, telnetServer);
        } else if (ch == '2') {
            menu_set_mode(state, MENU_SCREEN_SELECT, telnetServer);
        } else if (ch == '3') {
            menu_set_mode(state, MENU_HDLC_STATUS, telnetServer);
        }
        break;

    // ----- Screen selector -----
    case MENU_SCREEN_SELECT:
        if (is_esc) {
            menu_set_mode(state, MENU_NONE, telnetServer);
            return;
        }
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
        if ((ch == 'r' || ch == 'R') && telnetServer) {
            menu_set_mode(state, MENU_SCREEN_RELEASE, telnetServer);
            return;
        }
        if ((ch == 'p' || ch == 'P') && telnetServer) {
            menu_set_mode(state, MENU_PENDING_LIST, telnetServer);
            return;
        }
#endif
        {
            int choice = -1;
            if (ch >= '1' && ch <= '9')
                choice = ch - '1';
            else if (ch >= 'a' && ch <= 'z')
                choice = 9 + (ch - 'a');

            if (choice >= 0 && choice < state->screenCount) {
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
                if (telnetServer && TelnetServer_IsDeviceConnected(
                        telnetServer, state->screens[choice].device)) {
                    menu_show_message(state, "Terminal is in use by telnet client.",
                                      MENU_SCREEN_SELECT);
                    return;
                }

                // If not locally active, re-activate it and clear carrier
                if (telnetServer && state->screens[choice].isInputCapable &&
                    !state->screens[choice].localActive) {
                    state->screens[choice].localActive = true;
                    TelnetServer_SetDeviceLocallyActive(telnetServer,
                        state->screens[choice].device, true);
                    TelnetServer_ClearDeviceCarrier(telnetServer,
                        state->screens[choice].device);
                }
#endif
                *state->activeScreen = choice;
            }
            menu_set_mode(state, MENU_NONE, telnetServer);
        }
        break;

    // ----- Release prompt (unified close/disconnect) -----
    case MENU_SCREEN_RELEASE:
        if (is_esc) {
            menu_set_mode(state, MENU_SCREEN_SELECT, telnetServer);
            return;
        }
        {
            int choice = -1;
            if (ch >= '1' && ch <= '9')
                choice = ch - '1';
            else if (ch >= 'a' && ch <= 'z')
                choice = 9 + (ch - 'a');

            if (choice < 0 || choice >= state->screenCount) {
                menu_show_message(state, "Invalid selection.",
                                  MENU_SCREEN_SELECT);
                return;
            }
            if (choice == 0) {
                menu_show_message(state, "Cannot release the Console.",
                                  MENU_SCREEN_SELECT);
                return;
            }
            if (!state->screens[choice].isInputCapable) {
                menu_show_message(state, "Cannot release output-only screens.",
                                  MENU_SCREEN_SELECT);
                return;
            }

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
            if (telnetServer) {
                // If telnet-connected: disconnect the client
                if (TelnetServer_IsDeviceConnected(telnetServer, state->screens[choice].device)) {
                    TelnetServer_DisconnectDevice(telnetServer, state->screens[choice].device);
                    char msg[64];
                    snprintf(msg, sizeof(msg), "%s telnet client disconnected.",
                             state->screens[choice].name);
                    menu_show_message(state, msg, MENU_SCREEN_SELECT);
                    return;
                }

                // If locally active: release for telnet
                if (state->screens[choice].localActive) {
                    if (choice == *state->activeScreen) {
                        menu_show_message(state, "Cannot release the active screen. Switch first.",
                                          MENU_SCREEN_SELECT);
                        return;
                    }
                    state->screens[choice].localActive = false;
                    TelnetServer_SetDeviceLocallyActive(telnetServer,
                        state->screens[choice].device, false);
                    char msg[64];
                    snprintf(msg, sizeof(msg), "%s released for telnet.",
                             state->screens[choice].name);
                    menu_show_message(state, msg, MENU_SCREEN_SELECT);
                    return;
                }

                // Already inactive
                menu_show_message(state, "Terminal is already inactive.",
                                  MENU_SCREEN_SELECT);
            }
#endif
        }
        break;

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    // ----- Pending connections (live view) -----
    case MENU_PENDING_LIST:
        if (is_esc) {
            menu_set_mode(state, MENU_SCREEN_SELECT, telnetServer);
            return;
        }
        if (telnetServer) {
            if (ch == 'd' || ch == 'D') {
                int count = TelnetServer_GetPendingCount(telnetServer);
                if (count == 0) {
                    menu_show_message(state, "No pending connections to drop.",
                                      MENU_PENDING_LIST);
                } else if (count == 1) {
                    TelnetServer_DropPending(telnetServer, 0);
                    menu_show_message(state, "Dropped pending connection.",
                                      MENU_PENDING_LIST);
                } else {
                    printf("\nDrop which connection (1-%d)? ", count);
                    fflush(stdout);
                    // We'll handle the digit on the next keypress via a simple approach:
                    // For now, just prompt. The next digit key will be caught here.
                }
            } else if (ch >= '1' && ch <= '9') {
                int idx = ch - '1';
                if (TelnetServer_DropPending(telnetServer, idx)) {
                    draw_pending_list(telnetServer);
                    state->lastRefresh = time(NULL);
                }
            } else if (ch == 'a' || ch == 'A') {
                TelnetServer_DropAllPending(telnetServer);
                menu_show_message(state, "All pending connections dropped.",
                                  MENU_PENDING_LIST);
            }
        }
        break;
#endif

    // ----- HDLC status (live view) -----
    case MENU_HDLC_STATUS:
        if (is_esc) {
            menu_set_mode(state, MENU_F12, telnetServer);
        }
        break;

    case MENU_MESSAGE:
        break;

    default:
        break;
    }
}
