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
#include <unistd.h>
#include <errno.h>

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

// CPU internals for debugger access
#include "../cpu/cpu_types.h"
#include "../cpu/cpu_protos.h"

// Debugger support (WITH_DEBUGGER is now enabled for WASM)
#ifdef WITH_DEBUGGER
#include "../debugger/debugger.h"
#include "dap_server.h"

/* Functions from debugger.c WASM API */
extern DAPServer *dbg_get_server(void);
extern const char *dbg_get_scopes_json(void);
extern const char *dbg_get_variables_json(int scope_id);
extern const char *dbg_get_stack_trace_json(void);
extern const char *dbg_get_threads_json(void);
extern int dbg_step_in(void);
extern int dbg_step_over(void);
extern int dbg_step_out(void);
#endif

// Global variables
static int initialized = 0;
static int running = 0;
static int js_terminal_handler_enabled = 0;
static int dbg_paused = 0;

// Array of device references for quick access
#define MAX_TERMINALS 16
static Device* terminals[MAX_TERMINALS] = {NULL};

// Define a function pointer type for terminal output callbacks
typedef void (*TerminalOutputCallback)(int terminalId, char c);

// Array of callbacks for terminal output
static TerminalOutputCallback terminalOutputCallbacks[MAX_TERMINALS] = {NULL};

// JavaScript terminal output handler - called from JavaScript
EMSCRIPTEN_EXPORT void TerminalOutputToJS(int identCode, char c) {
    // This is the C function that will be called from JavaScript
    // We need a special emscripten wrapper to call from JavaScript to C
    EM_ASM({
        if (typeof window.handleTerminalOutputFromC === 'function') {
            window.handleTerminalOutputFromC($0, $1);
        } else {
            console.error('handleTerminalOutputFromC not defined in JavaScript');
        }
    }, identCode, c);
}

// Enable or disable the JavaScript terminal handler
EMSCRIPTEN_EXPORT void SetJSTerminalOutputHandler(int enable) {
    js_terminal_handler_enabled = enable;
    printf("JavaScript terminal handler %s\n", enable ? "enabled" : "disabled");
}

// Character device output handler for terminals
static void WasmTerminalOutputHandler(Device *device, char c) 
{
    if (!device)
        return;
          
    if (js_terminal_handler_enabled) {
        // Use JavaScript handler - direct call to JS without function pointers
        TerminalOutputToJS(device->identCode, c);
        return;
    }
    
    // Traditional callback approach (with function pointers)
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

// Initialize the system (hardware only, no boot)
EMSCRIPTEN_EXPORT void Init(void)
{
    printf("ND100X WASM build: %s %s\n", __DATE__, __TIME__);

    // Only initialize once
    if (initialized) {
        printf("Already initialized.\n");
        return;
    }

#ifdef WITH_DEBUGGER
    // Initialize machine with debugger enabled
    machine_init(1);
#else
    // Initialize machine components including devices
    machine_init(0);
#endif

    // Add terminals 5-8 (Group 1) and 9-11 (Group 9)
    // Console (thumbwheel 0) is already added by DeviceManager_AddAllDevices
    // Total: 8 terminals (console + 7)
    for (uint8_t tw = 5; tw <= 11; tw++) {
        DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, tw);
    }

    // Populate terminals[] by scanning device manager for all terminal devices
    int termIdx = 0;
    int devCount = DeviceManager_GetDeviceCount();
    for (int i = 0; i < devCount && termIdx < MAX_TERMINALS; i++) {
        Device *dev = DeviceManager_GetDeviceByIndex(i);
        if (dev && dev->type == DEVICE_TYPE_TERMINAL) {
            terminals[termIdx++] = dev;
        }
    }

    // Set up character device output handler for the console terminal
    for (int i = 0; i < MAX_TERMINALS; i++) {
        if (terminals[i]) {
            Device_SetCharacterOutput(terminals[i], WasmTerminalOutputHandler);
        }
    }

    initialized = 1;
}

