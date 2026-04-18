/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2006 Per-Olof Astrom
 * Copyright (c) 2006-2008 Roger Abrahamsson
 * Copyright (c) 2008 Zdravko
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100em project.
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
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h> // For signal / sigaction
#include <string.h> // For memset
#include <pthread.h>
#include <limits.h>
#include <time.h>   // for time()

#ifdef _WIN32
#  include <windows.h>   /* Sleep, GetProcessTimes, CreateDirectoryA */
#  include <direct.h>    /* _mkdir */
#  define ND_MKDIR(p) _mkdir(p)
#else
#  include <termios.h>
#  include <unistd.h>
#  include <poll.h>
#  include <sys/resource.h>  /* getrusage, struct rusage */
#  include <sys/time.h>      /* struct timeval */
#  define ND_MKDIR(p) mkdir((p), 0755)
#endif


#include "../ndlib/ndlib_types.h"
#include "../ndlib/ndlib_protos.h"

#include "../machine/machine_types.h"
#include "../machine/machine_protos.h"
#include "devices_types.h"
#include "../../devices/devices_protos.h"

#ifdef WITH_DEBUGGER
void stop_debugger_thread();
#endif

#include "nd100x_types.h"
#include "nd100x_protos.h"
#include "keyboard.h"
#include "vscreen.h"

#include "../../devices/papertape/devicePapertape.h"
#include "../../devices/papertapewriter/devicePaperTapeWriter.h"
#include "../../ndlib/printjob.h"

#include "screenmenu.h"

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
#include "../../ndlib/telnetserver.h"
#endif

// dont include debugger.h here, it will cause other problems, but we define the function here
void debugger_kbd_input(char c) ;

#ifndef _WIN32
struct rusage *used;
#endif

double usertime;
double systemtime;
double totaltime;
Config_t config;

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
static TelnetServer *telnetServer = NULL;

// Carrier callback for telnet connect/disconnect
static void set_terminal_carrier(Device *dev, bool missing)
{
    if (!dev) return;
    TerminalData *data = (TerminalData *)dev->deviceData;
    if (!data) return;

    data->noCarrier = missing;
    data->inputStatus.bits.carrierMissing = missing ? 1 : 0;

    if (missing) {
        // Queue a space character to wake SINTRAN
        Terminal_QueueKeyCode(dev, ' ');
    }
}
#endif

void handle_sigint(int sig) {
    printf("\nCaught signal %d (Ctrl-C). Cleaning up...\n", sig);

#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (telnetServer) {
        TelnetServer_Stop(telnetServer);
        TelnetServer_Destroy(telnetServer);
        telnetServer = NULL;
    }
#endif

#ifdef WITH_DEBUGGER
    // Stop the debugger server gracefully
    stop_debugger_thread();
#endif

    // Stop the machine
    machine_stop();

    // Restore terminal settings before exit
    unsetcbreak();

    // Exit the program
    exit(0);
}




void register_signals()
{
#ifdef _WIN32
    // Windows signal handling
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);
#else
    // POSIX signal handling using sigaction
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));  // Initialize the structure
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart interrupted system calls
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGABRT, &sa, NULL) == -1) {
        perror("sigabrt");
        exit(1);
    }
#endif
}

void dump_stats()
{
#ifdef _WIN32
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        ULARGE_INTEGER u, k;
        u.LowPart  = user.dwLowDateTime;  u.HighPart  = user.dwHighDateTime;
        k.LowPart  = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
        /* FILETIME is in 100ns units. */
        usertime   = (double)u.QuadPart / 10000000.0;
        systemtime = (double)k.QuadPart / 10000000.0;
    } else {
        usertime = systemtime = 0.0;
    }
#else
    getrusage(RUSAGE_SELF, used); /* Read how much resources we used */
    usertime   = used->ru_utime.tv_sec + ((float)used->ru_utime.tv_usec / 1000000);
    systemtime = used->ru_stime.tv_sec + ((float)used->ru_stime.tv_usec / 1000000);
#endif
    totaltime = (float)usertime + (float)systemtime;

    printf("Number of instructions run: %lu, time used: %f\n", instr_counter, totaltime);
    printf("usertime: %f  systemtime: %f\n", usertime, systemtime);
    if (instr_counter > 0) {
        printf("Current cpu cycle time is:%f microsecs\n",
               (totaltime / ((double)instr_counter / 1000000.0)));
    }
}

