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

#include "deviceHDLC.h"

// Debug flags (convert from C# #define)
//#define DEBUG_DETAIL
//#define DEBUG_DETAIL_PLUS_DESCRIPTION
//#define DMA_DEBUG
//#define RX_BLAST_LOGGING

// Device definitions array for HDLC devices
static const struct {
    uint16_t addressBase;
    uint16_t identCode;
    const char* deviceName;
} hdlcDeviceDefinitions[] = {
    {01640, 0150, "HDLC/MEGALINK 1"},  // thumbwheel 1
    {01660, 0151, "HDLC/MEGALINK 2"},  // thumbwheel 2
    {01700, 0152, "HDLC/MEGALINK 3"},  // thumbwheel 3
    {01720, 0153, "HDLC/MEGALINK 4"},  // thumbwheel 4
    {01740, 0154, "HDLC/MEGALINK 5"},  // thumbwheel 5
};

static void HDLC_Reset(Device *self)
{
    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return;

    // Clear all registers and status
    memset(&data->rxTransferStatus, 0, sizeof(HDLCReceiverTransferStatus));
    memset(&data->rxTransferControl, 0, sizeof(HDLCReceiverTransferControl));
    memset(&data->txTransferStatus, 0, sizeof(HDLCTransmitterTransferStatus));
    memset(&data->txTransferControl, 0, sizeof(HDLCTransmitterTransferControl));

    // Reset HDLC chip registers
    data->rxDataRegister = 0;
    data->rxStatusRegister = 0;
    data->txDataRegister = 0;
    data->txStatusRegister = 0;
    data->parameterControlRegister = 0;
    data->syncAddressRegister = 0;
    data->characterLength = 0;
    data->txControlRegister = 0;

    // Reset DMA state
    data->dmaAddress = 0;
    data->dmaCommand = 0;
    data->txState = HDLC_DMA_TX_STOPPED;
    data->blockState = HDLC_DMA_BLOCK_IDLE;

    // Reset internal state
    data->deviceActive = false;
    data->maintenanceMode = false;
    data->tickCounter = 0;

    // Set default baud rate based on thumbwheel
    data->baudRate = HDLC_BAUD_9600; // Default to 9600 bps
}

static uint16_t HDLC_Tick(Device *self)
{
    if (!self) return 0;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return 0;

    // Process I/O delays
    Device_TickIODelay(self);

    // Increment tick counter for timing
    data->tickCounter++;

    // TODO: Implement COM5025 chip emulation
    // TODO: Implement modem handling
    // TODO: Implement DMA engine

    return self->interruptBits;
}

static uint16_t HDLC_Read(Device *self, uint32_t address)
{
    if (!self) return 0;

    HDLCData *data = (HDLCData *)self->deviceData;
    uint16_t value = 0;
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_DETAIL
    printf("HDLC Read from register %d (address %o)\n", reg, address);
#endif

    switch (reg) {
        case HDLC_READ_RX_DATA:                // IOX +0: Read Receiver Data Register
            value = data->rxDataRegister;
            break;

        case HDLC_READ_RX_STATUS:              // IOX +2: Read Receiver Status Register
            value = data->rxStatusRegister;
            break;

        case HDLC_WRITE_CHAR_LENGTH:           // IOX +4: Character Length (read operation)
            // TODO: Implement character length reading
            value = 0; // Default to 8 bits
            break;

        case HDLC_READ_TX_STATUS:              // IOX +6: Read Transmitter Status Register
            value = data->txStatusRegister;
            break;

        case HDLC_READ_RX_TRANSFER_STATUS:     // IOX +10: Read Receiver Transfer Status
#ifdef DEBUG_DETAIL_PLUS_DESCRIPTION
            printf("Read RRTS: 0x%04X\n", data->rxTransferStatus.raw);
#endif
            // Clear DMA Module Request before reading
            data->rxTransferStatus.bits.dmaModuleRequest = 0;

            value = data->rxTransferStatus.raw;

            // Clear DMA bits 8-15 after read
            data->rxTransferStatus.bits.blockEnd = 0;
            data->rxTransferStatus.bits.frameEnd = 0;
            data->rxTransferStatus.bits.listEnd = 0;
            data->rxTransferStatus.bits.listEmpty = 0;
            data->rxTransferStatus.bits.undefined12 = 0;
            break;

        case HDLC_READ_TX_TRANSFER_STATUS:     // IOX +12: Read Transmitter Transfer Status
#ifdef DEBUG_DETAIL_PLUS_DESCRIPTION
            printf("Read RTTS: 0x%04X\n", data->txTransferStatus.raw);
#endif
            // Clear DMA Module Request before reading
            data->txTransferStatus.bits.dmaModuleRequest = 0;

            value = data->txTransferStatus.raw;

            // Clear DMA bits 8-14 after read (bit 15 is NOT auto-cleared)
            data->txTransferStatus.bits.blockEnd = 0;
            data->txTransferStatus.bits.frameEnd = 0;
            data->txTransferStatus.bits.listEnd = 0;
            data->txTransferStatus.bits.transmissionFinished = 0;
            data->txTransferStatus.bits.undefined12 = 0;
            data->txTransferStatus.bits.undefined13 = 0;
            data->txTransferStatus.bits.undefined14 = 0;
            break;

        case HDLC_READ_DMA_ADDRESS:            // IOX +14: Read DMA Address
            value = data->dmaAddress;
            break;

        case HDLC_READ_DMA_COMMAND:            // IOX +16: Read DMA Command Register
            value = ((uint16_t)data->dmaCommand) << 8;
            break;

        default:
            // Invalid register
            break;
    }

#ifdef DEBUG_DETAIL
    printf("HDLC Read from register %d returned 0x%04X\n", reg, value);
#endif

    return value;
}

