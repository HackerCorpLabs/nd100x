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
#include "../../cpu/cpu_protos.h"

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
    {"bsd-debug",  no_argument,       0, 'G'},
    {"trace",      no_argument,       0, 't'},
    {"max-instr",  required_argument, 0, 'n'},
    {"breakpoint", required_argument, 0, 'B'},
    {"watch",      required_argument, 0, 'W'},
    {"text-start", required_argument, 0, 'T'},
    {"printdir",   required_argument, 0, 'P'},
    {"tapedir",    required_argument, 0, 'D'},
    {"tape",       required_argument, 0, 'e'},
    {"telnet",     optional_argument, 0, 'N'},
    {"printer",    required_argument, 0, 'r'},
    {"printformat",required_argument, 0, 'f'},
    {"charset",    required_argument, 0, 'L'},
    {"hdlc",       required_argument, 0, 'H'},
    {"throttle",   optional_argument, 0, 'Z'},
    {"ring-dump",  optional_argument, 0, 'R'},
    {"overlay-deposit", no_argument, 0, 'O'},
    {"watch-skip", required_argument, 0, 0x130},
    {"watch-min-value", required_argument, 0, 0x131},
    {"smd0",       required_argument, 0, 0x100},
    {"smd1",       required_argument, 0, 0x101},
    {"smd2",       required_argument, 0, 0x102},
    {"smd3",       required_argument, 0, 0x103},
    {"scsi0",      required_argument, 0, 0x110},
    {"scsi1",      required_argument, 0, 0x111},
    {"scsi2",      required_argument, 0, 0x112},
    {"scsi3",      required_argument, 0, 0x113},
    {"scsi4",      required_argument, 0, 0x114},
    {"scsi5",      required_argument, 0, 0x115},
    {"scsi6",      required_argument, 0, 0x116},
    {"scsi-debug", no_argument,       0, 0x117},
    {"config",     required_argument, 0, 0x120},
    {"ini",        required_argument, 0, 0x120},
    {"show-config",no_argument,       0, 0x121},
    {"write-config",required_argument,0, 0x122},
    {0, 0, 0, 0}
};

void Config_Init(Config_t *config) {
    if (!config) return;
    
    config->bootType = BOOT_NONE;
    config->bootUnit = 0;
    config->iniFile = NULL;
    config->showConfig = false;
    config->writeConfig = NULL;
    config->imageFile = NULL;
    config->startAddress = 0;
    config->disasmEnabled = false;
    config->verbose = false;
    config->showHelp = false;
    config->debuggerEnabled = false;
    config->debuggerPort = 4711;
    config->smdDebug = false;
    config->bsdDebug = false;
    config->traceEnabled = false;
    config->maxInstructions = 0;
    config->breakpointEnabled = false;
    config->breakpointAddr = 0;
    config->textStartSet = false;
    config->textStart = 0;
    config->overlayDeposit = false;
    config->printDir = NULL;
    config->tapeDir = NULL;
    config->tapeFile = NULL;
    for (int i = 0; i < 4; i++) config->smdFile[i] = NULL;
    config->scsiEnabled = false;
    config->scsiDebug = false;
    for (int i = 0; i < SCSI_MAX_UNITS; i++) {
        config->scsiFile[i] = NULL;
        config->scsiType[i] = SCSI_UNIT_NONE;
    }
    config->telnetEnabled = false;
    config->telnetPort = 9000;
    config->watchCount = 0;
    config->printerType = PRINTER_TEXT;
    config->printFormat = PRINT_FORMAT_TXT;
    config->charset = CHARSET_OFF;
    // HDLC configuration
    config->hdlcCount = 0;
    for (int i = 0; i < MAX_HDLC_DEVICES; i++) {
        config->hdlc[i].deviceNum = 0;
        config->hdlc[i].isServer = false;
        config->hdlc[i].address = NULL;
        config->hdlc[i].port = HDLC_DEFAULT_PORT;
    }
}

/* Parse a --boot argument into bootType + bootUnit.
 * Accepts the bare names (bp, bpun, aout, floppy, smd, scsi) plus an optional
 * unit digit on the disk controllers: smd0-smd3 and scsi0-scsi6. A bare
 * "smd"/"scsi" means unit 0. Prints its own error message and returns false
 * on an unknown name or an out-of-range unit. */