/// @brief Initialize the emulator. Add devices and load program

void initialize()
{
   srand ( time(NULL) ); /* Generate PRNG Seed */
#ifndef _WIN32
   used=calloc(1,sizeof(struct rusage)); /* Perf counter stuff */
#endif


	//blocksignals();
	register_signals();
	
	if (DISASM) disasm_init();

	machine_init(config.debuggerEnabled, config.debuggerPort);

    //     {0340, 044, 044, "TERMINAL 5/ TET12"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 5);
    
    // {0350, 045, 045, "TERMINAL 6/ TET11"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 6);
    

    // {0360, 046, 046, "TERMINAL 7/ TET10"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 7);


    // {0370, 047, 047, "TERMINAL 8/ TET9"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 8);

    // {01300, 050, 060, "TERMINAL 9"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 9);

    // {01310, 051, 061, "TERMINAL 10"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 10);

    // {01320, 052, 062, "TERMINAL 11"},
    DeviceManager_AddDevice(DEVICE_TYPE_TERMINAL, 11);

	program_load(config.bootType, config.imageFile, config.verbose, (uint16_t)config.textStart);
	gPC = STARTADDR;

	/* Direct input/output enabled */
	setcbreak ();
	setvbuf(stdout, NULL, _IONBF, 0);
}




void cleanup()
{
	cleanup_machine();
	unsetcbreak ();	
}



// =========================================================
// VScreen (virtual terminal switching) state
// =========================================================
static VScreen screens[VSCREEN_MAX];
static int screenCount = 0;
static int activeScreen = 0;
static MenuState menuState;

// =========================================================
// Log VScreen state
// =========================================================
static int logScreenIndex = -1;
static pthread_mutex_t logScreenMutex = PTHREAD_MUTEX_INITIALIZER;

// =========================================================
// Printer output state (managed by PrintJob in ndlib)
// =========================================================
static PrintJob *printJob = NULL;

// =========================================================
// Paper tape writer file output state
//
// Single-threaded: tapeWriterJobNumber increments per flush.
// Tape data is accumulated in the device's own buffer and
// flushed to a .bpun file on timeout or shutdown.
// =========================================================
static int tapeWriterJobNumber = 0;
#define TAPE_WRITER_JOB_TIMEOUT 5  // seconds of silence = end of tape job

// Helper: ensure directory exists (mkdir -p equivalent for one level)
static void ensure_directory(const char *path)
{
    if (ND_MKDIR(path) != 0 && errno != EEXIST) {
        fprintf(stderr, "Failed to create directory: %s\n", path);
    }
}

// Get the effective tape directory
static const char* get_tape_dir(void)
{
    return config.tapeDir ? config.tapeDir : "./tapes";
}


// VScreen output handler - routes output to the right screen buffer
// and only prints to physical terminal if screen is active
static void VScreenOutputHandler(Device *device, char c)
{
    if (!device) return;

    for (int i = 0; i < screenCount; i++) {
        if (screens[i].device == device) {
            VScreen_Write(&screens[i], c);
            if (i == activeScreen && !menu_is_active(&menuState)) {
                printf("%c", c);
            }
            return;
        }
    }

    // Fallback: just print (if no menu visible)
    if (!menu_is_active(&menuState)) {
        printf("%c", c);
    }
}

// Dedicated printer output handler - routes to PrintJob, also feeds VScreen
static void PrinterOutputHandler(Device *device, char c)
{
    if (!device) return;

    // Feed character to PrintJob manager
    if (printJob) {
        PrintJob_PutChar(printJob, c);
    }

    // Also write to VScreen if printer has one
    for (int i = 0; i < screenCount; i++) {
        if (screens[i].device == device) {
            VScreen_Write(&screens[i], c);
            if (i == activeScreen && !menu_is_active(&menuState)) {
                printf("%c", c);
            }
            return;
        }
    }
}

// Dedicated paper tape writer output handler - accumulates bytes
// Tape buffer is maintained in the device itself (PaperTapeWriterData.tapeBuffer).
// This handler stores to file on timeout or shutdown.
static time_t tapeWriterLastOutputTime = 0;
static bool tapeWriterActive = false;

static void PaperTapeWriterOutputHandler(Device *device, char c)
{
    (void)c;  // Data is stored in device's tapeBuffer already
    if (!device) return;
    tapeWriterLastOutputTime = time(NULL);
    tapeWriterActive = true;
}

