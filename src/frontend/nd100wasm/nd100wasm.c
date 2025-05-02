/*
 * nd100wasm - ND100 Virtual Machine for WebAssembly
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100x project.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Add Emscripten specific headers when building for WASM
#ifdef EMSCRIPTEN
#include <emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EMSCRIPTEN_EXPORT
#endif

// TODO: Create proper nd100wasm_types.h or share types with nd100x
// #include "nd100x_types.h" 
#include "nd100wasm_protos.h"

// Include Machine and CPU support
#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"
#include "../machine/machine_types.h"
#include "../machine/machine_protos.h"
#include "../devices/terminal/deviceTerminal.h"
#include "../devices/devices_protos.h"

// Global variables
static int initialized = 0;
static int running = 0;

// Array of device references for quick access
#define MAX_TERMINALS 16
static Device* terminals[MAX_TERMINALS] = {NULL};

// Define a function pointer type for terminal output callbacks
typedef void (*TerminalOutputCallback)(int terminalId, char c);

// Array of callbacks for terminal output
static TerminalOutputCallback terminalOutputCallbacks[MAX_TERMINALS] = {NULL};

// Character device output handler for terminals
static void WasmTerminalOutputHandler(Device *device, char c) 
{
    if (!device)
        return;
        
    // Find terminal ID in our array by device reference
    int terminalId = -1;
    for (int i = 0; i < MAX_TERMINALS; i++) {
        if (terminals[i] == device) {
            terminalId = i;
            break;
        }
    }

    // If we found the terminal and have a callback, use it
    if (terminalId >= 0 && terminalId < MAX_TERMINALS && terminalOutputCallbacks[terminalId]) {
        terminalOutputCallbacks[terminalId](terminalId, c);
    }
    // No fallback printf - we only want output through the callback
}


// Initialize the system
EMSCRIPTEN_EXPORT void Init()
{
    printf("ND100 WebAssembly Emulator Initializing...\n");
    
    // Only initialize once
    if (initialized) {
        printf("Already initialized.\n");
        return;
    }
    
    // Initialize machine components
    machine_init(0); // No debugger in WASM version
    
    // Initialize terminal references - we'll start with console (index 0)
    terminals[0] = DeviceManager_GetDeviceByAddress(0300); // Console
    if (!terminals[0]) {
        printf("Warning: Console terminal device not found!\n");
    } else {
        printf("Console terminal device found at address %04o\n", terminals[0]->startAddress);
        
        // Set up character device output handler for the console terminal
        Device_SetCharacterOutput(terminals[0], WasmTerminalOutputHandler);
    }
    

    
    
    // Load boot program (hardcoded for now)
    program_load(BOOT_FLOPPY, "/FLOPPY.IMG", 1);
    gPC = STARTADDR; // Default start address
    
    initialized = 1;
    printf("ND100 WebAssembly Emulator Initialized.\n");
}

// Send a key to a specific terminal device
EMSCRIPTEN_EXPORT int SendKeyToTerminal(int terminalId, int keyCode)
{
    // Validate terminalId is in range and device exists
    if (terminalId < 0 || terminalId >= MAX_TERMINALS || !terminals[terminalId]) {
        printf("Error: Invalid terminal ID %d\n", terminalId);
        return 0; // Failure
    }
    
    // If device is a character device, use the character input function
    if (terminals[terminalId]->deviceClass == DEVICE_CLASS_CHARACTER && 
        terminals[terminalId]->charCallbacks.inputFunc) {
        Device_InputCharacter(terminals[terminalId], (char)keyCode);
    } 

    return 1; // Success
}

// Get the address of a terminal by ID (for debugging)
EMSCRIPTEN_EXPORT int GetTerminalAddress(int terminalId)
{
    if (terminalId < 0 || terminalId >= MAX_TERMINALS || !terminals[terminalId]) {
        return -1; // Invalid or not found
    }
    
    return terminals[terminalId]->startAddress;
}

// Setup with configuration
EMSCRIPTEN_EXPORT void Setup(const char* config)
{
    printf("Setup called with config: %s\n", config);
    // Parse configuration string and apply settings
    // For now, just print the config
}

// Execute a specific number of steps
EMSCRIPTEN_EXPORT void Step(int steps)
{
    if (!initialized) {
        printf("Error: System not initialized. Call Init() first.\n");
        return;
    }        
    machine_run(steps);
}

// Start the emulation
EMSCRIPTEN_EXPORT void Start()
{
    if (!initialized) {
        Init();
    }
    
    running = 1;
    printf("ND100 Emulation started\n");
}

// Stop the emulation
EMSCRIPTEN_EXPORT void Stop()
{
    running = 0;
    printf("ND100 Emulation stopped\n");
    //machine_stop();
}

// Set a callback for terminal output
EMSCRIPTEN_EXPORT void SetTerminalOutputCallback(int terminalId, void (*callback)(int terminalId, char c))
{
    if (terminalId < 0 || terminalId >= MAX_TERMINALS) {
        printf("Error: Invalid terminal ID %d\n", terminalId);
        return;
    }
    
    terminalOutputCallbacks[terminalId] = callback;
    printf("Set output callback for terminal %d\n", terminalId);
}

// Main function for both Emscripten and non-Emscripten builds
int main(int argc, char *argv[]) 
{
#ifdef EMSCRIPTEN
    printf("ND100 WebAssembly Emulator starting...\n");
    printf("Call Init() to initialize the system\n");
    printf("Call Start() to begin emulation\n");
    return 0;
#else
    printf("nd100wasm: This program is intended to be compiled to WebAssembly.\n");
    printf("Please use emscripten to compile this program.\n");
    return 0;
#endif
}