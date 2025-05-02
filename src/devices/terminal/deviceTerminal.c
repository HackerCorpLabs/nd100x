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

#include "deviceTerminal.h"

// Device definitions array
static const DeviceDefinition deviceDefinitions[] = {
    // Group 1 (starts at offset 0)
    {0300, 01, 01, "CONSOLE TERMINAL - TERMINAL 1"},
    {0310, 05, 11, "TERMINAL 2/ TET15"},
    {0320, 06, 42, "TERMINAL 3/ TET14"},
    {0330, 07, 43, "TERMINAL 4/ TET15"},
    {0340, 044, 44, "TERMINAL 5/ TET12"},
    {0350, 045, 45, "TERMINAL 6/ TET11"},
    {0360, 046, 46, "TERMINAL 7/ TET10"},
    {0370, 047, 47, "TERMINAL 8/ TET9"},

    // Group 9
    {01300, 050, 60, "TERMINAL 9"},
    {01310, 051, 61, "TERMINAL 10"},
    {01320, 052, 62, "TERMINAL 11"},
    {01330, 053, 63, "TERMINAL 12"},
    {01340, 054, 64, "TERMINAL 13"},
    {01350, 055, 65, "TERMINAL 14"},
    {01360, 056, 66, "TERMINAL 15"},
    {01370, 057, 67, "TERMINAL 16"},

    // Group 17
    {0200, 060, 7, "TERMINAL 17"},
    {0210, 061, 17, "TERMINAL 18"},
    {0220, 062, 52, "TERMINAL 19"},
    {0230, 063, 53, "TERMINAL 20"},
    {0240, 064, 54, "TERMINAL 21"},
    {0250, 065, 55, "TERMINAL 22"},
    {0260, 066, 56, "TERMINAL 23"},
    {0270, 067, 57, "TERMINAL 24"},

    // Group 25
    {01200, 070, 70, "TERMINAL 25"},
    {01210, 071, 71, "TERMINAL 26"},
    {01220, 072, 72, "TERMINAL 27"},
    {01230, 073, 73, "TERMINAL 28"},
    {01240, 074, 74, "TERMINAL 29/PHOTOS.1"},
    {01250, 075, 75, "TERMINAL 30/PHOTOS.2"},
    {01260, 076, 76, "TERMINAL 31/PHOTOS.3"},
    {01270, 077, 77, "TERMINAL 32/PHOTOS.4"},

    // Group 33
    {0640, 0124, 1040, "TERMINAL 33"},
    {0650, 0125, 1041, "TERMINAL 34"},
    {0660, 0126, 1042, "TERMINAL 35"},
    {0670, 0127, 1043, "TERMINAL 36"},

    // Group 37
    {01100, 0130, 1044, "TERMINAL 37"},
    {01110, 0131, 1045, "TERMINAL 38"},
    {01120, 0132, 1046, "TERMINAL 39"},
    {01130, 0133, 1047, "TERMINAL 40"},
    {01140, 0134, 1050, "TERMINAL 41"},
    {01150, 0135, 1051, "TERMINAL 42"},
    {01160, 0136, 1052, "TERMINAL 43"},
    {01170, 0137, 1053, "TERMINAL 44"},

    // Group 45
    {01400, 0140, 1054, "TERMINAL 45"},
    {01410, 0141, 1055, "TERMINAL 46"},
    {01420, 0142, 1056, "TERMINAL 47"},
    {01430, 0143, 1057, "TERMINAL 48"},

    // Group 49
    {01500, 0144, 1060, "TERMINAL 49"},
    {01510, 0145, 1061, "TERMINAL 50"},
    {01520, 0146, 1062, "TERMINAL 51"},
    {01530, 0147, 1063, "TERMINAL 52"}};

static const size_t numDeviceDefinitions = sizeof(deviceDefinitions) / sizeof(DeviceDefinition);

// Forward declaration of WriteEnd
static bool WriteEnd(void *context, int param);


static void Terminal_Reset(Device *self)
{
    TerminalData *data = (TerminalData *)self->deviceData;
    if (!data)
        return;

    // Clear input status and make device active
    data->inputStatus.raw = 0;
    data->inputStatus.bits.deviceActivated = true;
    data->inputStatus.bits.deviceReadyForTransfer = false;

    // Clear output status
    data->outputStatus.raw = 0;
    data->outputStatus.bits.readyForTransfer = true;

    // Clear other
    // data->noCarrier = false;
    // data->uartInputBuf = 0;
}

