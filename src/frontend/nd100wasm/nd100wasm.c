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
          
    // Find terminal ID in our array by identCode reference    
    for (int i = 0; i < MAX_TERMINALS; i++) {
        Device *term = terminals[i];

        if (term)
        {
            if (term->identCode == device->identCode && term->deviceClass == device->deviceClass) {                
                if (terminalOutputCallbacks[i])
                {                                  
                    terminalOutputCallbacks[i](term->identCode , c);
                }
                else
                {
                    printf("No callback for terminal %d\n", term->identCode);
                }
                break;
            }
        }
    }    
}


// Initialize the system
EMSCRIPTEN_EXPORT void Init(int boot_smd)
{
 
    
    // Only initialize once
    if (initialized) {
        printf("Already initialized.\n");
        return;
    }
    
    // Initialize machine components including devices
    machine_init(0); // No debugger in WASM versionmake

    // Initialize terminal references - we'll start with console (index 0)
    terminals[0] = DeviceManager_GetDeviceByAddress(0300); // Console
    if (!terminals[0]) {
        printf("Warning: Console terminal device not found!\n");
    } 


    // Add other terminals!!

    //     {0340, 044, 044, "TERMINAL 5/ TET12"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 5);
    terminals[1]  = DeviceManager_GetDeviceByAddress(0340);

    // {0350, 045, 045, "TERMINAL 6/ TET11"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 6);
    terminals[2] = DeviceManager_GetDeviceByAddress(0350);

    // {0360, 046, 046, "TERMINAL 7/ TET10"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 7);
    terminals[3] = DeviceManager_GetDeviceByAddress(0360);    

    // Set up character device output handler for the console terminal
    for (int i = 0; i < MAX_TERMINALS; i++) {
        if (terminals[i]) {
            Device_SetCharacterOutput(terminals[i], WasmTerminalOutputHandler);            
        }
    }
    
    
    // Load boot program (hardcoded for now)
    if (boot_smd)
    {
        program_load(BOOT_SMD, "SMD0.IMG", 1);
    }
    else
    {
        program_load(BOOT_FLOPPY, "FLOPPY.IMG", 1);
    }
    
    
    gPC = STARTADDR; // Default start address
    
    initialized = 1;    
}

// Send a key to a specific terminal device
EMSCRIPTEN_EXPORT int SendKeyToTerminal(int identCode, int keyCode)
{
    // Find the terminal with matching ID
    Device* terminal = NULL;
    for (int i = 0; i < MAX_TERMINALS; i++) {
        if (terminals[i] && terminals[i]->identCode == identCode) {
            terminal = terminals[i];
            break;
        }
    }

    // Validate terminal was found
    if (!terminal) {
        printf("Error: Terminal with IdentCode %d not found\n", identCode);
        return 0; // Failure
    }
    
    // If device is a character device, use the character input function
    if (terminal->deviceClass == DEVICE_CLASS_CHARACTER && 
        terminal->charCallbacks.inputFunc) {
        Device_InputCharacter(terminal, (char)keyCode);
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

// Get the identCode of a terminal by ID (for JS tab naming)
EMSCRIPTEN_EXPORT int GetTerminalIdentCode(int terminalId)
{
    if (terminalId < 0 || terminalId >= MAX_TERMINALS || !terminals[terminalId]) {
        return -1;
    }
    return terminals[terminalId]->identCode;
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


// Stop the emulation
EMSCRIPTEN_EXPORT void Stop()
{
    running = 0;
    //machine_stop();
}

// Set a callback for terminal output
EMSCRIPTEN_EXPORT void SetTerminalOutputCallback(int identCode, void (*callback)(int identCode, char c))
{
    for (int i = 0; i < MAX_TERMINALS; i++) {
        Device *term = terminals[i];

        if (term)
        {
            if (term->identCode == identCode) {         
                terminalOutputCallbacks[i] = callback;                
                return;
            }
        }
    }    
}

// Main function for both Emscripten and non-Emscripten builds
int main(int argc, char *argv[]) 
{
#ifdef EMSCRIPTEN
    
    //printf("Call Init() to initialize the system\n");
    //printf("Call Start() to begin emulation\n");
    return 0;
#else
    printf("nd100wasm: This program is intended to be compiled to WebAssembly.\n");
    printf("Please use emscripten to compile this program.\n");
    return 0;
#endif
}