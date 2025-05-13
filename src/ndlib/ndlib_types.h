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


#ifndef ND_LIB_TYPES_H
#define ND_LIB_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


// Physical memory functions in cpu_mms.c
extern int ReadPhysicalMemory(int physicalAddress, bool privileged);
extern void WritePhysicalMemory(int physicalAddress, uint16_t value, bool privileged);


// ** LOGGING **
// Log levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/// @brief Minimum log level for filtering messages
/// @details Messages below this level will not be logged.
extern LogLevel minLogLevel;


/// @brief Array of log level strings for formatting output 
extern const char *level_str[];

// ** BPUN **
typedef struct
{
    uint16_t start;              /* octal start address for the program */
    uint16_t boot;               /* octal value giving the start address of the bootstrap loader */
    uint16_t address;            /* Address where the binary load of the data will start */    
    uint16_t checksum;           /* Checksum value */
    uint16_t calculatedChecksum; /* Calculated checksum for verification */
    uint16_t action;             /* Action field - if zero, execution starts at start address */
    uint16_t count;              /* Number of 16-bit words in code stream */
    bool isFloMon;               /* Is this FloMon format (floppy-boot sector) */
} BPUN_Header;

typedef enum {
    LoadState_Preamble,
    LoadState_Address,
    LoadState_Count,
    LoadState_Data,
    LoadState_Checksum,
    LoadState_Action,
    LoadState_FloMonCount,
    LoadState_FloMonLoad
} LoadState;










#endif // 