static uint16_t Terminal_Tick(Device *self)
{
    if (!self)
        return 0;

    TerminalData *data = (TerminalData *)self->deviceData;
    if (!data)
        return 0;

    // Process I/O delays
    Device_TickIODelay(self);

    // Check for new incoming characters

    data->checkInputQueueTick++;
    if (data->checkInputQueueTick > MAX_TICKS)
    {
        data->checkInputQueueTick = 0;

        if ((data->inputQueue.count > 0) &&
            (!data->inputStatus.bits.deviceReadyForTransfer) &&
            (data->outputStatus.bits.readyForTransfer))
        {

            uint16_t value = data->inputQueue.buffer[data->inputQueue.head];
            data->inputQueue.head = (data->inputQueue.head + 1) % TERMINAL_QUEUE_SIZE;
            data->inputQueue.count--;

            // Process character based on length
            switch (data->inputControl.bits.characterLength) // 0=8, 1=7, 2=6, 3=5)
            {
            case 0: // 8 bits
                value &= 0xFF;
                if (Device_GetOddParity(value) == 1)
                {
                    value |= (1 << 7);
                }
                break;

            case 1: // 7 bits
                value &= 0x7F;
                if (data->inputControl.bits.parityGeneration)
                {
                    if (Device_GetOddParity(value) == 1)
                    {
                        value |= (1 << 7);
                    }
                }
                break;

            case 2: // 6 bits
                value &= 0x3F;
                break;

            case 3: // 5 bits
                value &= 0x1F;
                break;
            }

            // printf("Terminal_Tick: %c, value: %o interruptEnabled: %d\n", (char)value, value, data->inputStatus.bits.interruptEnabled);
            data->uartInputBuf = value;
            data->inputStatus.bits.deviceReadyForTransfer = true;
            Device_SetInterruptStatus(self, data->inputStatus.bits.interruptEnabled && data->inputStatus.bits.deviceReadyForTransfer, 12);
        }
    }

    return self->interruptBits;
}

static uint16_t Terminal_Read(Device *self, uint32_t address)
{
    if (!self)
        return 0;

    TerminalData *data = (TerminalData *)self->deviceData;
    uint16_t value = 0;
    uint32_t reg = Device_RegisterAddress(self, address);

    switch (reg)
    {
    case TERMINAL_READ_INPUT_DATA:
        value = data->uartInputBuf;

        data->uartInputBuf = 0;
        data->inputStatus.bits.deviceReadyForTransfer = false;

        Device_SetInterruptStatus(self, data->inputStatus.bits.interruptEnabled && data->inputStatus.bits.deviceReadyForTransfer, 12);

        break;

    case TERMINAL_READ_INPUT_STATUS:
        value = data->inputStatus.raw;
        break;

    case TERMINAL_READ_RETURN0:
        value = 0;
        break;

    case TERMINAL_READ_OUTPUT_STATUS:
        value = data->outputStatus.raw;
        break;
    default:
        //printf("Unexpected: Terminal Read from address: %o value: %o\n", address, value);
        break;
    }

#ifdef DEBUG_TERMINAL
    if (reg != TERMINAL_READ_INPUT_DATA)
    {
        printf("Terminal Reading from address: %o value: %o\n", address, value);
    }
#endif

    return value;
}

static void Terminal_Write(Device *self, uint32_t address, uint16_t value)
{
    if (!self)
        return;

    TerminalData *data = (TerminalData *)self->deviceData;
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_TERMINAL
    if (reg != TERMINAL_WRITE_DATA)
    {
        printf("Terminal Writing value: %o to address: %o\n", value, address);
    }
#endif

    switch (reg)
    {
    case TERMINAL_WRITE_NO_OPERATION:
        // Do nothing
        break;

    case TERMINAL_WRITE_INPUT_CONTROL:

        // make a copy of the control word
        data->inputControl.raw = value;

        // Update status register
        data->inputStatus.bits.interruptEnabled = data->inputControl.bits.interruptEnabled;
        data->inputStatus.bits.deviceActivated = data->inputControl.bits.deviceActivated;

        // Trigger interrupt ?
        Device_SetInterruptStatus(self, data->inputStatus.bits.interruptEnabled && data->inputStatus.bits.deviceReadyForTransfer, 12);

        if (data->inputControl.bits.deviceClear)
        {
            // Device clear
            // printf("Device clear\n");

            // Clear input status
            data->inputStatus.raw = 0;
            data->inputStatus.bits.deviceActivated = true;

            // Clear output status
            data->outputStatus.raw = 0;
            data->outputStatus.bits.readyForTransfer = true;
        }

        // Clear errors
        data->inputStatus.bits.framingError = false;
        data->inputStatus.bits.parityError = false;
        data->inputStatus.bits.overrunError = false;

        break;

    case TERMINAL_WRITE_DATA:
        if (value == 0)
            break;

        char c = (char)(value);

        // seems like we need to strip the parity bit always
        //if (data->inputControl.bits.characterLength != 0)
        
        c &= 0x7F;

        // Clear interrupt status
        data->outputStatus.bits.readyForTransfer = false;
        Device_SetInterruptStatus(self, data->outputStatus.bits.interruptEnabled && data->outputStatus.bits.readyForTransfer, 10);

        if (data->inputControl.bits.testMode)
        {
            // In test mode, echo the character back with parity
            Terminal_QueueKeyCode(self, c);
        }
        else
        {            
            // Use character device callback if available
            if (self->deviceClass == DEVICE_CLASS_CHARACTER && self->charCallbacks.outputFunc) {
                Device_OutputCharacter(self, c);
            } else {
                // Fallback to printf if no callback is set
                printf("%c", c);
            }
        }

        // Simulate transfer delay
        Device_QueueIODelay(self, IODELAY_TERMINAL, WriteEnd, self->identCode, self->interruptLevel);
        break;

    case TERMINAL_WRITE_SET_OUTPUT_CONTROL:
        // make a copy of the control word
        data->outputControl.raw = value;

        // Update status register
        data->outputStatus.bits.interruptEnabled = data->outputControl.bits.interruptEnabled;

        // Trigger interrupt ?
        Device_SetInterruptStatus(self,
                                  data->outputStatus.bits.interruptEnabled && data->outputStatus.bits.readyForTransfer,
                                  10);
        break;
    }
}

