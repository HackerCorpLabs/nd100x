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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "nd100x_types.h"
#include "../../machine/machine_types.h"
#include "../../devices/hdlc/hdlc_constants.h"

// Long options
static struct option long_options[] = {
    {"boot",       required_argument, 0, 'b'},
    {"image",      required_argument, 0, 'i'},
    {"start",      required_argument, 0, 's'},
    {"disasm",     no_argument,       0, 'a'},
    {"verbose",    no_argument,       0, 'v'},
    {"help",       no_argument,       0, 'h'},
    {"debugger",   no_argument,       0, 'd'},
    {"port",       required_argument, 0, 'p'},
    {"smd-debug",  no_argument,       0, 'S'},
    {"trace",      no_argument,       0, 't'},
    {"max-instr",  required_argument, 0, 'n'},
    {"breakpoint", required_argument, 0, 'B'},
    {"text-start", required_argument, 0, 'T'},
    {"printdir",   required_argument, 0, 'P'},
    {"tapedir",    required_argument, 0, 'D'},
    {"tape",       required_argument, 0, 'e'},
    {"telnet",     optional_argument, 0, 'N'},
    {"printer",    required_argument, 0, 'r'},
    {"printformat",required_argument, 0, 'f'},
    {"hdlc",       required_argument, 0, 'H'},
    {0, 0, 0, 0}
};

void Config_Init(Config_t *config) {
    if (!config) return;
    
    config->bootType = BOOT_NONE;
    config->imageFile = NULL;
    config->startAddress = 0;
    config->disasmEnabled = false;
    config->verbose = false;
    config->showHelp = false;
    config->debuggerEnabled = false;
    config->debuggerPort = 4711;
    config->smdDebug = false;
    config->traceEnabled = false;
    config->maxInstructions = 0;
    config->breakpointEnabled = false;
    config->breakpointAddr = 0;
    config->textStartSet = false;
    config->textStart = 0;
    config->printDir = NULL;
    config->tapeDir = NULL;
    config->tapeFile = NULL;
    config->telnetEnabled = false;
    config->telnetPort = 9000;
    config->printerType = PRINTER_TEXT;
    config->printFormat = PRINT_FORMAT_TXT;
    // HDLC configuration
    config->hdlcEnabled = false;
    config->hdlcAddress = NULL;
    config->hdlcPort = HDLC_DEFAULT_PORT;
    config->hdlcServer = false;
}

static BOOT_TYPE parseBootType(const char *bootStr) {
    if (!bootStr) return BOOT_NONE;
    
    if (strcmp("bp", bootStr) == 0) return BOOT_BP;
    if (strcmp("bpun", bootStr) == 0) return BOOT_BPUN;
    if (strcmp("aout", bootStr) == 0) return BOOT_AOUT;
    if (strcmp("floppy", bootStr) == 0) return BOOT_FLOPPY;
    if (strcmp("smd", bootStr) == 0) return BOOT_SMD;
    
    return BOOT_NONE;
}

static bool parseHDLCConfig(Config_t *config, const char *hdlcStr) {
    if (!hdlcStr || !config) return false;

    // Parse HDLC configuration string
    // Format: "port" for server mode or "ip:port" for client mode
    char *str = strdup(hdlcStr);
    if (!str) return false;

    char *colon = strchr(str, ':');

    if (colon) {
        // Client mode: "ip:port" or "hostname:port"
        *colon = '\0';
        char *portStr = colon + 1;

        config->hdlcAddress = strdup(str);
        if (!config->hdlcAddress) {
            free(str);
            return false;
        }

        char *endptr;
        config->hdlcPort = strtol(portStr, &endptr, 10);
        if (*endptr != '\0' || config->hdlcPort <= 0 || config->hdlcPort > 65535) {
            free(config->hdlcAddress);
            config->hdlcAddress = NULL;
            free(str);
            return false;
        }

        config->hdlcServer = false;
    } else {
        // Server mode: just port number
        char *endptr;
        config->hdlcPort = strtol(str, &endptr, 10);
        if (*endptr != '\0' || config->hdlcPort <= 0 || config->hdlcPort > 65535) {
            free(str);
            return false;
        }

        config->hdlcAddress = NULL;
        config->hdlcServer = true;
    }

    free(str);
    config->hdlcEnabled = true;
    return true;
}