static bool parseBootSpec(Config_t *config, const char *bootStr) {
    if (!bootStr || !config) return false;

    config->bootUnit = 0;

    if (strcmp("bp", bootStr) == 0)     { config->bootType = BOOT_BP;     return true; }
    if (strcmp("bpun", bootStr) == 0)   { config->bootType = BOOT_BPUN;   return true; }
    if (strcmp("aout", bootStr) == 0)   { config->bootType = BOOT_AOUT;   return true; }
    if (strcmp("floppy", bootStr) == 0) { config->bootType = BOOT_FLOPPY; return true; }

    if (strncmp("smd", bootStr, 3) == 0) {
        const char *u = bootStr + 3;
        if (*u == '\0') { config->bootType = BOOT_SMD; return true; }
        if (u[0] >= '0' && u[0] <= '3' && u[1] == '\0') {
            config->bootType = BOOT_SMD;
            config->bootUnit = u[0] - '0';
            return true;
        }
        fprintf(stderr, "Invalid SMD boot unit in '%s' (use smd or smd0-smd3)\n", bootStr);
        return false;
    }

    if (strncmp("scsi", bootStr, 4) == 0) {
        const char *u = bootStr + 4;
        if (*u == '\0') { config->bootType = BOOT_SCSI; return true; }
        if (u[0] >= '0' && u[0] <= '6' && u[1] == '\0') {
            config->bootType = BOOT_SCSI;
            config->bootUnit = u[0] - '0';
            return true;
        }
        fprintf(stderr, "Invalid SCSI boot unit in '%s' (use scsi or scsi0-scsi6; ID 7 is the controller)\n", bootStr);
        return false;
    }

    fprintf(stderr, "Invalid boot type: %s\n", bootStr);
    return false;
}

// Parse HDLC config: "N:PORT" (server) or "N:HOST:PORT" (client)
// N = device number 1-4
static bool parseHDLCConfig(Config_t *config, const char *hdlcStr) {
    if (!hdlcStr || !config) return false;
    if (config->hdlcCount >= MAX_HDLC_DEVICES) {
        fprintf(stderr, "Too many HDLC devices (max %d)\n", MAX_HDLC_DEVICES);
        return false;
    }

    char *str = strdup(hdlcStr);
    if (!str) return false;

    // First token: device number
    char *first_colon = strchr(str, ':');
    if (!first_colon) {
        fprintf(stderr, "HDLC config must start with device number: N:PORT or N:HOST:PORT\n");
        free(str);
        return false;
    }

    *first_colon = '\0';
    char *endptr;
    int devNum = (int)strtol(str, &endptr, 10);
    if (*endptr != '\0' || devNum < 1 || devNum > 4) {
        fprintf(stderr, "HDLC device number must be 1-4, got: %s\n", str);
        free(str);
        return false;
    }

    // Check for duplicate device number
    for (int i = 0; i < config->hdlcCount; i++) {
        if (config->hdlc[i].deviceNum == devNum) {
            fprintf(stderr, "HDLC device %d already configured\n", devNum);
            free(str);
            return false;
        }
    }

    char *rest = first_colon + 1;
    char *second_colon = strchr(rest, ':');

    int idx = config->hdlcCount;
    config->hdlc[idx].deviceNum = devNum;

    if (second_colon) {
        // Client mode: "HOST:PORT"
        *second_colon = '\0';
        char *portStr = second_colon + 1;

        config->hdlc[idx].address = strdup(rest);
        if (!config->hdlc[idx].address) {
            free(str);
            return false;
        }

        config->hdlc[idx].port = (int)strtol(portStr, &endptr, 10);
        if (*endptr != '\0' || config->hdlc[idx].port <= 0 || config->hdlc[idx].port > 65535) {
            free(config->hdlc[idx].address);
            config->hdlc[idx].address = NULL;
            free(str);
            return false;
        }
        config->hdlc[idx].isServer = false;
    } else {
        // Server mode: just PORT
        config->hdlc[idx].port = (int)strtol(rest, &endptr, 10);
        if (*endptr != '\0' || config->hdlc[idx].port <= 0 || config->hdlc[idx].port > 65535) {
            free(str);
            return false;
        }
        config->hdlc[idx].address = NULL;
        config->hdlc[idx].isServer = true;
    }

    config->hdlcCount++;
    free(str);
    return true;
}

