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

#include "keyboard.h"
#include "vscreen.h"
#include "screenmenu.h"

// Forward declaration for floppy menu (conditionally available)
#if !defined(PLATFORM_RISCV)
extern int show_floppy_menu(void);
#endif

// =========================================================
// Internal: set mode and draw the appropriate screen
// =========================================================

static void draw_f12(void);
static void draw_screen_select(MenuState *state, void *telnetServer);
static void draw_release_prompt(MenuState *state);

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
    printf("\nPress 1-2 to select, ESC to cancel: ");
    fflush(stdout);
}

static void draw_screen_select(MenuState *state, void *telnetServer)
{
    bool hasTelnet = (telnetServer != NULL);

    printf("\033[2J\033[H");

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (hasTelnet) {
        printf("=== Virtual Screens (telnet port %d) ===\n\n",
               TelnetServer_GetPort((TelnetServer *)telnetServer));
    } else
#endif
    {
        printf("=== Virtual Screen Selector ===\n\n");
    }

    for (int i = 0; i < state->screenCount; i++) {
        const char *status = "";

        if (i == *state->activeScreen) {
            status = " *";
        } else if (!state->screens[i].isInputCapable) {
            status = " (output only)";
        }
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
        else if (hasTelnet) {
            TelnetServer *ts = (TelnetServer *)telnetServer;
            if (TelnetServer_IsDeviceConnected(ts, state->screens[i].device))
                status = " [Telnet]";
            else if (!state->screens[i].localActive)
                status = " [Inactive]";
            else if (state->screens[i].isInputCapable && i > 0)
                status = " [Virtual]";
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
        printf("\n\nPress 1-%d/a to switch, R to release, ESC to cancel: ",
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
}

// =========================================================
// Key handler — processes one keypress per call
// =========================================================

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
void menu_process_key(MenuState *state, const char *keybuf, int keylen, TelnetServer *telnetServer)
#else
void menu_process_key(MenuState *state, const char *keybuf, int keylen, void *telnetServer)
#endif
{
    if (keylen <= 0) return;

    switch (state->mode) {

    // ----- F12 top-level menu -----
    case MENU_F12:
        if (is_escape_key(keybuf)) {
            menu_set_mode(state, MENU_NONE, telnetServer);
        } else if (keybuf[0] == '1') {
#if defined(PLATFORM_RISCV)
            printf("\nFloppy menu not available on RISC-V build\n");
            fflush(stdout);
#else
            int ret = show_floppy_menu();
            if (ret == -1) printf("Failed to show floppy menu\n");
#endif
            menu_set_mode(state, MENU_NONE, telnetServer);
        } else if (keybuf[0] == '2') {
            menu_set_mode(state, MENU_SCREEN_SELECT, telnetServer);
        }
        break;

    // ----- Screen selector -----
    case MENU_SCREEN_SELECT:
        if (is_escape_key(keybuf)) {
            menu_set_mode(state, MENU_NONE, telnetServer);
            return;
        }
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
        if ((keybuf[0] == 'r' || keybuf[0] == 'R') && telnetServer) {
            menu_set_mode(state, MENU_SCREEN_RELEASE, telnetServer);
            return;
        }
#endif
        {
            int choice = -1;
            if (keybuf[0] >= '1' && keybuf[0] <= '9')
                choice = keybuf[0] - '1';
            else if (keybuf[0] >= 'a' && keybuf[0] <= 'z')
                choice = 9 + (keybuf[0] - 'a');

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
        if (is_escape_key(keybuf)) {
            menu_set_mode(state, MENU_SCREEN_SELECT, telnetServer);
            return;
        }
        {
            int choice = -1;
            if (keybuf[0] >= '1' && keybuf[0] <= '9')
                choice = keybuf[0] - '1';
            else if (keybuf[0] >= 'a' && keybuf[0] <= 'z')
                choice = 9 + (keybuf[0] - 'a');

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

    case MENU_MESSAGE:
        break;

    default:
        break;
    }
}
