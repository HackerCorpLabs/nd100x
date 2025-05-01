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

// Long options
static struct option long_options[] = {
    {"boot",    required_argument, 0, 'b'},
    {"image",   required_argument, 0, 'i'},
    {"start",   required_argument, 0, 's'},
    {"disasm",  no_argument,       0, 'a'},
    {"verbose", no_argument,       0, 'v'},
    {"help",    no_argument,       0, 'h'},
    {"debugger", no_argument,      0, 'd'},
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

bool Config_ParseCommandLine(Config_t *config, int argc, char *argv[]) {
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "b:i:s:dvh",
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
                char *endptr;
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

            case 'v':
                config->verbose = true;
                break;
                
            case 'h':
                config->showHelp = true;
                return true;
                
            case '?':
                return false;
                
            default:
                fprintf(stderr, "Unknown option: %c\n", c);
                return false;
        }
    }
    
    // Check required arguments
    if (!config->showHelp) {
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
    }
    

    return true;
}

void Config_PrintHelp(const char *progName) {
    printf("Usage: %s [options]\n\n", progName);
    printf("Options:\n");
    printf("  -b, --boot=TYPE    Boot type (bp, bpun, aout, floppy, smd)\n");
    printf("  -i, --image=FILE   Image file to load\n");
    printf("  -s, --start=ADDR   Start address (default: 0)\n");
    printf("  -a, --disasm       Enable disassembly output\n");
    printf("  -d, --debugger     Enable DAP debugger\n");
    printf("  -v, --verbose      Enable verbose output\n");
    printf("  -h, --help         Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --boot=bpun --image=test.bpun\n", progName);
    printf("  %s --boot=floppy --image=disk.img --start=0x1000 --disasm\n", progName);
} 