bool Config_ParseCommandLine(Config_t *config, int argc, char *argv[]) {
    int option_index = 0;
    int c;
    char *endptr;
    
    while ((c = getopt_long(argc, argv, "b:i:s:avhdp:Stn:B:T:P:D:e:N::r:f:H:",
                           long_options, &option_index)) != -1) {
        switch (c) {
            case 'b':
                config->bootType = parseBootType(optarg);
                if (config->bootType == BOOT_NONE) {
                    fprintf(stderr, "Invalid boot type: %s\n", optarg);
                    return false;
                }
                break;
                
            case 'i':
                config->imageFile = strdup(optarg);
                if (!config->imageFile) {
                    fprintf(stderr, "Failed to allocate memory for image file\n");
                    return false;
                }
                break;
                
            case 's':
                config->startAddress = strtoul(optarg, &endptr, 0);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid start address: %s\n", optarg);
                    return false;
                }
                break;
                
            case 'a':
                config->disasmEnabled = true;
                break;

            case 'd':
                config->debuggerEnabled = true;
                break;

            case 'p':
                config->debuggerPort = (int)strtol(optarg, &endptr, 0);
                if (*endptr != '\0' || config->debuggerPort <= 0 || config->debuggerPort > 65535) {
                    fprintf(stderr, "Invalid port number: %s\n", optarg);
                    return false;
                }
                break;

            case 'v':
                config->verbose = true;
                break;
                
            case 'P':
                config->printDir = strdup(optarg);
                break;

            case 'D':
                config->tapeDir = strdup(optarg);
                break;

            case 'e':
                config->tapeFile = strdup(optarg);
                break;

            case 'N':
                config->telnetEnabled = true;
                if (optarg) {
                    char *portEnd;
                    long port = strtol(optarg, &portEnd, 10);
                    if (*portEnd != '\0' || port <= 0 || port > 65535) {
                        fprintf(stderr, "Invalid telnet port: %s\n", optarg);
                        return false;
                    }
                    config->telnetPort = (int)port;
                }
                break;

            case 'r':
                if (strcmp(optarg, "text") == 0) {
                    config->printerType = PRINTER_TEXT;
                } else if (strcmp(optarg, "escp") == 0) {
                    config->printerType = PRINTER_ESCP;
                } else if (strcmp(optarg, "laser") == 0) {
                    fprintf(stderr, "Laser printer emulation is not yet implemented\n");
                    return false;
                } else {
                    fprintf(stderr, "Invalid printer type: %s (use text, escp, or laser)\n", optarg);
                    return false;
                }
                break;

            case 'f':
                if (strcmp(optarg, "txt") == 0) {
                    config->printFormat = PRINT_FORMAT_TXT;
                } else if (strcmp(optarg, "pdf") == 0) {
                    config->printFormat = PRINT_FORMAT_PDF;
                } else {
                    fprintf(stderr, "Invalid print format: %s (use txt or pdf)\n", optarg);
                    return false;
                }
                break;

            case 'h':
                config->showHelp = true;
                return true;

            case 'H':
                if (!parseHDLCConfig(config, optarg)) {
                    fprintf(stderr, "Invalid HDLC configuration: %s\n", optarg);
                    return false;
                }
                break;

            case 'S':
                config->smdDebug = true;
                break;

            case 't':
                config->traceEnabled = true;
                break;

            case 'n':
                config->maxInstructions = strtoull(optarg, &endptr, 0);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid max instruction count: %s\n", optarg);
                    return false;
                }
                break;

            case 'B':
                config->breakpointEnabled = true;
                config->breakpointAddr = strtoul(optarg, &endptr, 0);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid breakpoint address: %s\n", optarg);
                    return false;
                }
                break;

            case 'T':
                config->textStartSet = true;
                config->textStart = strtoul(optarg, &endptr, 0);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid text start address: %s\n", optarg);
                    return false;
                }
                break;

            case '?':
                return false;
                
            default:
                fprintf(stderr, "Unknown option: %c\n", c);
                return false;
        }
    }
    
    // Check required arguments
    if ((!config->showHelp && !config->debuggerEnabled)) {
        if (config->bootType == BOOT_NONE) {
            config->bootType = BOOT_SMD;

            fprintf(stderr, "Boot type must be specified\n");
            return false;
        }
        if (!config->imageFile) {
            if (config->bootType == BOOT_FLOPPY) {
                config->imageFile = strdup("FLOPPY.IMG");
            } else
            if (config->bootType != BOOT_SMD) {
                fprintf(stderr, "Image file must be specified\n");
                return false;
            }
        }
    }
    
   if (config->verbose) {
        printf("Configuration:\n");
        printf("  Boot type: %s\n", boot_type_str[config->bootType]);
        printf("  Image file: %s\n", config->imageFile);
        printf("  Start address: 0x%x\n", config->startAddress);
        printf("  Disassembly: %s\n", config->disasmEnabled ? "enabled" : "disabled");
        if (config->hdlcEnabled) {
            if (config->hdlcServer) {
                printf("  HDLC: Server mode on port %d\n", config->hdlcPort);
            } else {
                printf("  HDLC: Client mode to %s:%d\n", config->hdlcAddress, config->hdlcPort);
            }
        }
    }
    

    return true;
}

