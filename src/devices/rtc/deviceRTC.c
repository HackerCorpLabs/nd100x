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


//#define DEBUG_RTC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../devices_types.h"
#include "../devices_protos.h"

#include "deviceRTC.h"

#define TICKS_20MS 10550 // Ticks for 20ms timer (adjusted for stability)
//#define DEBUG_RTC
//#define DEBUG_RTC_TICK

static void RTC_Reset(Device *self) {
    RTCData *data = (RTCData *)self->deviceData;
    if (!data) return;

    // Clear all registers and status    
    data->rtcCounter = 0;
    data->divisionNumberN = TICKS_20MS;
    data->register1 = 0;
    
    data->statusRegister.raw = 0;
    data->controlRegister.raw = 0;
}

static void RTC_ClearClockTicks(Device *self) {
    RTCData *data = (RTCData *)self->deviceData;
    if (!data) return;

    data->rtcCounter = data->divisionNumberN;

#ifdef DEBUG_RTC
//    printf("RTC_ClearClockTicks: %d\n", data->rtcCounter);
#endif
}

static uint16_t RTC_Tick(Device *self) {
    if (!self) return 0;
    
    RTCData *data = (RTCData *)self->deviceData;
    if (!data) return 0;

#ifdef DEBUG_RTC_TICK
    printf("RTC Tick %d\n", data->rtcCounter);
#endif 


    // Process I/O delays
    Device_TickIODelay(self);

    // Count down the timer
    data->rtcCounter--;

    if (data->rtcCounter <= 0) {        
        if (!data->statusRegister.bits.readyForTransfer)
        {
            data->statusRegister.bits.readyForTransfer = true;
            if (data->statusRegister.bits.interruptEnabled) {
#ifdef DEBUG_RTC
                printf("RTC Setting interrupt status to true\n");
#endif                
                Device_SetInterruptStatus(self, true, self->interruptLevel);
            }
        }
        RTC_ClearClockTicks(self);
    }

    return self->interruptBits;
}

static uint16_t RTC_Read(Device *self, uint32_t address) {
    if (!self) return 0;
    
    RTCData *data = (RTCData *)self->deviceData;
    uint16_t value = 0;
    uint32_t reg = Device_RegisterAddress(self, address);

    switch (reg) {
        case RTC_READ_DATA_REGISTER:
            value = (uint16_t)data->rtcCounter;
            break;

        case RTC_READ_STATUS:
            value = data->statusRegister.raw;
            break;

        default:
            break;
    }

#ifdef DEBUG_RTC
    printf("RTC Reading from address: %o value: %o\n", address, value);
#endif


    return value;
}

static void RTC_Write(Device *self, uint32_t address, uint16_t value) {
    if (!self) return;
    
    RTCData *data = (RTCData *)self->deviceData;
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_RTC
    printf("RTC Writing value: %o to address: %o\n", value, address);
#endif

    switch (reg) {
        case RTC_CLEAR_COUNTER:
            RTC_ClearClockTicks(self);
            data->statusRegister.bits.readyForTransfer = false;
            Device_SetInterruptStatus(self, false, self->interruptLevel);
            break;

        case RTC_WRITE_CONTROL:
            data->controlRegister.raw = value;

            // Update status register
            data->statusRegister.bits.interruptEnabled = data->controlRegister.bits.interruptEnabled;            
            
            // Handle interrupt enable/disable            
            if (!data->statusRegister.bits.interruptEnabled) {
                Device_SetInterruptStatus(self, false, self->interruptLevel);
            }


            // Clear ready for transfer if requested and clear interrupt bit 13 (Needed for testprogram TPE Monitor version B)
            if (data->controlRegister.bits.clearReadyForTransfer) {
                data->statusRegister.bits.readyForTransfer = 0;
                self->interruptBits &= ~(1 << 13);
            }

            // Clear external hold signal if requested
            if (data->controlRegister.bits.clearExternalHold) {
                data->statusRegister.bits.externalHoldPulse = 0;
            }

            // Restart clock if requested
            if (data->controlRegister.bits.restartClock) {            
                data->rtcCounter = data->divisionNumberN;
                data->clockCountingStarted = true;
            }
            break;

        default:
            break;
    }
}

static uint16_t RTC_Ident(Device *self, uint16_t level) {
    if (!self) return 0;
    
    RTCData *data = (RTCData *)self->deviceData;
    if (!data) return 0;

    if ((self->interruptBits & (1 << level)) != 0) {
        RTC_ClearClockTicks(self);
        data->statusRegister.bits.interruptEnabled = false;

#ifdef DEBUG_RTC
        printf("RTC_Ident: %d\n", self->identCode);
#endif       
        Device_SetInterruptStatus(self, false, level);
        return self->identCode;
    }
#ifdef DEBUG_RTC    
    else
    {
        printf("RTC_Ident: interrupt not set\n");
    }
#endif       
    return 0;
}

Device* CreateRTCDevice(uint8_t thumbwheel) {
    Device *dev = malloc(sizeof(Device));
    if (!dev) return NULL;

    RTCData *data = malloc(sizeof(RTCData));
    if (!data) {
        free(dev);
        return NULL;
    }

    // Initialize device base structure
    Device_Init(dev, thumbwheel);

    // Set up device-specific data
    memset(data, 0, sizeof(RTCData));

    // Set up device properties based on thumbwheel
    switch (thumbwheel) {
        case 0:
            dev->identCode = 01;
            dev->startAddress = 010;
            dev->endAddress = 013;
            dev->interruptLevel = 13;
            dev->isRTC = 1;
            strcpy(dev->memoryName, "RTC 1");
            break;
        case 1:
            dev->identCode = 02;
            dev->startAddress = 014;
            dev->endAddress = 017;
            dev->interruptLevel = 13;
            strcpy(dev->memoryName, "RTC 2");
            break;
        case 2:
            dev->identCode = 06;
            dev->startAddress = 020;
            dev->endAddress = 023;
            dev->interruptLevel = 13;
            strcpy(dev->memoryName, "RTC 3");
            break;
        default:
            printf("Unexpected thumbwheel code %d\n", thumbwheel);
            free(data);
            free(dev);
            return NULL;
    }

    // Set up device function pointers
    dev->Reset = RTC_Reset;
    dev->Tick = RTC_Tick;
    dev->Read = RTC_Read;
    dev->Write = RTC_Write;
    dev->Ident = RTC_Ident;
    dev->deviceData = data;

    printf("RTC device created: %s\n", dev->memoryName);
    return dev;
} 