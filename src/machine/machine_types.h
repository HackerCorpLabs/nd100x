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

#endif