void Config_PrintHelp(const char *progName) {
    printf("Usage: %s [options]\n\n", progName);
    printf("Options:\n");
    printf("  -b,      --boot=TYPE    Boot type (bp, bpun, aout, floppy, smd)\n");
    printf("  -i,      --image=FILE   Image file to load\n");
    printf("  -s,      --start=ADDR   Start address (default: 0)\n");
    printf("  -a,      --disasm       Enable disassembly output\n");
    printf("  -d,      --debugger     Enable DAP debugger\n");
    printf("  -p PORT, --port=PORT    Set debugger port (default: 4711)\n");
    printf("  -S,      --smd-debug    Enable SMD disk controller debug log (stderr)\n");
    printf("  -t,      --trace        Enable CPU execution trace to stderr\n");
    printf("  -n N,    --max-instr=N  Stop after N instructions\n");
    printf("  -B ADDR, --breakpoint=ADDR  Stop at address (octal/hex/decimal)\n");
    printf("  -T ADDR, --text-start=ADDR  Text segment load address for a.out (default: 0)\n");
    printf("  -v,      --verbose      Enable verbose output\n");
    printf("  -P DIR,  --printdir=DIR  Printer output directory (default: ./prints/)\n");
    printf("  -D DIR,  --tapedir=DIR   Paper tape output directory (default: ./tapes/)\n");
    printf("  -e FILE, --tape=FILE     Paper tape reader input file (.bpun)\n");
    printf("  -N[PORT],--telnet[=PORT] Enable telnet server (default port: 9000)\n");
    printf("  -r TYPE, --printer=TYPE  Printer emulation: text (default), escp, laser\n");
    printf("  -f FMT,  --printformat=FMT  Output format: txt (default), pdf\n");
    printf("  -H CFG,  --hdlc=CFG     Enable HDLC controller\n");
    printf("                          Server mode: --hdlc=PORT\n");
    printf("                          Client mode: --hdlc=HOST:PORT\n");
    printf("  -h,      --help         Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --boot=bpun --image=test.bpun\n", progName);
    printf("  %s --boot=floppy --image=disk.img --start=0x1000 --disasm\n", progName);
    printf("  %s --debugger\n", progName);
    printf("  %s --hdlc=%d                    # HDLC server on port %d\n", progName, HDLC_DEFAULT_PORT, HDLC_DEFAULT_PORT);
    printf("  %s --hdlc=192.168.1.10:%d       # HDLC client to 192.168.1.10:%d\n", progName, HDLC_DEFAULT_PORT, HDLC_DEFAULT_PORT);
} 