// Parse watchpoint config: "[phys:]ADDR[:r|w|rw]"
// ADDR accepts octal (leading 0), hex (0x), or decimal, matching -B.
static bool parseWatchConfig(Config_t *config, const char *watchStr) {
    if (!watchStr || !config) return false;
    if (config->watchCount >= MAX_CLI_WATCHPOINTS) {
        fprintf(stderr, "Too many watchpoints (max %d)\n", MAX_CLI_WATCHPOINTS);
        return false;
    }

    char *str = strdup(watchStr);
    if (!str) return false;

    char *p = str;
    bool isPhys = false;
    if (strncmp(p, "phys:", 5) == 0) {
        isPhys = true;
        p += 5;
    }

    int type = 3; // default READWRITE
    char *colon = strrchr(p, ':');
    if (colon) {
        char *t = colon + 1;
        if (strcmp(t, "r") == 0) type = 1;
        else if (strcmp(t, "w") == 0) type = 2;
        else if (strcmp(t, "rw") == 0 || strcmp(t, "wr") == 0) type = 3;
        else {
            fprintf(stderr, "Invalid watch access type: %s (use r, w, or rw)\n", t);
            free(str);
            return false;
        }
        *colon = '\0';
    }

    char *endptr;
    uint32_t addr = (uint32_t)strtoul(p, &endptr, 0);
    if (p == endptr || *endptr != '\0') {
        fprintf(stderr, "Invalid watch address: %s\n", p);
        free(str);
        return false;
    }

    int idx = config->watchCount;
    config->watch[idx].isPhysical = isPhys;
    config->watch[idx].address = addr;
    config->watch[idx].type = type;
    config->watchCount++;
    free(str);
    return true;
}