// Boot the system (load boot sector and set PC)
// boot_type: 0=FLOPPY, 1=SMD, 2=BPUN
// Returns: start address (PC) on success, -1 on failure
EMSCRIPTEN_EXPORT int Boot(int boot_type)
{
    if (!initialized) {
        printf("Error: System not initialized. Call Init() first.\n");
        return -1;
    }

    // Pre-mount drives so they are available for I/O regardless of boot choice.
    // autoMountDrives() inside program_load() will skip already-mounted drives.

    // Mount floppy unit 0 - try both MEMFS paths
    if (!isMounted(DRIVE_FLOPPY, 0)) {
        mount_floppy("/FLOPPY0.IMG", 0);
    }
    if (!isMounted(DRIVE_FLOPPY, 0)) {
        mount_floppy("FLOPPY0.IMG", 0);
    }

    // Mount SMD drives
    if (!isMounted(DRIVE_SMD, 0)) {
        mount_smd("SMD0.IMG", 0);
    }
    mount_smd(NULL, 1);
    mount_smd(NULL, 2);
    mount_smd(NULL, 3);

    int rc;
    switch (boot_type) {
    case 1: // SMD
        rc = program_load(BOOT_SMD, "SMD0.IMG", 1);
        break;
    case 2: // BPUN
        rc = program_load(BOOT_BPUN, "BPUN_UPLOAD.IMG", 1);
        break;
    default: // FLOPPY (0)
        rc = program_load(BOOT_FLOPPY, "FLOPPY0.IMG", 1);
        break;
    }

    if (rc < 0) {
        return -1;
    }

    gPC = STARTADDR;
    return (int)gPC;
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

// Get the device name of a terminal by ID (returns pointer to C string)
EMSCRIPTEN_EXPORT const char* GetTerminalName(int terminalId)
{
    if (terminalId < 0 || terminalId >= MAX_TERMINALS || !terminals[terminalId]) {
        return NULL;
    }
    return terminals[terminalId]->memoryName;
}

// Get the SINTRAN logical device number of a terminal by ID
EMSCRIPTEN_EXPORT int GetTerminalLogicalDevice(int terminalId)
{
    if (terminalId < 0 || terminalId >= MAX_TERMINALS || !terminals[terminalId]) {
        return -1;
    }
    return terminals[terminalId]->logicalDevice;
}

// Set or clear carrier status on a terminal.
// flag=1: carrier missing (closing window / hanging up)
//   - sets carrierMissing bit and enqueues a space so SINTRAN detects it
// flag=0: carrier present (reopening window / reconnecting)
//   - clears carrierMissing bit
EMSCRIPTEN_EXPORT int SetTerminalCarrier(int flag, int identCode)
{
    Device* terminal = NULL;
    for (int i = 0; i < MAX_TERMINALS; i++) {
        if (terminals[i] && terminals[i]->identCode == identCode) {
            terminal = terminals[i];
            break;
        }
    }

    if (!terminal || !terminal->deviceData) {
        printf("Error: Terminal with IdentCode %d not found\n", identCode);
        return 0;
    }

    TerminalData *data = (TerminalData *)terminal->deviceData;

    if (flag) {
        // Carrier missing - terminal window closed
        data->noCarrier = true;
        data->inputStatus.bits.carrierMissing = 1;
        // Enqueue a space so SINTRAN will poll the status and notice carrier loss
        Terminal_QueueKeyCode(terminal, ' ');
        printf("Terminal %d: carrier missing\n", identCode);
    } else {
        // Carrier present - terminal window reopened
        data->noCarrier = false;
        data->inputStatus.bits.carrierMissing = 0;
        printf("Terminal %d: carrier restored\n", identCode);
    }

    return 1;
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

// Check if machine has been initialized
EMSCRIPTEN_EXPORT int IsInitialized(void)
{
    return initialized;
}

// Remount a floppy drive (close old FILE*, re-open from MEMFS)
// Unit N uses "/FLOPPYN.IMG" (absolute path for MEMFS compatibility)
EMSCRIPTEN_EXPORT int RemountFloppy(int unit)
{
    if (unit < 0 || unit > 2) {
        return -1;
    }

    char filename[32];
    sprintf(filename, "/FLOPPY%d.IMG", unit);

    // Check file exists before attempting mount
    FILE *ftmp = fopen(filename, "rb");
    if (!ftmp) {
        return -1;
    }
    fclose(ftmp);

    // Unmount existing if mounted
    if (isMounted(DRIVE_FLOPPY, unit)) {
        unmount_drive(DRIVE_FLOPPY, unit);
    }

    // Mount from the (possibly updated) MEMFS file
    mount_drive(DRIVE_FLOPPY, unit, "md5-unknown", "Floppy", "Mounted floppy image", filename);

    return isMounted(DRIVE_FLOPPY, unit) ? 0 : -1;
}

// Remount an SMD drive (close old FILE*, re-open from MEMFS)
// Unit N uses "/SMDN.IMG" (absolute path for MEMFS compatibility)
EMSCRIPTEN_EXPORT int RemountSMD(int unit)
{
    if (unit < 0 || unit > 3) {
        return -1;
    }

    char filename[32];
    sprintf(filename, "/SMD%d.IMG", unit);

    // Unmount existing if mounted
    if (isMounted(DRIVE_SMD, unit)) {
        unmount_drive(DRIVE_SMD, unit);
    }

    // Mount from the (possibly updated) MEMFS file
    mount_drive(DRIVE_SMD, unit, "md5-unknown", "SMD", "Mounted SMD image", filename);

    return isMounted(DRIVE_SMD, unit) ? 0 : -1;
}

// Unmount a floppy drive (close FILE*, mark as not mounted)
EMSCRIPTEN_EXPORT int UnmountFloppy(int unit)
{
    if (unit < 0 || unit > 2) {
        return -1;
    }

    if (isMounted(DRIVE_FLOPPY, unit)) {
        unmount_drive(DRIVE_FLOPPY, unit);
        return 0;
    }

    return 0; // already unmounted
}

// Unmount an SMD drive (close FILE*, mark as not mounted)
EMSCRIPTEN_EXPORT int UnmountSMD(int unit)
{
    if (unit < 0 || unit > 3) {
        return -1;
    }

    if (isMounted(DRIVE_SMD, unit)) {
        unmount_drive(DRIVE_SMD, unit);
        return 0;
    }

    return 0; // already unmounted
}

// Set a callback for terminal output (traditional callback approach)
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

/* =========================================================
   Debugger API - Uses DAP debugger infrastructure for WASM
   Single-threaded, calls through debugger.c public API
   ========================================================= */

// --- Execution Control ---

EMSCRIPTEN_EXPORT void Dbg_SetPaused(int paused)
{
    dbg_paused = paused;
#ifdef WITH_DEBUGGER
    if (paused) {
        /* Request pause - set mode directly for single-threaded WASM */
        set_cpu_run_mode(CPU_PAUSED);
        set_debugger_request_pause(false);
        set_debugger_control_granted(false);
    } else {
        /* Resume - clear all debugger flags and set running */
        set_debugger_request_pause(false);
        set_debugger_control_granted(false);
        set_cpu_run_mode(CPU_RUNNING);
    }
#endif
}

EMSCRIPTEN_EXPORT int Dbg_IsPaused(void)
{
#ifdef WITH_DEBUGGER
    CPURunMode mode = get_cpu_run_mode();
    return (mode == CPU_PAUSED || mode == CPU_BREAKPOINT) ? 1 : 0;
#else
    return dbg_paused;
#endif
}

EMSCRIPTEN_EXPORT int Dbg_StepOne(void)
{
    if (!initialized) return -1;
#ifdef WITH_DEBUGGER
    /* Clear debugger pause flags so machine_run won't get stuck */
    set_debugger_request_pause(false);
    set_debugger_control_granted(false);
    set_cpu_run_mode(CPU_RUNNING);

    dbg_step_in();
    /* step_cpu sets up step breakpoint and calls ensure_cpu_running */
    machine_run(1);

    /* Ensure we end in a paused state after stepping */
    if (get_cpu_run_mode() == CPU_RUNNING) {
        set_cpu_run_mode(CPU_PAUSED);
    }
    return (int)gPC;
#else
    machine_run(1);
    IO_Tick();
    return (int)gPC;
#endif
}

EMSCRIPTEN_EXPORT int Dbg_StepOver(void)
{
    if (!initialized) return -1;
#ifdef WITH_DEBUGGER
    /* Clear debugger pause flags so machine_run won't get stuck */
    set_debugger_request_pause(false);
    set_debugger_control_granted(false);
    set_cpu_run_mode(CPU_RUNNING);

    dbg_step_over();
    /* step_cpu sets temp breakpoint at return addr for calls, or step_one for simple instr */
    machine_run(1000);

    /* Ensure we end in a paused state after stepping */
    if (get_cpu_run_mode() == CPU_RUNNING) {
        set_cpu_run_mode(CPU_PAUSED);
    }
    return (int)gPC;
#else
    machine_run(1);
    return (int)gPC;
#endif
}

EMSCRIPTEN_EXPORT int Dbg_StepOut(void)
{
    if (!initialized) return -1;
#ifdef WITH_DEBUGGER
    /* Clear debugger pause flags so machine_run won't get stuck */
    set_debugger_request_pause(false);
    set_debugger_control_granted(false);
    set_cpu_run_mode(CPU_RUNNING);

    dbg_step_out();
    /* step_cpu sets breakpoint at return address from stack trace */
    machine_run(10000);

    /* Ensure we end in a paused state after stepping */
    if (get_cpu_run_mode() == CPU_RUNNING) {
        set_cpu_run_mode(CPU_PAUSED);
    }
    return (int)gPC;
#else
    machine_run(1);
    return (int)gPC;
#endif
}

EMSCRIPTEN_EXPORT int Dbg_RunWithBreakpoints(int maxSteps)
{
    if (!initialized) return 0;

    /* Ensure clean state for running */
    set_debugger_request_pause(false);
    set_debugger_control_granted(false);

    if (get_cpu_run_mode() != CPU_RUNNING) {
        set_cpu_run_mode(CPU_RUNNING);
    }

    for (int i = 0; i < maxSteps; i++) {
        machine_run(1);
        /* machine_run(1)->cpu_run(1) already checks breakpoints internally
           and sets cpu_run_mode to CPU_BREAKPOINT if hit */
        CPURunMode mode = get_cpu_run_mode();
        if (mode != CPU_RUNNING) {
            return maxSteps - i;
        }
    }
    return 0; /* all steps consumed */
}

// --- Register Access (current runlevel) ---

EMSCRIPTEN_EXPORT int Dbg_GetPC(void)     { return (int)gPC; }
EMSCRIPTEN_EXPORT int Dbg_GetRegA(void)   { return (int)gA; }
EMSCRIPTEN_EXPORT int Dbg_GetRegD(void)   { return (int)gD; }
EMSCRIPTEN_EXPORT int Dbg_GetRegB(void)   { return (int)gB; }
EMSCRIPTEN_EXPORT int Dbg_GetRegT(void)   { return (int)gT; }
EMSCRIPTEN_EXPORT int Dbg_GetRegL(void)   { return (int)gL; }
EMSCRIPTEN_EXPORT int Dbg_GetRegX(void)   { return (int)gX; }
EMSCRIPTEN_EXPORT int Dbg_GetSTS(void)    { return (int)gSTSr; }
EMSCRIPTEN_EXPORT int Dbg_GetPIL(void)    { return (int)gPIL; }
EMSCRIPTEN_EXPORT int Dbg_GetEA(void)     { return (int)gEA; }

// --- Register Write (current runlevel) ---

EMSCRIPTEN_EXPORT void Dbg_SetPC(int val)   { gPC  = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegA(int val)  { gA   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegD(int val)  { gD   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegB(int val)  { gB   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegT(int val)  { gT   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegL(int val)  { gL   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetRegX(int val)  { gX   = (ushort)(val & 0xFFFF); }
EMSCRIPTEN_EXPORT void Dbg_SetSTS(int val)   {
    /* STS MSB is shared, LSB is per-level */
    gReg->reg_STS = (ushort)(val & 0xFF00);
    gReg->reg[gPIL][_STS] = (ushort)(val & 0x00FF);
}

// --- Register access for any runlevel ---

EMSCRIPTEN_EXPORT int Dbg_GetRegAtLevel(int level, int regIndex)
{
    if (level < 0 || level > 15 || regIndex < 0 || regIndex > 15) return -1;
    if (regIndex == _STS) {
        /* STS is split: MSB shared, LSB per-level */
        return (int)((gReg->reg_STS & 0xFF00) | (gReg->reg[level][_STS] & 0x00FF));
    }
    return (int)gReg->reg[level][regIndex];
}

// --- Privileged System Registers (read-only) ---

EMSCRIPTEN_EXPORT int Dbg_GetPANS(void)   { return (int)gPANS; }
EMSCRIPTEN_EXPORT int Dbg_GetOPR(void)    { return (int)gOPR; }
EMSCRIPTEN_EXPORT int Dbg_GetPGS(void)    { return (int)gPGS; }
EMSCRIPTEN_EXPORT int Dbg_GetPVL(void)    { return (int)gPVL; }
EMSCRIPTEN_EXPORT int Dbg_GetIIC(void)    { return (int)gIIC; }
EMSCRIPTEN_EXPORT int Dbg_GetIID(void)    { return (int)gIID; }
EMSCRIPTEN_EXPORT int Dbg_GetPID(void)    { return (int)gPID; }
EMSCRIPTEN_EXPORT int Dbg_GetPIE(void)    { return (int)gPIE; }
EMSCRIPTEN_EXPORT int Dbg_GetCSR(void)    { return (int)gCSR; }
EMSCRIPTEN_EXPORT int Dbg_GetALD(void)    { return (int)gALD; }
EMSCRIPTEN_EXPORT int Dbg_GetPES(void)    { return (int)gPES; }
EMSCRIPTEN_EXPORT int Dbg_GetPGC(void)    { return (int)gPGC; }
EMSCRIPTEN_EXPORT int Dbg_GetPEA(void)    { return (int)gPEA; }
EMSCRIPTEN_EXPORT int Dbg_GetPCR(int level)
{
    if (level < 0 || level > 15) return -1;
    return (int)gReg->reg_PCR[level];
}

// --- Privileged System Registers (write-only but readable from struct) ---

EMSCRIPTEN_EXPORT int Dbg_GetPANC(void)   { return (int)gPANC; }
EMSCRIPTEN_EXPORT int Dbg_GetLMP(void)    { return (int)gLMP; }
EMSCRIPTEN_EXPORT int Dbg_GetIIE(void)    { return (int)gIIE; }
EMSCRIPTEN_EXPORT int Dbg_GetCCL(void)    { return (int)gCCL; }
EMSCRIPTEN_EXPORT int Dbg_GetLCIL(void)   { return (int)gLCIL; }
EMSCRIPTEN_EXPORT int Dbg_GetUCIL(void)   { return (int)gUCIL; }
EMSCRIPTEN_EXPORT int Dbg_GetECCR(void)   { return (int)gECCR; }

// --- Instruction Counter ---

EMSCRIPTEN_EXPORT double Dbg_GetInstrCount(void)
{
    /* Return as double since JS numbers can hold 53-bit integers */
    return (double)instr_counter;
}

// --- CPU State ---

EMSCRIPTEN_EXPORT int Dbg_GetRunMode(void)
{
    return (int)get_cpu_run_mode();
}

EMSCRIPTEN_EXPORT int Dbg_GetStopReason(void)
{
    return (int)get_cpu_stop_reason();
}

// --- Memory Access ---

EMSCRIPTEN_EXPORT int Dbg_ReadMemory(int addr)
{
    return (int)MemoryRead((ushort)(addr & 0xFFFF), false);
}

// --- Bulk Memory Read (for SINTRAN data structure inspection) ---

static uint16_t mem_block_buffer[4096];

EMSCRIPTEN_EXPORT int Dbg_ReadMemoryBlock(int startAddr, int count)
{
    if (count <= 0 || count > 4096) count = 4096;
    for (int i = 0; i < count; i++) {
        mem_block_buffer[i] = MemoryRead((ushort)((startAddr + i) & 0xFFFF), false);
    }
    return (int)(uintptr_t)mem_block_buffer;
}

EMSCRIPTEN_EXPORT void Dbg_WriteMemory(int addr, int val)
{
    MemoryWrite((ushort)val, (ushort)(addr & 0xFFFF), false, 2);
}

// --- Physical Memory Dump (raw physical memory, bypasses MMS) ---

EMSCRIPTEN_EXPORT int Dbg_DumpPhysicalMemory(int wordCount)
{
    if (wordCount <= 0 || wordCount > (int)(sizeof(VolatileMemory) / sizeof(ushort)))
        wordCount = 256 * 1024;

    FILE *f = fopen("/nd100_physmem.bin", "wb");
    if (!f) return -1;

    for (int i = 0; i < wordCount; i++) {
        ushort w = VolatileMemory.n_Array[i];
        unsigned char hi = (w >> 8) & 0xFF;
        unsigned char lo = w & 0xFF;
        fputc(hi, f);
        fputc(lo, f);
    }
    fclose(f);
    return 0;
}

// --- Breakpoints ---

EMSCRIPTEN_EXPORT void Dbg_AddBreakpoint(int addr)
{
    breakpoint_manager_add((uint16_t)(addr & 0xFFFF), BP_TYPE_USER, NULL, NULL, NULL);
}

EMSCRIPTEN_EXPORT void Dbg_RemoveBreakpoint(int addr)
{
    breakpoint_manager_remove((uint16_t)(addr & 0xFFFF), BP_TYPE_USER);
}

EMSCRIPTEN_EXPORT void Dbg_ClearBreakpoints(void)
{
    breakpoint_manager_clear();
}

// --- Breakpoint Listing ---

static char bp_list_buffer[4096];

extern BreakpointManager *mgr;

EMSCRIPTEN_EXPORT const char* Dbg_GetBreakpointList(void)
{
    int pos = 0;
    bp_list_buffer[0] = '\0';

    if (!mgr) return bp_list_buffer;

    for (int h = 0; h < HASH_SIZE; h++) {
        BreakpointEntry *curr = mgr->buckets[h];
        while (curr && pos < (int)sizeof(bp_list_buffer) - 64) {
            int n = snprintf(bp_list_buffer + pos, sizeof(bp_list_buffer) - pos,
                "%d %d %d\n", curr->address, curr->type, curr->hitCount);
            if (n > 0) pos += n;
            curr = curr->next;
        }
    }
    bp_list_buffer[pos] = '\0';
    return bp_list_buffer;
}

// --- Watchpoints (memory access breakpoints) ---

EMSCRIPTEN_EXPORT int Dbg_AddWatchpoint(int addr, int type)
{
    return watchpoint_add((uint16_t)(addr & 0xFFFF), (WatchpointType)type);
}

EMSCRIPTEN_EXPORT void Dbg_RemoveWatchpoint(int addr)
{
    watchpoint_remove((uint16_t)(addr & 0xFFFF));
}

EMSCRIPTEN_EXPORT void Dbg_ClearWatchpoints(void)
{
    watchpoint_clear();
}

EMSCRIPTEN_EXPORT int Dbg_GetWatchpointCount(void)
{
    return watchpoint_get_count();
}

EMSCRIPTEN_EXPORT int Dbg_GetWatchpointAddr(int index)
{
    uint16_t addr;
    int type;
    if (watchpoint_get(index, &addr, &type) == 0)
        return (int)addr;
    return -1;
}

EMSCRIPTEN_EXPORT int Dbg_GetWatchpointType(int index)
{
    uint16_t addr;
    int type;
    if (watchpoint_get(index, &addr, &type) == 0)
        return type;
    return -1;
}

// --- Disassembly ---

static char disasm_buffer[8192];

EMSCRIPTEN_EXPORT const char* Dbg_Disassemble(int startAddr, int count)
{
    char line[128];
    char mnemonic[64];
    int pos = 0;

    disasm_buffer[0] = '\0';

    for (int i = 0; i < count && pos < (int)sizeof(disasm_buffer) - 128; i++) {
        ushort addr = (ushort)((startAddr + i) & 0xFFFF);
        ushort word = MemoryRead(addr, false);

        OpToStr(mnemonic, sizeof(mnemonic), word);

        int n = snprintf(line, sizeof(line), "%06o %06o %s\n", addr, word, mnemonic);
        if (n > 0 && pos + n < (int)sizeof(disasm_buffer)) {
            memcpy(disasm_buffer + pos, line, n);
            pos += n;
        }
    }
    disasm_buffer[pos] = '\0';
    return disasm_buffer;
}

// --- Level info (for thread/runlevel view) ---

static char levels_buffer[4096];

EMSCRIPTEN_EXPORT const char* Dbg_GetLevelInfo(void)
{
    int pos = 0;
    levels_buffer[0] = '\0';

    for (int lev = 0; lev < 16; lev++) {
        ushort pcr = gReg->reg_PCR[lev];
        int ring = pcr & 0x03;
        int pt = (pcr >> 11) & 0x0F;
        int apt = (pcr >> 7) & 0x0F;
        ushort p_reg = gReg->reg[lev][_P];
        ushort sts_lsb = gReg->reg[lev][_STS] & 0xFF;

        int n = snprintf(levels_buffer + pos, sizeof(levels_buffer) - pos,
            "%d %06o %03o R%d PT%d APT%d\n",
            lev, p_reg, sts_lsb, ring, pt, apt);
        if (n > 0) pos += n;
    }
    levels_buffer[pos] = '\0';
    return levels_buffer;
}

/* =========================================================
   DAP-based variable/scope/stack/thread inspection
   Returns JSON strings for JS consumption
   ========================================================= */

#ifdef WITH_DEBUGGER
EMSCRIPTEN_EXPORT const char* Dbg_GetScopes(void)
{
    return dbg_get_scopes_json();
}

EMSCRIPTEN_EXPORT const char* Dbg_GetVariables(int scopeId)
{
    return dbg_get_variables_json(scopeId);
}

EMSCRIPTEN_EXPORT const char* Dbg_GetThreads(void)
{
    return dbg_get_threads_json();
}

EMSCRIPTEN_EXPORT const char* Dbg_GetStackTrace(void)
{
    return dbg_get_stack_trace_json();
}
#else
EMSCRIPTEN_EXPORT const char* Dbg_GetScopes(void) { return "[]"; }
EMSCRIPTEN_EXPORT const char* Dbg_GetVariables(int scopeId) { (void)scopeId; return "[]"; }
EMSCRIPTEN_EXPORT const char* Dbg_GetThreads(void) { return "[]"; }
EMSCRIPTEN_EXPORT const char* Dbg_GetStackTrace(void) { return "[]"; }
#endif

// --- Physical Memory Access (bypasses MMS, direct array read) ---

EMSCRIPTEN_EXPORT int Dbg_ReadPhysicalMemory(int physAddr)
{
    if (physAddr < 0 || physAddr >= (int)ND_Memsize)
        return 0;
    return (int)VolatileMemory.n_Array[physAddr];
}

static uint16_t phys_block_buffer[4096];

EMSCRIPTEN_EXPORT int Dbg_ReadPhysicalMemoryBlock(int startAddr, int count)
{
    if (count <= 0 || count > 4096) count = 4096;
    int maxAddr = (int)ND_Memsize;
    for (int i = 0; i < count; i++) {
        int addr = startAddr + i;
        if (addr >= 0 && addr < maxAddr)
            phys_block_buffer[i] = VolatileMemory.n_Array[addr];
        else
            phys_block_buffer[i] = 0;
    }
    return (int)(uintptr_t)phys_block_buffer;
}

// --- Page Table Access ---

EMSCRIPTEN_EXPORT int Dbg_GetPageTableCount(void)
{
    /* MMS1 = 4 page tables, MMS2 = 16 page tables */
    return (mmsType == MMS1) ? 4 : 16;
}

EMSCRIPTEN_EXPORT int Dbg_GetPageTableEntryRaw(int pageTable, int vpn)
{
    if (pageTable < 0 || pageTable >= 16) return 0;
    if (vpn < 0 || vpn >= 64) return 0;

    /* Use debugger reader which checks mmsType instead of STS_SEXI,
       so we can read all 16 page tables even when paused at a level without SEXI. */
    PageTableMode ptm = (mmsType == MMS2) ? Sixteen : Four;
    uint pte = GetPageTableEntryForDebugger((uint)pageTable, (uint)vpn, ptm);
    return (int)pte;
}

EMSCRIPTEN_EXPORT int Dbg_GetExtendedMode(void)
{
    return STS_SEXI ? 1 : 0;
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