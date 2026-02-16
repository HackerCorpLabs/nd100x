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

 
#ifndef MACHINE_TYPES_H
#define MACHINE_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "../cpu/cpu_types.h"
#include "../cpu/cpu_protos.h"
#include "../devices/devices_types.h"


typedef enum {    
    BOOT_NONE = 0,
    BOOT_BPUN, 
    BOOT_AOUT,
    BOOT_BP,
    BOOT_FLOPPY, 
    BOOT_SMD
} BOOT_TYPE;

extern const char* boot_type_str[];


// Drive types
typedef enum {
    DRIVE_SMD,
    DRIVE_FLOPPY
} DRIVE_TYPE;

// Mounted drive information structure
typedef struct {
    char md5[33];
    char name[256];
    char description[1024];
    char image_path[1024];  // Path or URL of the image
    bool is_mounted;        // Is the drive mounted
    bool is_remote;         // true if downloaded from HTTP, false if local file
    bool is_writeprotected; // true if the drive is write-protected
    bool is_opfs;           // true if using OPFS (block I/O via JS, no FILE*)
    
    union {
        FILE* local_file;   // File handle for local files
        char* remote_data;  // Downloaded data for remote files
    } data;
    size_t data_size;       // Size of the data in bytes
    int block_size;         // Block size for this drive (256, 512, 1024 bytes)
} MountedDriveInfo_t;

#endif
