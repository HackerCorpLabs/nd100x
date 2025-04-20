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


#ifndef DEVICE_PAPERTAPE_H
#define DEVICE_PAPERTAPE_H

// Paper tape registers
typedef enum {
    PAPERTAPE_READ_DATA_REGISTER = 0,    // 0400
    PAPERTAPE_WRITE_DATA_BUFFER = 1,     // 0401
    PAPERTAPE_READ_STATUS_REGISTER = 2,  // 0402
    PAPERTAPE_WRITE_CONTROL_WORD = 3     // 0403
} PaperTapeRegisters;

// Status register bits
typedef union {
    uint16_t raw;
    struct {
        uint16_t interruptEnabled : 1;    // Bit 0
        uint16_t notUsed1 : 1;            // Bit 1
        uint16_t readActive : 1;          // Bit 2
        uint16_t readyForTransfer : 1;    // Bit 3
        uint16_t notUsed4 : 1;            // Bit 4
        uint16_t notUsed5 : 1;            // Bit 5
        uint16_t notUsed6 : 1;            // Bit 6
        uint16_t notUsed7 : 1;            // Bit 7
        uint16_t notUsed8 : 1;            // Bit 8
        uint16_t notUsed9 : 1;            // Bit 9
        uint16_t notUsed10 : 1;           // Bit 10
        uint16_t notUsed11 : 1;           // Bit 11
        uint16_t notUsed12 : 1;           // Bit 12
        uint16_t notUsed13 : 1;           // Bit 13
        uint16_t notUsed14 : 1;           // Bit 14
        uint16_t notUsed15 : 1;           // Bit 15
    } bits;
} PaperTapeStatus;

// Control word bits
typedef union {
    uint16_t raw;
    struct {
        uint16_t interruptEnabled : 1;    // Bit 0
        uint16_t notUsed1 : 1;            // Bit 1
        uint16_t readActive : 1;          // Bit 2
        uint16_t readyForTransfer : 1;    // Bit 3
        uint16_t deviceClear : 1;         // Bit 4
        uint16_t notUsed5 : 1;            // Bit 5
        uint16_t notUsed6 : 1;            // Bit 6
        uint16_t notUsed7 : 1;            // Bit 7
        uint16_t notUsed8 : 1;            // Bit 8
        uint16_t notUsed9 : 1;            // Bit 9
        uint16_t notUsed10 : 1;           // Bit 10
        uint16_t notUsed11 : 1;           // Bit 11
        uint16_t notUsed12 : 1;           // Bit 12
        uint16_t notUsed13 : 1;           // Bit 13
        uint16_t notUsed14 : 1;           // Bit 14
        uint16_t notUsed15 : 1;           // Bit 15
    } bits;
} PaperTapeControl;

// Paper tape device data
typedef struct {
    FILE *papertapeFile;
    const char *papertapeName;
    uint8_t characterBuffer;
    PaperTapeStatus statusRegister;
    PaperTapeControl controlWord;
} PaperTapeData;

// Function declarations
Device* CreatePaperTapeDevice(uint8_t thumbwheel);

#endif /* DEVICE_PAPERTAPE_H */ 