static void HDLC_Write(Device *self, uint32_t address, uint16_t value)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_DETAIL
    printf("HDLC Write to register %d (address %o) value 0x%04X\n", reg, address, value);
#endif

    switch (reg) {
        case HDLC_WRITE_PARAMETER_CONTROL:     // IOX +1: Write Parameter Control Register
            data->parameterControlRegister = (uint8_t)value;
#ifdef DEBUG_DETAIL_PLUS_DESCRIPTION
            printf("Writing Parameter Control Register = 0x%02X\n", (uint8_t)value);
#endif
            // TODO: Configure COM5025 MODE register
            break;

        case HDLC_WRITE_SYNC_ADDRESS:          // IOX +3: Write Sync/Address Register
            data->syncAddressRegister = (uint8_t)value;
            // TODO: Configure COM5025 SYNC/Address register
            break;

        case HDLC_WRITE_TX_DATA:               // IOX +5: Write Transmitter Data Register
            data->txDataRegister = (uint8_t)value;
            // TODO: Send data to COM5025 transmitter
            break;

        case HDLC_WRITE_TX_CONTROL:            // IOX +7: Write Transmitter Control Register
            data->txControlRegister = (uint8_t)value;
            // TODO: Configure COM5025 transmitter control
            break;

        case HDLC_WRITE_RX_TRANSFER_CONTROL:   // IOX +11: Write Receiver Transfer Control
            data->rxTransferControl.raw = value;

            if (value == 0) {
                // Clear maintenance/loopback mode
                data->maintenanceMode = false;
            }

            // Handle device clear/maintenance mode
            if (!data->maintenanceMode && data->rxTransferControl.bits.deviceClearSelectMaint) {
#ifdef DEBUG_DETAIL
                printf("HDLC Device Clear\n");
#endif
                HDLC_Reset(self);
                data->rxTransferControl.bits.deviceClearSelectMaint = 0;
                data->maintenanceMode = true;
            }

            // TODO: Configure receiver enable, DMA enable, etc.
            // TODO: Handle DTR signal to modem
            break;

        case HDLC_WRITE_TX_TRANSFER_CONTROL:   // IOX +13: Write Transmitter Transfer Control
            data->txTransferControl.raw = value;
            // TODO: Configure transmitter enable, DMA enable, etc.
            // TODO: Handle RTS signal logic
            break;

        case HDLC_WRITE_DMA_ADDRESS:           // IOX +15: Write DMA Address
            data->dmaAddress = value;
            break;

        case HDLC_WRITE_DMA_COMMAND:           // IOX +17: Write DMA Command
            data->dmaCommand = (value >> 8) & 0x07;
#ifdef DMA_DEBUG
            printf("HDLC DMA Command: %d\n", data->dmaCommand);
#endif
            // TODO: Execute DMA command
            break;

        default:
            // Invalid register
            break;
    }
}

static uint16_t HDLC_Ident(Device *self, uint16_t level)
{
    if (!self) return 0;

#ifdef DEBUG_DETAIL
    printf("HDLC IDENT level %d\n", level);
#endif

    HDLCData *data = (HDLCData *)self->deviceData;

    // Clear interrupt enable flags for the serviced level using masks
    if (level == 12) {
        data->txTransferControl.raw &= HDLC_TTC_MASK_CLEAR_IDENT;
    }

    if (level == 13) {
        data->rxTransferControl.raw &= HDLC_RTC_MASK_CLEAR_IDENT;
    }

    // Always clear interrupt status for this level
    Device_SetInterruptStatus(self, false, level);

    // Always return ident code
    return self->identCode;
}