bool Config_ParseCommandLine(Config_t *config, int argc, char *argv[]) {
    int option_index = 0;
    int c;
    char *endptr;
    
    while ((c = getopt_long(argc, argv, "b:i:s:avhdp:StGn:B:W:T:P:D:e:N::r:f:L:H:Z::R::O",
                           long_options, &option_index)) != -1) {
        switch (c) {
            case 'b':
                if (!parseBootSpec(config, optarg)) {
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

            case 'L': {
                CharsetVariant cs;
                if (!charset_from_name(optarg, &cs)) {
                    fprintf(stderr, "Invalid charset: %s (use off, norwegian, swedish, german)\n", optarg);
                    return false;
                }
                config->charset = cs;
                break;
            }

            case 'h':
                config->showHelp = true;
                return true;

            case 'H':
                if (!parseHDLCConfig(config, optarg)) {
                    fprintf(stderr, "Invalid HDLC configuration: %s\n", optarg);
                    return false;
                }
                break;

            case 'Z':
                cpu_throttle_set_enabled(true);
                if (optarg) {
                    double mhz = atof(optarg);
                    if (mhz > 0) cpu_throttle_set_mhz(mhz);
                }
                break;

            case 'R': {
                int n = 50; // default
                if (optarg) {
                    n = (int)strtol(optarg, &endptr, 0);
                    if (*endptr != '\0' || n < 1 || n > 65536) {
                        fprintf(stderr, "Invalid ring-dump size (1-65536): %s\n", optarg);
                        return false;
                    }
                }
                config->ringDumpSize = n;
                break;
            }

            case 'S':
                config->smdDebug = true;
                break;

            case 'G':
                config->bsdDebug = true;
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

            case 'W':
                if (!parseWatchConfig(config, optarg)) {
                    return false;
                }
                break;

            case 0x130:  /* --watch-skip N */
                config->watchSkip = (int)strtol(optarg, &endptr, 0);
                if (*endptr != '\0' || config->watchSkip < 0) {
                    fprintf(stderr, "Invalid --watch-skip value: %s\n", optarg);
                    return false;
                }
                break;

            case 0x131:  /* --watch-min-value V */
                config->watchMinValue = (int)strtoul(optarg, &endptr, 0);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid --watch-min-value: %s\n", optarg);
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

            case 'O':
                config->overlayDeposit = true;
                break;

            case 0x100: case 0x101: case 0x102: case 0x103: {
                int unit = c - 0x100;
                config->smdFile[unit] = strdup(optarg);
                if (!config->smdFile[unit]) {
                    fprintf(stderr, "Failed to allocate memory for SMD%d file\n", unit);
                    return false;
                }
                break;
            }

            case 0x110: case 0x111: case 0x112: case 0x113:
            case 0x114: case 0x115: case 0x116: {
                /* --scsiN=[TYPE:]FILE  e.g. --scsi0=hdd:/path/disk.img
                 * TYPE is optional and defaults to hdd. The type prefix is only
                 * honoured when the text before the first ':' is a known type
                 * name, so a bare path (including a Windows "C:\..." path) is
                 * still treated as a filename. */
                int unit = c - 0x110;
                SCSIUnitType type = SCSI_UNIT_HDD;
                const char *file = optarg;

                const char *colon = strchr(optarg, ':');
                const char *slash = strchr(optarg, '/');
                /* A colon only introduces a type when it comes before any '/',
                 * so "/tmp/a:b.img" stays a filename. If the text there is not
                 * a known type it is a typo, not a path - say so rather than
                 * silently trying to open a file named "hdX:...". */
                if (colon && colon != optarg && (!slash || colon < slash)) {
                    size_t len = (size_t)(colon - optarg);
                    char prefix[16];
                    if (len >= sizeof(prefix)) {
                        fprintf(stderr, "Error: --scsi%d has an unknown type prefix in '%s'\n", unit, optarg);
                        return false;
                    }
                    memcpy(prefix, optarg, len);
                    prefix[len] = '\0';
                    SCSIUnitType parsed = SCSI_ParseUnitType(prefix);
                    if (parsed == SCSI_UNIT_NONE) {
                        fprintf(stderr, "Error: --scsi%d unknown type '%s' "
                                        "(expected hdd, tape, cdrom or floppy)\n", unit, prefix);
                        return false;
                    }
                    type = parsed;
                    file = colon + 1;
                }

                if (*file == '\0') {
                    fprintf(stderr, "Error: --scsi%d needs a file (got '%s')\n", unit, optarg);
                    return false;
                }

                config->scsiFile[unit] = strdup(file);
                if (!config->scsiFile[unit]) {
                    fprintf(stderr, "Failed to allocate memory for SCSI%d file\n", unit);
                    return false;
                }
                config->scsiType[unit] = type;
                config->scsiEnabled = true;
                break;
            }

            case 0x117:
                config->scsiDebug = true;
                break;

            case 0x120: /* --config / --ini */
                config->iniFile = strdup(optarg);
                if (!config->iniFile) {
                    fprintf(stderr, "Failed to allocate memory for config file path\n");
                    return false;
                }
                break;

            case 0x121: /* --show-config */
                config->showConfig = true;
                break;

            case 0x122: /* --write-config=FILE */
                config->writeConfig = strdup(optarg);
                if (!config->writeConfig) {
                    fprintf(stderr, "Failed to allocate memory for write-config path\n");
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
    if ((!config->showHelp && !config->showConfig && !config->writeConfig && !config->iniFile && !config->debuggerEnabled)) {
        if (config->bootType == BOOT_NONE) {
            config->bootType = BOOT_SMD;

            fprintf(stderr, "Boot type must be specified\n");
            return false;
        }
        // --image is only for aout, bpun, bp, and floppy boot types
        if (config->imageFile && config->bootType == BOOT_SMD) {
            fprintf(stderr, "Error: --image is not used with --boot=smd. Use --smd0..--smd3 instead.\n");
            return false;
        }
        if (config->imageFile && config->bootType == BOOT_SCSI) {
            fprintf(stderr, "Error: --image is not used with --boot=scsi. Use --scsi0..--scsi6 instead.\n");
            return false;
        }
        if (config->bootType == BOOT_SCSI) {
            int u = config->bootUnit;
            if (!config->scsiFile[u]) {
                fprintf(stderr, "Error: --boot=scsi%d needs a boot image on SCSI ID %d. Use --scsi%d=hdd:FILE.\n", u, u, u);
                return false;
            }
            if (config->scsiType[u] != SCSI_UNIT_HDD) {
                fprintf(stderr, "Error: --boot=scsi%d needs a 'hdd' target on SCSI ID %d (it is '%s').\n",
                        u, u, SCSI_UnitTypeName(config->scsiType[u]));
                return false;
            }
        }
        if (!config->imageFile) {
            if (config->bootType == BOOT_FLOPPY) {
                config->imageFile = strdup("FLOPPY.IMG");
            } else
            // SMD and SCSI take their images from --smdN / --scsiN, not --image.
            if (config->bootType != BOOT_SMD && config->bootType != BOOT_SCSI) {
                fprintf(stderr, "Image file must be specified\n");
                return false;
            }
        }
    }
    
   if (config->verbose) {
        printf("Configuration:\n");
        if (config->iniFile && config->bootType == BOOT_NONE) {
            // Boot device comes from the INI, resolved after this summary prints.
            printf("  Boot type: (from config file %s)\n", config->iniFile);
        } else if (config->bootType == BOOT_SMD || config->bootType == BOOT_SCSI) {
            printf("  Boot type: %s unit %d\n", boot_type_str[config->bootType], config->bootUnit);
        } else {
            printf("  Boot type: %s\n", boot_type_str[config->bootType]);
        }
        printf("  Image file: %s\n", config->imageFile);
        for (int i = 0; i < 4; i++) {
            if (config->smdFile[i])
                printf("  SMD%d image: %s\n", i, config->smdFile[i]);
        }
        printf("  Start address: 0x%x\n", config->startAddress);
        printf("  Disassembly: %s\n", config->disasmEnabled ? "enabled" : "disabled");
        for (int i = 0; i < config->hdlcCount; i++) {
            if (config->hdlc[i].isServer) {
                printf("  HDLC %d: Server mode on port %d\n",
                       config->hdlc[i].deviceNum, config->hdlc[i].port);
            } else {
                printf("  HDLC %d: Client mode to %s:%d\n",
                       config->hdlc[i].deviceNum,
                       config->hdlc[i].address, config->hdlc[i].port);
            }
        }
    }
    

    return true;
}

void Config_PrintHelp(const char *progName) {
    printf("Usage: %s [options]\n\n", progName);
    printf("Options:\n");
    printf("  -b,      --boot=TYPE    Boot type (bp, bpun, aout, floppy, smd[0-3], scsi[0-6])\n");
    printf("                          smd/scsi take an optional boot unit digit,\n");
    printf("                          e.g. --boot=smd1 or --boot=scsi2 (default: unit 0)\n");
    printf("  -i,      --image=FILE   Image file to load (aout, bpun, floppy only)\n");
    printf("           --smd0=FILE    SMD unit 0 disk image (default: SMD0.IMG)\n");
    printf("           --smd1=FILE    SMD unit 1 disk image (default: SMD1.IMG)\n");
    printf("           --smd2=FILE    SMD unit 2 disk image (default: SMD2.IMG)\n");
    printf("           --smd3=FILE    SMD unit 3 disk image (default: SMD3.IMG)\n");
    printf("           --scsi0=[TYPE:]FILE  SCSI ID 0 target image (adds the ND-3201 controller)\n");
    printf("           --scsi1=[TYPE:]FILE  SCSI ID 1 target image\n");
    printf("           --scsi2=[TYPE:]FILE  SCSI ID 2 target image\n");
    printf("           --scsi3=[TYPE:]FILE  SCSI ID 3 target image\n");
    printf("           --scsi4=[TYPE:]FILE  SCSI ID 4 target image\n");
    printf("           --scsi5=[TYPE:]FILE  SCSI ID 5 target image\n");
    printf("           --scsi6=[TYPE:]FILE  SCSI ID 6 target image\n");
    printf("                          TYPE is one of:\n");
    printf("                            hdd     Micropolis 1375-ND hard disk (default)\n");
    printf("                            tape    streamer tape           (not implemented yet)\n");
    printf("                            cdrom   CD-ROM                  (not implemented yet)\n");
    printf("                            floppy  SCSI floppy             (not implemented yet)\n");
    printf("                          SCSI ID 7 is the controller itself and cannot be a target.\n");
    printf("                          Example: --scsi0=hdd:SCSI-K.image\n");
    printf("  -s,      --start=ADDR   Start address (default: 0)\n");
    printf("  -a,      --disasm       Enable disassembly output\n");
    printf("  -d,      --debugger     Enable DAP debugger\n");
    printf("  -p PORT, --port=PORT    Set debugger port (default: 4711)\n");
    printf("  -S,      --smd-debug    Enable SMD disk controller debug log (stderr)\n");
    printf("           --scsi-debug   Enable SCSI disk controller debug log (stderr)\n");
    printf("           --bsd-debug    Track BSD kernel-stack high-water (KSTKHW, stderr)\n");
    printf("  -t,      --trace        Enable CPU execution trace to stderr\n");
    printf("  -n N,    --max-instr=N  Stop after N instructions\n");
    printf("  -B ADDR, --breakpoint=ADDR  Stop at address (octal/hex/decimal)\n");
    printf("  -W SPEC, --watch=SPEC   Stop on memory access at full speed (repeatable, max %d)\n", MAX_CLI_WATCHPOINTS);
    printf("                          SPEC = [phys:]ADDR[:r|w|rw]  (default rw, virtual)\n");
    printf("  -T ADDR, --text-start=ADDR  Text segment load address for a.out (default: 0)\n");
    printf("  -v,      --verbose      Enable verbose output\n");
    printf("  -P DIR,  --printdir=DIR  Printer output directory (default: ./prints/)\n");
    printf("  -D DIR,  --tapedir=DIR   Paper tape output directory (default: ./tapes/)\n");
    printf("  -e FILE, --tape=FILE     Paper tape reader input file (.bpun)\n");
    printf("  -N[PORT],--telnet[=PORT] Enable telnet server (default port: 9000)\n");
    printf("  -r TYPE, --printer=TYPE  Printer emulation: text (default), escp, laser\n");
    printf("  -f FMT,  --printformat=FMT  Output format: txt (default), pdf\n");
    printf("  -L CS,   --charset=CS    Local-console national 7-bit charset (telnet/TCP unaffected):\n");
    printf("                          off (default), norwegian, swedish, german\n");
    printf("  -H CFG,  --hdlc=CFG     Enable HDLC controller (up to 4x)\n");
    printf("                          Server: --hdlc=N:PORT  (N=1-4)\n");
    printf("                          Client: --hdlc=N:HOST:PORT\n");
    printf("  -O,      --overlay-deposit Deposit data_click at phys word 1 for kernel boot-info\n");
    printf("  -R[N],   --ring-dump[=N]  Dump last N instructions on halt/crash (default: 50, max: 65536)\n");
    printf("  -Z[MHZ], --throttle[=MHZ] Throttle CPU to real-time speed (default: 0.5275 MHz)\n");
    printf("           --config=FILE  Machine configuration INI file\n");
    printf("           --ini=FILE     Alias for --config\n");
    printf("                          (default: autoload <binaryname>.ini in the current dir)\n");
    printf("           --show-config  Resolve+validate the machine config, print it, and exit\n");
    printf("           --write-config=FILE  Write the resolved machine config to an INI file and exit\n");
    printf("  -h,      --help         Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --boot=bpun --image=test.bpun\n", progName);
    printf("  %s --boot=floppy --image=disk.img --start=0x1000 --disasm\n", progName);
    printf("  %s --debugger\n", progName);
    printf("  %s --hdlc=1:%d                  # HDLC 1 server on port %d\n", progName, HDLC_DEFAULT_PORT, HDLC_DEFAULT_PORT);
    printf("  %s --hdlc=1:192.168.1.10:%d     # HDLC 1 client\n", progName, HDLC_DEFAULT_PORT);
    printf("  %s --boot=smd --smd0=myboot.img --smd1=data.img\n", progName);
    printf("  %s --boot=smd1                  # Boot from SMD unit 1\n", progName);
    printf("  %s --boot=scsi0 --scsi0=hdd:SCSI-K.image  # Boot from SCSI ID 0\n", progName);
    printf("  %s --hdlc=1:5000 --hdlc=2:5001  # Two HDLC devices\n", progName);
} 