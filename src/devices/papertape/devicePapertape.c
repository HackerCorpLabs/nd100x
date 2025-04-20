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

#include "../devices_types.h"
#include "../devices_protos.h"

#include "devicePapertape.h"

static void PaperTape_Reset(Device *self) {
    PaperTapeData *data = (PaperTapeData *)self->deviceData;
    if (data && data->papertapeFile) {
        rewind(data->papertapeFile);
    }
}

static uint16_t PaperTape_Tick(Device *self) {
    if (!self) return 0;
    Device_TickIODelay(self);
    return self->interruptBits;
}

static uint16_t PaperTape_Read(Device *self, uint32_t address) {
    if (!self) return 0;
    
    PaperTapeData *data = (PaperTapeData *)self->deviceData;
    uint16_t value = 0;
    uint32_t reg = Device_RegisterAddress(self, address);

    switch (reg) {
        case PAPERTAPE_READ_DATA_REGISTER:
            data->statusRegister.bits.readyForTransfer = 0;
            value = data->characterBuffer;
            break;

        case PAPERTAPE_READ_STATUS_REGISTER:
            value = data->statusRegister.raw;
            break;
    }

#ifdef DEBUG_PT
    printf("PaperTape Reading from address: %o value: %o\n", address, value);
#endif

    return value;
}

static void PaperTape_Write(Device *self, uint32_t address, uint16_t value) {
    if (!self) return;
    
    PaperTapeData *data = (PaperTapeData *)self->deviceData;
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_PT
    printf("PaperTape Writing value: %o to address: %o\n", value, address);
#endif

    switch (reg) {
        case PAPERTAPE_WRITE_DATA_BUFFER:
            // Only for PaperTapeWRITER
            break;

        case PAPERTAPE_WRITE_CONTROL_WORD:
            data->controlWord.raw = value;
            data->statusRegister.bits.interruptEnabled = data->controlWord.bits.interruptEnabled;
            data->statusRegister.bits.readActive = data->controlWord.bits.readActive;
            data->statusRegister.bits.readyForTransfer = data->controlWord.bits.readyForTransfer;

            if (data->controlWord.bits.deviceClear) {                
                data->statusRegister.bits.readActive = 0;
                data->statusRegister.bits.readyForTransfer = 0;
                data->characterBuffer = 0x00;
            }

            Device_SetInterruptStatus(self, 
                data->statusRegister.bits.interruptEnabled && 
                data->statusRegister.bits.readyForTransfer,
                self->interruptLevel);

            if (data->statusRegister.bits.readActive) {
                data->statusRegister.bits.readyForTransfer = 0;

                int w = -1;
                if (data->papertapeFile == NULL) {
                    printf("NO Papertape\n");
                } else {
                    w = getc(data->papertapeFile) & 0377;
                }

                if (w >= 0) {
                    data->characterBuffer = w;
                    data->statusRegister.bits.readyForTransfer = 1;
                } else {
                    //printf("Papertape EOF\n");
                }

                data->statusRegister.bits.readActive = 0;
            }

            Device_SetInterruptStatus(self, 
                data->statusRegister.bits.interruptEnabled && 
                data->statusRegister.bits.readyForTransfer,
                self->interruptLevel);
            break;
    }
}

static uint16_t PaperTape_Ident(Device *self, uint16_t level) {
    if (!self) return 0;
    
    //printf("PaperTape::IDENT called with level %d\n", level);
    if ((self->interruptBits & (1 << level)) != 0) {
        PaperTapeData *data = (PaperTapeData *)self->deviceData;
        data->statusRegister.bits.interruptEnabled = 0;
        Device_SetInterruptStatus(self, false, level);
        return self->identCode;
    }
    return 0;
}

Device* CreatePaperTapeDevice(uint8_t thumbwheel) {
    Device *dev = malloc(sizeof(Device));
    if (!dev) return NULL;

    PaperTapeData *data = malloc(sizeof(PaperTapeData));
    if (!data) {
        free(dev);
        return NULL;
    }

    // Initialize device base structure
    Device_Init(dev, thumbwheel);

    // Set up device-specific data
    data->papertapeFile = NULL;
    data->papertapeName = "test.bpun"; // "INSTRUCTION-B.BPUN";
    data->characterBuffer = 0;
    data->statusRegister.raw = 0;
    data->controlWord.raw = 0;

    // Set up device address and interrupt settings based on thumbwheel
    switch (thumbwheel) {
        case 0:
            strcpy(dev->memoryName, "PAPER TAPE 0");
            dev->interruptLevel = 12;
            dev->identCode = 02;  // octal 02
            dev->startAddress = 0400;
            dev->endAddress = 0403;
            break;
        case 1:
            strcpy(dev->memoryName, "PAPER TAPE 1");
            dev->interruptLevel = 12;
            dev->identCode = 022;  // Octal 22
            dev->startAddress = 0404;
            dev->endAddress = 0407;
            break;
        default:
            free(data);
            free(dev);
            return NULL;
    }

    // Open paper tape file
    if ((data->papertapeFile = fopen(data->papertapeName, "r")) == NULL) {
        printf("Unable to open file %s\n", data->papertapeName);
        //free(data);
        //free(dev);
        //return NULL;
    }

    // Set up device function pointers
    dev->Reset = PaperTape_Reset;
    dev->Tick = PaperTape_Tick;
    dev->Read = PaperTape_Read;
    dev->Write = PaperTape_Write;
    dev->Ident = PaperTape_Ident;
    dev->deviceData = data;

    printf("PaperTape object created.\n");
    return dev;
} 