static void HDLC_Destroy(Device *self)
{
    if (!self) return;

    // Any device-specific cleanup would go here
    // For now, the base Device_Destroy will handle freeing deviceData
}

Device* CreateHDLCDevice(uint8_t thumbwheel)
{
    // Validate thumbwheel value
    if (thumbwheel < 1 || thumbwheel > 5) {
        printf("HDLC: Invalid thumbwheel value %d (must be 1-5)\n", thumbwheel);
        return NULL;
    }

    // Allocate device structure
    Device *dev = malloc(sizeof(Device));
    if (!dev) {
        printf("HDLC: Failed to allocate device structure\n");
        return NULL;
    }

    // Initialize device with STANDARD class (could be CHARACTER if needed)
    Device_Init(dev, thumbwheel, DEVICE_CLASS_STANDARD, 0);

    // Allocate device-specific data
    HDLCData *data = malloc(sizeof(HDLCData));
    if (!data) {
        printf("HDLC: Failed to allocate device data\n");
        free(dev);
        return NULL;
    }

    // Clear device data
    memset(data, 0, sizeof(HDLCData));

    // Set up device configuration based on thumbwheel
    int devIndex = thumbwheel - 1;

    // Configure device address range
    dev->startAddress = hdlcDeviceDefinitions[devIndex].addressBase;
    dev->endAddress = hdlcDeviceDefinitions[devIndex].addressBase + 15; // 16 registers (0-15)

    // Configure device identification
    dev->identCode = hdlcDeviceDefinitions[devIndex].identCode;
    dev->logicalDevice = 1360 + (thumbwheel - 1) * 2; // SINTRAN logical device numbers
    dev->interruptLevel = 12; // Default to transmitter interrupt level

    // Set device name
    strncpy(dev->memoryName, hdlcDeviceDefinitions[devIndex].deviceName, MAX_DEVICE_NAME - 1);
    dev->memoryName[MAX_DEVICE_NAME - 1] = '\0';

    // Configure device-specific data
    data->thumbwheel = thumbwheel;
    data->baudRate = HDLC_BAUD_9600; // Default baud rate

    // Set device function pointers
    dev->Reset = HDLC_Reset;
    dev->Tick = HDLC_Tick;
    dev->Read = HDLC_Read;
    dev->Write = HDLC_Write;
    dev->Ident = HDLC_Ident;
    dev->Destroy = HDLC_Destroy;
    dev->Boot = NULL; // HDLC devices don't support booting

    // Link device data
    dev->deviceData = data;

#ifdef DEBUG_DETAIL
    printf("HDLC: Created device thumbwheel=%d address=%o-%o ident=%o\n",
           thumbwheel, dev->startAddress, dev->endAddress, dev->identCode);
#endif

    return dev;
}

// Helper functions for future implementation

static void HDLC_CheckInterrupts(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;

    // Check level 13 (receiver) interrupts
    if (data->rxTransferStatus.bits.dataAvailable &&
        data->rxTransferControl.bits.dataAvailableIE) {
        Device_SetInterruptStatus(self, true, 13);
    }

    if (data->rxTransferStatus.bits.statusAvailable &&
        data->rxTransferControl.bits.statusAvailableIE) {
        Device_SetInterruptStatus(self, true, 13);
    }

    // Check level 12 (transmitter) interrupts
    if (data->txTransferStatus.bits.transmitBufferEmpty &&
        data->txTransferControl.bits.transmitBufferEmptyIE) {
        Device_SetInterruptStatus(self, true, 12);
    }

    if (data->txTransferStatus.bits.transmitterUnderrun &&
        data->txTransferControl.bits.transmitterUnderrunIE) {
        Device_SetInterruptStatus(self, true, 12);
    }
}

static void HDLC_SetBaudRate(Device *self, HDLCBaudRate baudRate)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    data->baudRate = baudRate;

    // TODO: Configure actual timing based on baud rate
}

// TODO: Implement these functions for full HDLC functionality:
// - HDLC_InitializeCOM5025()
// - HDLC_InitializeModem()
// - HDLC_InitializeDMAEngine()
// - HDLC_ProcessReceivedData()
// - HDLC_ProcessTransmitData()
// - HDLC_HandleModemSignals()
// - HDLC_ExecuteDMACommand()