static uint16_t Terminal_Ident(Device *self, uint16_t level)
{
    if (!self)
        return 0;

    if ((self->interruptBits & (1 << level)) != 0)
    {
        TerminalData *data = (TerminalData *)self->deviceData;
        if (level == 12)
        { // input
            data->inputStatus.bits.interruptEnabled = false;
        }
        else if (level == 10)
        { // output
            data->outputStatus.bits.interruptEnabled = false;
        }
        Device_SetInterruptStatus(self, false, level);
        return self->identCode;
    }
    return 0;
}

static bool WriteEnd(void *context, int param)
{
    Device *self = (Device *)context;
    if (!self)
        return false;

    TerminalData *data = (TerminalData *)self->deviceData;
    if (!data)
        return false;

    data->outputStatus.bits.readyForTransfer = true;
    data->checkInputQueueTick = 0; // Make sure input check is delayed

    Device_SetInterruptStatus(self,
                              data->outputStatus.bits.interruptEnabled && data->outputStatus.bits.readyForTransfer,
                              10);

    return false;
}

// Add QueueKeyCode implementation
void Terminal_QueueKeyCode(Device *self, uint8_t keycode)
{
    if (!self)
        return;

    TerminalData *data = (TerminalData *)self->deviceData;
    if (!data)
        return;

    // Check if queue is full
    if (data->inputQueue.count >= TERMINAL_QUEUE_SIZE)
    {
        data->inputStatus.bits.overrunError = true;
        return;
    }

    // Add keycode to queue
    data->inputQueue.buffer[data->inputQueue.tail] = keycode;
    data->inputQueue.tail = (data->inputQueue.tail + 1) % TERMINAL_QUEUE_SIZE;
    data->inputQueue.count++;

    // printf("Terminal_QueueKeyCode: %c, count: %ld IRQ[%d]\n", (char)keycode, data->inputQueue.count, data->inputStatus.bits.interruptEnabled);
}

// Character input handler for terminal devices
static void Terminal_InputFunction(Device *device, char c) {
    if (!device) return;
    Terminal_QueueKeyCode(device, (uint8_t)c);
}


Device *CreateTerminalDevice(uint8_t thumbwheel)
{
    Device *dev = malloc(sizeof(Device));
    if (!dev)
        return NULL;

    TerminalData *data = malloc(sizeof(TerminalData));
    if (!data)
    {
        free(dev);
        return NULL;
    }

    // Initialize device base structure as a character device
    Device_Init(dev, thumbwheel, DEVICE_CLASS_CHARACTER, 0);
    
    // Set up device-specific data
    memset(data, 0, sizeof(TerminalData));

    // Find device definition
    const DeviceDefinition *def = NULL;
    if (thumbwheel < numDeviceDefinitions)
    {
        if (thumbwheel > 0)
            thumbwheel--; // Offset into definitions is 0
        def = &deviceDefinitions[thumbwheel];
    }
    else
    {
        printf("Unexpected thumbwheel code %d\n", thumbwheel);
        free(data);
        free(dev);
        return NULL;
    }

    // Set up device properties
    dev->identCode = def->identCode;
    dev->startAddress = def->addressBase;
    dev->logicalDevice = def->logicalDevice;
    dev->endAddress = def->addressBase + 7;
    dev->interruptLevel = 10; // 10 = output, 12 = input
    strncpy(dev->memoryName, def->deviceName, sizeof(dev->memoryName) - 1);
    dev->memoryName[sizeof(dev->memoryName) - 1] = '\0';

    // Set up device function pointers
    dev->Reset = Terminal_Reset;
    dev->Tick = Terminal_Tick;
    dev->Read = Terminal_Read;
    dev->Write = Terminal_Write;
    dev->Ident = Terminal_Ident;
    dev->deviceData = data;

    // Initialize input queue
    data->inputQueue.head = 0;
    data->inputQueue.tail = 0;
    data->inputQueue.count = 0;

    // Ready to be written to
    data->outputStatus.bits.readyForTransfer = true;


    // hook up  Device_SetCharacterInput to Terminal_QueueKeyCode
    Device_SetCharacterInput(dev, Terminal_InputFunction);

    printf("Terminal device created: %s CODE[%o] ADDRESS[%o-%o] \n", dev->memoryName, dev->identCode, dev->startAddress, dev->endAddress);
    return dev;
}

