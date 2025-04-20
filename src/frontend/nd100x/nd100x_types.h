
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

#ifndef ND100X_TYPES_H
#define ND100X_TYPES_H

#include <stdbool.h>
#include <stdint.h>


#include "../../machine/machine_types.h"
#include "../ndlib/ndlib_types.h"

// Configuration structure
typedef struct {
    BOOT_TYPE bootType;
    char *imageFile;
    uint32_t startAddress;
    bool disasmEnabled;
    bool verbose;
    bool showHelp;
} Config_t;

#endif // CONFIG_H 