// Flush paper tape writer output to a .bpun file
static void flush_tape_writer(Device *ptw)
{
    if (!ptw || !tapeWriterActive) return;

    size_t length = 0;
    const uint8_t *data = PaperTapeWriter_GetTapeData(ptw, &length);
    if (!data || length == 0) return;

    ensure_directory(get_tape_dir());
    tapeWriterJobNumber++;
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/tape-%d.bpun",
             get_tape_dir(), tapeWriterJobNumber);

    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(data, 1, length, f);
        fclose(f);
        Log(LOG_INFO, "Paper tape job %d saved: %s (%zu bytes)\n",
               tapeWriterJobNumber, filename, length);
    }
    tapeWriterActive = false;
}

// Log output handler - routes Log() messages to the Log VScreen
static void LogScreenHandler(const char *msg)
{
    if (logScreenIndex < 0) return;

    pthread_mutex_lock(&logScreenMutex);
    for (const char *p = msg; *p; p++) {
        VScreen_Write(&screens[logScreenIndex], *p);
    }
    if (logScreenIndex == activeScreen && !menu_is_active(&menuState)) {
        fputs(msg, stdout);
        fflush(stdout);
    }
    pthread_mutex_unlock(&logScreenMutex);
}

// Derive terminal display name using the same algorithm as the glass UI:
// Use logicalDevice if set, otherwise fall back to identCode.
static void make_terminal_name(char *buf, size_t bufsize, Device *dev)
{
    uint16_t num = dev->logicalDevice ? dev->logicalDevice : dev->identCode;
    snprintf(buf, bufsize, "Terminal %d", num);
}

// Find screen index for a device
static int findScreenForDevice(Device *device)
{
    for (int i = 0; i < screenCount; i++) {
        if (screens[i].device == device) return i;
    }
    return -1;
}

// Load paper tape from file
static void load_paper_tape_file(Device *ptr, const char *filename)
{
    if (!ptr || !filename) return;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open paper tape file: %s\n", filename);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        fprintf(stderr, "Paper tape file is empty: %s\n", filename);
        return;
    }

    uint8_t *data = malloc((size_t)size);
    if (!data) {
        fclose(f);
        fprintf(stderr, "Failed to allocate memory for paper tape\n");
        return;
    }

    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        fprintf(stderr, "Failed to read paper tape file\n");
        return;
    }
    fclose(f);

    PaperTape_LoadTape(ptr, data, (size_t)size);
    free(data);
}


