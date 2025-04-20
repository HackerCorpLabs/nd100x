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



#ifndef DEVICE_RTC_H
#define DEVICE_RTC_H

// RTC registers
typedef enum {
    RTC_READ_DATA_REGISTER = 0,           // IOX 010: Returns 0

    RTC_CLEAR_COUNTER = 1,                // IOX 011: Clear real-time clock counter. This instruction will cause the next clock pulse to occur exactly 20 ms later.
                                          // If executed repeatedly, the counter will never increment and no clock pulses will occur.
                                          // This may affect operator console terminal communication.
    RTC_READ_STATUS = 2,                  // IOX 012: Read real-time clock status
    
    RTC_WRITE_CONTROL = 3                 // IOX 013: Set real-time clock control word    
} RTCRegister;

// Selected frequency for programmable clock
typedef enum {
    RTC_FREQ_STOP = 0,                    // Stop
    RTC_FREQ_100_USEC = 1,                // 100 uSec (1/1000 of time base)
    RTC_FREQ_10_USEC = 2,                 // 10 uSec (1/100 of time base)
    RTC_FREQ_1_USEC = 3                   // 1 uSec (1/10 of time base)
} RTCFrequency;


// Status register bit fields
typedef union {
    uint16_t raw;
    struct {
        uint16_t interruptEnabled : 1;    // Bit 0: Clock will interrupt on next pulse
        uint16_t unused1 : 1;             // Bit 1: Unused
        uint16_t externalHoldPulse : 1;   // Bit 2: External Hold Pulse received
        uint16_t readyForTransfer : 1;    // Bit 3: Clock pulse occurred, ready for transfer
        uint16_t unused4_15 : 12;         // Bits 4-15: Unused
    } bits;
} RTCStatusRegister;

// Control register bit fields
typedef union {
    uint16_t raw;
    struct {
        uint16_t interruptEnabled : 1;      // Bit 0: Enable interrupt if ready for transfer occurs
        uint16_t returnAddressOpcom : 1;    // Bit 1: ?? (Seen set under SIII boot) (RETSG, Return address on opcom segment) ?
        uint16_t unused2_9 : 8;             // Bits 2-9: Unused
        uint16_t frequency : 2;             // Bits 10-11: Select Frequency (==> enum RTCFrequency)
        uint16_t externalHoldEnable : 1;    // Bit 12: Enable External Hold if 1, Disable if 0
        uint16_t clearReadyForTransfer : 1; // Bit 13: Clear "Ready for Transfer" flag if 1
        uint16_t clearExternalHold : 1;     // Bit 14: Clear "external hold signal occurred" if 1
        uint16_t restartClock : 1;          // Bit 15: Restart clock (Preset to N-1) if 1
    } bits;
} RTCControlRegister;


// RTC device data structure
typedef struct {
    int rtcCounter;
    int divisionNumberN;
    uint16_t register1;
        
    bool externalHoldEnabled;
    bool externalHoldSignal;
    
    bool clockCountingStarted;

    RTCControlRegister controlRegister;
    RTCStatusRegister statusRegister;
    RTCFrequency selectedFrequency;
} RTCData;

// Function declarations
Device* CreateRTCDevice(uint8_t thumbwheel);

#endif // DEVICE_RTC_H 