int main(int argc, char *argv[]) 
{

	// Initialize the configuration
    Config_Init(&config);

	 // Parse command line arguments
    if (!Config_ParseCommandLine(&config, argc, argv)) {
        Config_PrintHelp(argv[0]);
        return EXIT_FAILURE;
    }
    
    // Show help if requested
    if (config.showHelp) {
        Config_PrintHelp(argv[0]);
        return EXIT_SUCCESS;
    }
    
    if (config.debuggerEnabled) {
        printf("DAP Debugger enabled on port %d\n", config.debuggerPort);
    }

    // Set global variables from config
    DISASM = config.disasmEnabled;
    STARTADDR = config.startAddress;
    smd_debug_enabled = config.smdDebug;
    CPU_TRACE = config.traceEnabled;
    CPU_MAX_INSTR = config.maxInstructions;
    CPU_BREAKPOINT_ENABLED = config.breakpointEnabled;
    CPU_BREAKPOINT_ADDR = (ushort)config.breakpointAddr;

    initialize();

    // Add HDLC devices if configured via command line
    for (int i = 0; i < config.hdlcCount; i++) {
        machine_add_hdlc(config.hdlc[i].deviceNum,
                         config.hdlc[i].isServer,
                         config.hdlc[i].address,
                         config.hdlc[i].port);
    }

    // =========================================================
    // Set up terminal devices with VScreen output handlers
    // =========================================================
    Device *terminal = DeviceManager_GetDeviceByAddress(0300);
    if (!terminal) {
        printf("Terminal device not found\n");
        return EXIT_FAILURE;
    }

    // Initialize VScreens for all character devices
    screenCount = 0;

    // Screen 0: Console terminal
    VScreen_Init(&screens[screenCount], "Console", terminal, 80, true);
    Device_SetCharacterOutput(terminal, VScreenOutputHandler);
    screenCount++;

    // Additional terminals — names derived from logicalDevice (same algorithm as glass UI)
    static const uint16_t termAddresses[] = { 0340, 0350, 0360, 0370, 01300, 01310, 01320 };
    Device *extraTerminals[7] = {0};
    char tname[32];

    for (int i = 0; i < 7; i++) {
        extraTerminals[i] = DeviceManager_GetDeviceByAddress(termAddresses[i]);
        if (extraTerminals[i]) {
            make_terminal_name(tname, sizeof(tname), extraTerminals[i]);
            VScreen_Init(&screens[screenCount], tname, extraTerminals[i], 80, true);
            Device_SetCharacterOutput(extraTerminals[i], VScreenOutputHandler);
            screenCount++;
        }
    }

    // Create print job manager based on CLI options
    {
        PjPrinterType ptype = (config.printerType == PRINTER_ESCP) ? PJ_PRINTER_ESCP : PJ_PRINTER_TEXT;
        PjOutputFormat pfmt = (config.printFormat == PRINT_FORMAT_PDF) ? PJ_FORMAT_PDF : PJ_FORMAT_TXT;
        const char *printDir = config.printDir ? config.printDir : "./prints";
        printJob = PrintJob_Create(ptype, pfmt, printDir);
    }

    // Screen: Line Printer (output only, file-based)
    Device *printer = DeviceManager_GetDeviceByAddress(0430);
    if (printer) {
        VScreen_Init(&screens[screenCount], "Line Printer", printer, 132, false);
        Device_SetCharacterOutput(printer, PrinterOutputHandler);
        screenCount++;
    }

    // Screen: Paper Tape Punch (output only, file-based)
    Device *ptw = DeviceManager_GetDeviceByAddress(0410);
    if (ptw) {
        VScreen_Init(&screens[screenCount], "Paper Tape Punch", ptw, 80, false);
        Device_SetCharacterOutput(ptw, PaperTapeWriterOutputHandler);
        screenCount++;
    }

    // Screen: Log messages (output only, no device)
    VScreen_Init(&screens[screenCount], "Log", NULL, 120, false);
    logScreenIndex = screenCount;
    screenCount++;
    Log_SetOutputHandler(LogScreenHandler);

    // Load paper tape file if specified on command line
    Device *ptr = DeviceManager_GetDeviceByAddress(0400);
    if (ptr && config.tapeFile) {
        load_paper_tape_file(ptr, config.tapeFile);
    }

    // Start telnet server if enabled (after all VScreen setup so origOutput is set)
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (config.telnetEnabled) {
        TelnetServerConfig tc = {
            .port = config.telnetPort,
            .maxConnections = 8,
            .transport = TRANSPORT_TELNET
        };
        telnetServer = TelnetServer_Create(&tc);
        if (telnetServer) {
            // Register terminals 5-11 (not console)
            for (int i = 0; i < 7; i++) {
                Device *dev = extraTerminals[i];
                if (!dev) continue;

                // Use VScreen name (derived from logicalDevice) for telnet display
                int si = findScreenForDevice(dev);
                TelnetTerminalInfo info = {
                    .device = dev,
                    .identCode = dev->identCode,
                    .ioAddress = dev->startAddress,
                    .name = (si >= 0) ? screens[si].name : dev->memoryName,
                    .inputFunc = Terminal_QueueKeyCode,
                    .origOutput = dev->charCallbacks.outputFunc,
                    .carrierFunc = set_terminal_carrier,
                };
                TelnetServer_RegisterTerminal(telnetServer, &info);

                // Replace output handler with telnet-aware version
                extern void telnet_output_handler(struct Device *device, char c);
                Device_SetCharacterOutput(dev, telnet_output_handler);
            }

            // Set initial localActive state: terminals 8-11 (indices 3-6) start released for telnet
            for (int r = 3; r < 7; r++) {
                if (!extraTerminals[r]) continue;
                int si = findScreenForDevice(extraTerminals[r]);
                if (si >= 0) {
                    screens[si].localActive = false;
                }
            }

            // Sync locallyActive flags to telnet server (match by device pointer)
            for (int si = 0; si < screenCount; si++) {
                if (screens[si].device) {
                    TelnetServer_SetDeviceLocallyActive(telnetServer, screens[si].device, screens[si].localActive);
                }
            }

            TelnetServer_Start(telnetServer);
        }
    }
#endif

    if (config.debuggerEnabled) {
        set_cpu_run_mode(CPU_PAUSED);
    }

    // Initialize the menu state machine
    menu_init(&menuState, screens, screenCount, &activeScreen);

    // Run the machine until it stops
    CPURunMode runMode = get_cpu_run_mode();

    while (runMode != CPU_SHUTDOWN)
    {
        runMode = get_cpu_run_mode();
        machine_run(5000);

        runMode = get_cpu_run_mode();

        // Check for print job timeout
        if (printJob) {
            PrintJob_CheckTimeout(printJob);
        }

        // Check for paper tape writer timeout (flush to file)
        if (tapeWriterActive && ptw) {
            time_t now = time(NULL);
            if (tapeWriterLastOutputTime > 0 &&
                (now - tapeWriterLastOutputTime) >= TAPE_WRITER_JOB_TIMEOUT) {
                flush_tape_writer(ptw);
            }
        }

        // Handle keyboard input
        if (runMode != CPU_SHUTDOWN)
        {
            KeyEvent key = read_key_event();

            // If menu is active, route keys to menu and check timeouts
            if (menu_is_active(&menuState)) {
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
                menu_tick(&menuState, telnetServer);
                if (key.type != KEY_NONE) {
                    menu_process_key(&menuState, &key, telnetServer);
                }
#else
                menu_tick(&menuState, NULL);
                if (key.type != KEY_NONE) {
                    menu_process_key(&menuState, &key, NULL);
                }
#endif
            } else if (key.type == KEY_ALT_DIGIT) {
                int altScreen = key.ch - '0';
                if (altScreen > 0 && altScreen <= screenCount) {
                    int target = altScreen - 1;
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
                    // Block switching to telnet-connected terminal
                    if (telnetServer &&
                        TelnetServer_IsDeviceConnected(telnetServer, screens[target].device)) {
                        // Terminal in use by telnet - ignore
                    } else
#endif
                    {
                        // If not locally active, re-activate it
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
                        if (telnetServer && screens[target].isInputCapable &&
                            !screens[target].localActive) {
                            screens[target].localActive = true;
                            TelnetServer_SetDeviceLocallyActive(telnetServer, screens[target].device, true);
                            set_terminal_carrier(screens[target].device, false);
                        }
#endif
                        activeScreen = target;
                        VScreen_Redraw(&screens[activeScreen]);
                    }
                }
            } else if (key.type == KEY_F12) {
                // F12 - enter non-blocking menu
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
                menu_enter(&menuState, telnetServer);
#else
                menu_enter(&menuState, NULL);
#endif
            } else if (key.type != KEY_NONE) {
                // Process regular characters — forward every raw byte in the
                // event (covers KEY_CHAR, KEY_ESCAPE, KEY_UNKNOWN multi-byte
                // escape sequences the emulated terminal may want to consume).
                for (int i = 0; i < key.seqLen; i++) {
                    char ch = key.seq[i];

                    // ND doesnt like \n
                    if (ch == '\n') {
                        ch = '\r';
                    }

                    if ((runMode == CPU_PAUSED)||(runMode == CPU_BREAKPOINT)) {
                        debugger_kbd_input(ch);
                    } else {
                        // Route keyboard input to the active screen's device (if input capable and locally active)
                        if (screens[activeScreen].isInputCapable &&
                            screens[activeScreen].localActive &&
                            screens[activeScreen].device) {
                            Terminal_QueueKeyCode(screens[activeScreen].device, ch);
                        }
                    }
                }
            }
        }
    }

    // Flush any pending output before shutdown
    if (printJob) {
        PrintJob_Destroy(printJob);
        printJob = NULL;
    }
    if (ptw) flush_tape_writer(ptw);

    // Stop telnet server before VScreen cleanup
#if !defined(PLATFORM_WASM) && !defined(__EMSCRIPTEN__)
    if (telnetServer) {
        TelnetServer_Stop(telnetServer);
        TelnetServer_Destroy(telnetServer);
        telnetServer = NULL;
    }
#endif

    // Cleanup VScreens
    for (int i = 0; i < screenCount; i++) {
        VScreen_Destroy(&screens[i]);
    }

#ifdef WITH_DEBUGGER
    // Stop the debugger server gracefully (if its still running)
    stop_debugger_thread();
#endif

    if (DISASM)
        disasm_dump();

    dump_stats();
    cleanup();

    // exit with A register value from WAIT instruction
    return(gCpuExitCode);
}
