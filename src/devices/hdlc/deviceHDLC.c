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

#define DEBUG_DETAIL_PLUS_DESCRIPTION
#define DEBUG_DETAIL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../devices_types.h"
#include "../devices_protos.h"

#include "deviceHDLC.h"
#include "hdlcFrame.h"
#include "chipCOM5025.h"
#include "dmaEnum.h"

// Forward declarations
static void HDLC_CheckTriggerIRQ12(Device *self);
static void HDLC_CheckTriggerIRQ13(Device *self);
static void HDLC_CheckTriggerInterrupt(Device *self);
static void HDLC_UpdateRQTS(Device *self);
static void HDLC_UpdateFromCOM5025Pins(Device *self);

// Modem event callbacks
static void HDLC_OnModemReceivedData(Device *device, const uint8_t *data, int length);
static void HDLC_OnModemRingIndicator(Device *device, bool pinValue);
static void HDLC_OnModemDataSetReady(Device *device, bool pinValue);
static void HDLC_OnModemSignalDetector(Device *device, bool pinValue);
static void HDLC_OnModemClearToSend(Device *device, bool pinValue);
static void HDLC_OnModemRequestToSend(Device *device, bool pinValue);
static void HDLC_OnModemDataTerminalReady(Device *device, bool pinValue);

// DMA engine event callbacks
static void HDLC_OnDMAWriteDMA(Device *device, uint32_t address, uint16_t data);
static void HDLC_OnDMAReadDMA(Device *device, uint32_t address, int *data);
static void HDLC_OnDMASetInterruptBit(Device *device, uint8_t bit);
static void HDLC_OnDMASendHDLCFrame(Device *device, HDLCFrame *frame);
static void HDLC_OnDMAUpdateReceiverStatus(Device *device, uint16_t status);
static void HDLC_OnDMAClearCommand(Device *device);

// COM5025 transmitter output callback
static void HDLC_OnCOM5025TransmitterOutput(Device *device, uint8_t serialOutput);

// COM5025 pin value changed callback
static void HDLC_OnCOM5025PinValueChanged(Device *device, COM5025SignalPinOut pin, bool value);

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

    // Initialize COM5025 chip
    if (data->com5025) {
        COM5025_Init(data->com5025);
        COM5025_Reset(data->com5025);
    }

    // Initialize modem signal flags and masks
    data->rxModemFlags.raw = 0;
    data->txModemFlags.raw = 0;
    data->rxModemFlagsMask.raw = (1 << 5) | (1 << 6) | (1 << 7); // SignalDetector, DataSetReady, RingIndicator
    data->txModemFlagsMask.raw = (1 << 6); // ReadyForSending

    // Initialize clock timing
    data->cpuTicks = 0;
    data->cpuTicksPerTx = 625; // Default timing divider

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

    // Clock the COM5025 chip with timing divider
    data->cpuTicks++;
    if (data->cpuTicks >= data->cpuTicksPerTx) {
        data->cpuTicks = 0;
        COM5025_ClockTransmitter(data->com5025);
        COM5025_ClockReceiver(data->com5025);
    }

    // Update HDLC controller based on COM5025 pin changes
    HDLC_UpdateFromCOM5025Pins(self);

    // Tick modem and DMA engine
    if (data->modem) {
        Modem_Tick(data->modem);
    }
    if (data->dmaEngine) {
        DMAEngine_Tick(data->dmaEngine);
    }

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
            value = COM5025_ReadByte(data->com5025, COM5025_REG_BYTE_RECEIVER_DATA_BUFFER);
            break;

        case HDLC_READ_RX_STATUS:              // IOX +2: Read Receiver Status Register
            value = COM5025_ReadByte(data->com5025, COM5025_REG_BYTE_RECEIVER_STATUS);
            break;

        case HDLC_WRITE_CHAR_LENGTH:           // IOX +4: Character Length (read operation)
            // Write character length register (use 8 bits default)
            COM5025_WriteByte(data->com5025, COM5025_REG_BYTE_DATA_LENGTH_SELECT, 0);
            value = 0; // Default to 8 bits
            break;

        case HDLC_READ_TX_STATUS:              // IOX +6: Read Transmitter Status Register
            value = COM5025_ReadByte(data->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
            break;

        case HDLC_READ_RX_TRANSFER_STATUS:     // IOX +10: Read Receiver Transfer Status
#ifdef DEBUG_DETAIL_PLUS_DESCRIPTION
            printf("Read RRTS: 0x%04X\n", data->rxTransferStatus.raw);
#endif
            // Clear DMA Module Request before reading
            data->rxTransferStatus.bits.dmaModuleRequest = 0;

            // Latch modem signals (RI, SD, DSR) on read of this register
            data->rxTransferStatus.raw &= ~(data->rxModemFlagsMask.raw); // Clear old modem bits
            data->rxTransferStatus.raw |= (data->rxModemFlags.raw & data->rxModemFlagsMask.raw); // Latch new bits

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

            // Latch ReadyForSending (CTS) on read of this register
            data->txTransferStatus.raw &= ~(data->txModemFlagsMask.raw); // Clear old modem bits
            data->txTransferStatus.raw |= (data->txModemFlags.raw & data->txModemFlagsMask.raw); // Latch new bits

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
            // Configure COM5025 MODE register
            COM5025_WriteByte(data->com5025, COM5025_REG_BYTE_MODE_CONTROL, (uint8_t)value);
            break;

        case HDLC_WRITE_SYNC_ADDRESS:          // IOX +3: Write Sync/Address Register
            data->syncAddressRegister = (uint8_t)value;
            // Configure COM5025 SYNC/Address register
            COM5025_WriteByte(data->com5025, COM5025_REG_BYTE_SYNC_ADDRESS, (uint8_t)value);
            break;

        case HDLC_WRITE_TX_DATA:               // IOX +5: Write Transmitter Data Register
            data->txDataRegister = (uint8_t)value;
            // Send data to COM5025 transmitter
            COM5025_WriteByte(data->com5025, COM5025_REG_BYTE_TRANSMITTER_DATA, (uint8_t)value);
            break;

        case HDLC_WRITE_TX_CONTROL:            // IOX +7: Write Transmitter Control Register
            data->txControlRegister = (uint8_t)value;
            // Configure COM5025 transmitter control
            COM5025_WriteByte(data->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, (uint8_t)value);
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

            // Configure COM5025 receiver enable pin
            COM5025_SetInputPin(data->com5025, COM5025_PIN_IN_RXENA, data->rxTransferControl.bits.enableReceiver);

            // Set maintenance select pin
            if (data->maintenanceMode) {
                COM5025_SetInputPin(data->com5025, COM5025_PIN_IN_MSEL, true);
                HDLC_UpdateRQTS(self);
            }

            // Handle DTR signal to modem (equivalent to C# modem.SetDTR())
            if (data->modem) {
                Modem_SetDTR(data->modem, data->rxTransferControl.bits.dtr);
            }

            // Check for interrupt triggers
            HDLC_CheckTriggerIRQ13(self);
            HDLC_CheckTriggerInterrupt(self);
            break;

        case HDLC_WRITE_TX_TRANSFER_CONTROL:   // IOX +13: Write Transmitter Transfer Control
            data->txTransferControl.raw = value;

            // Configure COM5025 transmitter enable pin
            COM5025_SetInputPin(data->com5025, COM5025_PIN_IN_TXENA, data->txTransferControl.bits.transmitterEnabled);

            // Update RQTS signal logic
            HDLC_UpdateRQTS(self);

            // Check for interrupt triggers
            HDLC_CheckTriggerIRQ12(self);
            break;

        case HDLC_WRITE_DMA_ADDRESS:           // IOX +15: Write DMA Address
            data->dmaAddress = value;
            // Update DMA engine with the new address
            if (data->dmaEngine) {
                DMAEngine_SetDMAAddress(data->dmaEngine, value);
            }
            break;

        case HDLC_WRITE_DMA_COMMAND:           // IOX +17: Write DMA Command
            data->dmaCommand = (value >> 8) & 0x07;
#ifdef DMA_DEBUG
            printf("HDLC DMA Command: %d\n", data->dmaCommand);
#endif
            // Execute DMA command
            if (data->dmaEngine) {
                // First set the DMA address if it was provided
                // Note: DMA address should be set via HDLC_WRITE_DMA_ADDRESS before command
                // For data transfer commands, we need to pass the address to the DMA engine

                switch (data->dmaCommand) {
                    case DMA_CMD_DEVICE_CLEAR:
                        DMAEngine_CommandDeviceClear(data->dmaEngine);
                        break;
                    case DMA_CMD_INITIALIZE:
                        DMAEngine_CommandInitialize(data->dmaEngine);
                        break;
                    case DMA_CMD_RECEIVER_START:
                        DMAEngine_CommandReceiverStart(data->dmaEngine);
                        break;
                    case DMA_CMD_RECEIVER_CONTINUE:
                        DMAEngine_CommandReceiverContinue(data->dmaEngine);
                        break;
                    case DMA_CMD_TRANSMITTER_START:
                        DMAEngine_CommandTransmitterStart(data->dmaEngine);
                        break;
                    case DMA_CMD_DUMP_DATA_MODULE:
                        DMAEngine_CommandDumpDataModule(data->dmaEngine);
                        break;
                    case DMA_CMD_DUMP_REGISTERS:
                        DMAEngine_CommandDumpRegisters(data->dmaEngine);
                        break;
                    case DMA_CMD_LOAD_REGISTERS:
                        DMAEngine_CommandLoadRegisters(data->dmaEngine);
                        break;
                    default:
#ifdef DMA_DEBUG
                        printf("HDLC: Unknown DMA command: %d\n", data->dmaCommand);
#endif
                        break;
                }
            }
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

    HDLCData *data = (HDLCData *)self->deviceData;
    if (data) {
        // Clean up DMA engine
        if (data->dmaEngine) {
            DMAEngine_Destroy(data->dmaEngine);
            free(data->dmaEngine);
            data->dmaEngine = NULL;
        }

        // Clean up modem
        if (data->modem) {
            Modem_Destroy(data->modem);
            free(data->modem);
            data->modem = NULL;
        }

        // Clean up COM5025
        if (data->com5025) {
            free(data->com5025);
            data->com5025 = NULL;
        }
    }

    // Base Device_Destroy will handle freeing deviceData
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

    // Allocate COM5025 chip state
    data->com5025 = malloc(sizeof(COM5025State));
    if (!data->com5025) {
        printf("HDLC: Failed to allocate COM5025 state\n");
        free(data);
        free(dev);
        return NULL;
    }

    // Allocate modem state
    data->modem = malloc(sizeof(ModemState));
    if (!data->modem) {
        printf("HDLC: Failed to allocate modem state\n");
        free(data->com5025);
        free(data);
        free(dev);
        return NULL;
    }

    // Allocate DMA engine state
    data->dmaEngine = malloc(sizeof(DMAEngine));
    if (!data->dmaEngine) {
        printf("HDLC: Failed to allocate DMA engine state\n");
        free(data->modem);
        free(data->com5025);
        free(data);
        free(dev);
        return NULL;
    }

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

    // Initialize COM5025, modem, and DMA engine
    Modem_Init(data->modem, dev);
    DMAEngine_Init(data->dmaEngine, true, dev, data->modem, data->com5025);

    // Set up COM5025 callbacks
    COM5025_SetTransmitterOutputCallback(data->com5025, (void (*)(void *, uint8_t))HDLC_OnCOM5025TransmitterOutput, dev);
    COM5025_SetPinValueChangedCallback(data->com5025, (void (*)(void *, COM5025SignalPinOut, bool))HDLC_OnCOM5025PinValueChanged, dev);

    // Set up modem callbacks (equivalent to C# event subscriptions)
    Modem_SetReceivedDataCallback(data->modem, HDLC_OnModemReceivedData);
    Modem_SetRingIndicatorCallback(data->modem, HDLC_OnModemRingIndicator);
    Modem_SetDataSetReadyCallback(data->modem, HDLC_OnModemDataSetReady);
    Modem_SetSignalDetectorCallback(data->modem, HDLC_OnModemSignalDetector);
    Modem_SetClearToSendCallback(data->modem, HDLC_OnModemClearToSend);
    Modem_SetRequestToSendCallback(data->modem, HDLC_OnModemRequestToSend);
    Modem_SetDataTerminalReadyCallback(data->modem, HDLC_OnModemDataTerminalReady);

    // Set up DMA engine callbacks (equivalent to C# event subscriptions)
    DMAEngine_SetWriteDMACallback(data->dmaEngine, HDLC_OnDMAWriteDMA);
    DMAEngine_SetReadDMACallback(data->dmaEngine, HDLC_OnDMAReadDMA);
    DMAEngine_SetInterruptCallback(data->dmaEngine, HDLC_OnDMASetInterruptBit);
    DMAEngine_SetSendFrameCallback(data->dmaEngine, HDLC_OnDMASendHDLCFrame);
    DMAEngine_SetUpdateReceiverStatusCallback(data->dmaEngine, HDLC_OnDMAUpdateReceiverStatus);
    DMAEngine_SetClearCommandCallback(data->dmaEngine, HDLC_OnDMAClearCommand);

#ifdef DEBUG_DETAIL
    printf("HDLC: Created device thumbwheel=%d address=%o-%o ident=%o\n",
           thumbwheel, dev->startAddress, dev->endAddress, dev->identCode);
#endif

    return dev;
}

// Modem event callback implementations (equivalent to C# event handlers)

static void HDLC_OnModemReceivedData(Device *device, const uint8_t *data, int length)
{
    if (!device || !data) return;

    HDLCData *hdlcData = (HDLCData *)device->deviceData;
    if (!hdlcData || !hdlcData->dmaEngine) return;

    // Equivalent to C# Modem_OnReceivedData
    if (hdlcData->dmaEngine) {
        DMAEngine_BlastReceiveDataBuffer(hdlcData->dmaEngine, data, length);
    }
}

static void HDLC_OnModemRingIndicator(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Equivalent to C# Modem_OnRingIndicator
    if (pinValue) {
        data->rxModemFlags.bits.ringIndicator = 1;
    } else {
        data->rxModemFlags.bits.ringIndicator = 0;
    }
    HDLC_CheckTriggerIRQ13(device);
}

static void HDLC_OnModemDataSetReady(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Equivalent to C# Modem_OnDataSetReady
    if (pinValue) {
        data->rxModemFlags.bits.dataSetReady = 1;
    } else {
        data->rxModemFlags.bits.dataSetReady = 0;
    }
    HDLC_CheckTriggerIRQ13(device);
}

static void HDLC_OnModemSignalDetector(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Equivalent to C# Modem_OnSignalDetector
    if (pinValue) {
        data->rxModemFlags.bits.signalDetector = 1;
    } else {
        data->rxModemFlags.bits.signalDetector = 0;
    }
    HDLC_CheckTriggerIRQ13(device);
}

static void HDLC_OnModemClearToSend(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Equivalent to C# Modem_OnClearToSend
    if (pinValue) {
        data->txModemFlags.bits.readyForSending = 1;
    } else {
        data->txModemFlags.bits.readyForSending = 0;
    }
    HDLC_CheckTriggerIRQ12(device);
}

static void HDLC_OnModemRequestToSend(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data || !data->modem) return;

    // Equivalent to C# Modem_OnRequestToSend
    // In point-to-point setup, RTS connects to CTS
    Modem_SetCTS(data->modem, pinValue);
}

static void HDLC_OnModemDataTerminalReady(Device *device, bool pinValue)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data || !data->modem) return;

    // Equivalent to C# Modem_OnDataTerminalReady
    // In point-to-point setup, DTR connects to DSR
    Modem_SetDSR(data->modem, pinValue);
}

// DMA engine event callback implementations (equivalent to C# event handlers)

static void HDLC_OnDMAWriteDMA(Device *device, uint32_t address, uint16_t data)
{
    if (!device) return;

    // Equivalent to C# DmaEngine_OnWriteDMA
    Device_DMAWrite(address, data);
}

static void HDLC_OnDMAReadDMA(Device *device, uint32_t address, int *data)
{
    if (!device || !data) return;

    // Equivalent to C# DmaEngine_OnReadDMA
    *data = Device_DMARead(address);
}

static void HDLC_OnDMASetInterruptBit(Device *device, uint8_t bit)
{
    if (!device) return;

    // Equivalent to C# DmaEngine_OnSetInterruptBit
    Device_SetInterruptStatus(device, true, bit);
}

static void HDLC_OnDMASendHDLCFrame(Device *device, HDLCFrame *frame)
{
    if (!device || !frame) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data || !data->modem) return;

    // Equivalent to C# DmaEngine_OnSendHDLCFrame
    if (frame->frameBuffer && frame->frameLength > 0) {
        Modem_SendBytes(data->modem, frame->frameBuffer, frame->frameLength);
    }
}

static void HDLC_OnDMAUpdateReceiverStatus(Device *device, uint16_t status)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // OR the COM5025 receiver status into the receiver transfer status to prevent loss of information
    // Map COM5025 status bits to HDLC receiver transfer status bits
    data->rxTransferStatus.raw |= (status & 0xFF); // Only use lower 8 bits
}

static void HDLC_OnDMAClearCommand(Device *device)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Clear the DMA command register
    data->dmaCommand = 0;
}

// COM5025 transmitter output callback implementation

static void HDLC_OnCOM5025TransmitterOutput(Device *device, uint8_t serialOutput)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data || !data->modem) return;

    // Equivalent to C# Com5025_OnTransmitterOutput
    // Send serial output byte to modem
    Modem_SendByte(data->modem, serialOutput);
}

// COM5025 pin value changed callback implementation
static void HDLC_OnCOM5025PinValueChanged(Device *device, COM5025SignalPinOut pin, bool value)
{
    if (!device) return;

    HDLCData *data = (HDLCData *)device->deviceData;
    if (!data) return;

    // Equivalent to C# Com5025_OnPinValueChanged
    // Handle pin value changes that affect HDLC controller state
    switch (pin) {
        case COM5025_PIN_OUT_SFR: // Sync/Flag received
            if (value) {
                data->rxTransferStatus.bits.syncFlagReceived = 1;
            } else {
                data->rxTransferStatus.bits.syncFlagReceived = 0;
            }
            break;

        case COM5025_PIN_OUT_RXACT: // Receiver Active
            if (value) {
                data->rxTransferStatus.bits.receiverActive = 1;
            } else {
                data->rxTransferStatus.bits.receiverActive = 0;
            }
            break;

        case COM5025_PIN_OUT_RDA: // Receiver Data Available
            if (value) {
                data->rxTransferStatus.bits.dataAvailable = 1;
            } else {
                data->rxTransferStatus.bits.dataAvailable = 0;
            }
            break;

        case COM5025_PIN_OUT_TXACT: // Transmitter Active
            if (value) {
                data->txTransferStatus.bits.transmitterActive = 1;
            } else {
                data->txTransferStatus.bits.transmitterActive = 0;
            }
            break;

        case COM5025_PIN_OUT_TBMT: // Transmitter Buffer Empty
            if (value) {
                data->txTransferStatus.bits.transmitBufferEmpty = 1;
                // Handle DMA transmitter operations
                if (data->txTransferControl.bits.enableTransmitterDMA) {
                    // Trigger DMA engine transmitter operation (equivalent to C# dmaEngine.Transmitter.DMA_SendChar(false))
                    if (data->dmaEngine && data->dmaEngine->transmitter) {
                        DMATransmitter_SendChar(data->dmaEngine->transmitter, false);
                    }
                }
            } else {
                data->txTransferStatus.bits.transmitBufferEmpty = 0;
            }
            break;

        case COM5025_PIN_OUT_TSA: // Transmitter Status Available
            if (value) {
                data->txTransferStatus.bits.transmitterUnderrun = 1;
            } else {
                data->txTransferStatus.bits.transmitterUnderrun = 0;
            }
            break;

        case COM5025_PIN_OUT_RSA: // Receiver Status Available
            if (value) {
                data->rxTransferStatus.bits.statusAvailable = 1;
            } else {
                data->rxTransferStatus.bits.statusAvailable = 0;
            }
            break;

        case COM5025_PIN_OUT_TSO: // Transmitter Serial Output
            // This is the actual bit-level output, typically handled by modem
            break;

        default:
            break;
    }

    // Check and trigger interrupts based on pin changes
    HDLC_CheckTriggerInterrupt(device);
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

static void HDLC_CheckTriggerIRQ12(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return;

    // Check if TRANSMIT interrupt is enabled for modem status change
    if (!data->txTransferControl.bits.modemStatusChangeIE) return;

    // Check if there's a signal difference between modem signals and status register
    uint16_t write_signals = data->txTransferStatus.raw & data->txModemFlagsMask.raw;
    uint16_t modem_signals = data->txModemFlags.raw & data->txModemFlagsMask.raw;

    // TMCS (Transmit Modem Status Change) set => Trigger interrupt 12 on change
    bool TMCS = (write_signals != modem_signals);
    if (TMCS) {
        Device_SetInterruptStatus(self, true, 12);
    }
}

static void HDLC_CheckTriggerIRQ13(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return;

    // Check if RECEIVE interrupt is enabled for modem status change
    if (!data->rxTransferControl.bits.modemStatusChangeIE) return;

    // Check if there's a signal difference between modem signals and status register
    uint16_t read_signals = data->rxTransferStatus.raw & data->rxModemFlagsMask.raw;
    uint16_t modem_signals = data->rxModemFlags.raw & data->rxModemFlagsMask.raw;

    // RMSC (Receive Modem Status Change) set => Trigger interrupt 13 on change
    bool RMSC = (read_signals != modem_signals);
    if (RMSC) {
        Device_SetInterruptStatus(self, true, 13);
    }
}

static void HDLC_CheckTriggerInterrupt(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return;

    /*** LEVEL 13 RX ***/
    if (data->rxTransferStatus.bits.dataAvailable) {
        if (data->rxTransferControl.bits.dataAvailableIE) {
            Device_SetInterruptStatus(self, true, 13);
        }
    }

    if (data->rxTransferStatus.bits.statusAvailable) {
        if (data->rxTransferControl.bits.statusAvailableIE) {
            Device_SetInterruptStatus(self, true, 13);
        }
    }

    /*** LEVEL 12 TX ***/
    if (data->txTransferStatus.bits.transmitBufferEmpty) {
        if (data->txTransferControl.bits.transmitBufferEmptyIE) {
            Device_SetInterruptStatus(self, true, 12);
        }
    }

    if (data->txTransferStatus.bits.transmitterUnderrun) {
        if (data->txTransferControl.bits.transmitterUnderrunIE) {
            Device_SetInterruptStatus(self, true, 12);
        }
    }
}

static void HDLC_UpdateRQTS(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data) return;

    // In maintenance mode, always set RTS
    if (data->maintenanceMode) {
        // Set modem RTS signal to true (equivalent to C# modem.SetRTS(true))
        if (data->modem) {
            Modem_SetRTS(data->modem, true);
        }
        return;
    }

    // A = Negated Signal Detect
    bool a = !(data->rxModemFlags.bits.signalDetector);

    // B = RequestToSend OR TransmitterActive, then negated
    bool b = data->txTransferControl.bits.requestToSend || data->txTransferStatus.bits.transmitterActive;
    b = !b; // Negate B

    // C = Half Duplex mode
    bool c = data->txTransferControl.bits.halfDuplex;

    // RQTS is a 3-input Negated-OR (NOR) from A, B and C
    bool new_rqts = !(a || b || c);

    // Set modem RTS signal to new_rqts value (equivalent to C# modem.SetRTS(new_rqts))
    if (data->modem) {
        Modem_SetRTS(data->modem, new_rqts);
    }
}

static void HDLC_UpdateFromCOM5025Pins(Device *self)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    if (!data || !data->com5025) return;

    bool needsInterruptCheck = false;
    bool needsRQTSUpdate = false;

    // Check SFR pin - Sync/Flag received (bit 3)
    bool sfr = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_SFR);
    if (sfr != data->rxTransferStatus.bits.syncFlagReceived) {
        data->rxTransferStatus.bits.syncFlagReceived = sfr;
    }

    // Check RXACT pin - Receiver Active (bit 2)
    bool rxact = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_RXACT);
    if (rxact != data->rxTransferStatus.bits.receiverActive) {
        data->rxTransferStatus.bits.receiverActive = rxact;
    }

    // Check RDA pin - Receiver Data Available (interrupt on level 13 if enabled)
    bool rda = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_RDA);
    if (rda != data->rxTransferStatus.bits.dataAvailable) {
        data->rxTransferStatus.bits.dataAvailable = rda;
        if (rda) {
            needsInterruptCheck = true;
        }
    }

    // Check RSA pin - Receiver STATUS available (bit 1)
    bool rsa = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_RSA);
    if (rsa != data->rxTransferStatus.bits.statusAvailable) {
        data->rxTransferStatus.bits.statusAvailable = rsa;
        if (rsa) {
            needsInterruptCheck = true;
        }
    }

    // Check TXACT pin - Transmitter Active (bit 2)
    bool txact = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_TXACT);
    if (txact != data->txTransferStatus.bits.transmitterActive) {
        data->txTransferStatus.bits.transmitterActive = txact;
        needsRQTSUpdate = true; // TXACT may impact RQTS
    }

    // Check TBMT pin - Transmitter Buffer Empty (interrupt on level 12 if enabled)
    bool tbmt = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_TBMT);
    if (tbmt != data->txTransferStatus.bits.transmitBufferEmpty) {
        data->txTransferStatus.bits.transmitBufferEmpty = tbmt;
        if (tbmt) {
            needsInterruptCheck = true;
        }
    }

    // Check TSA pin - Transmitter Status Available (TERR bit, indicating transmitter underflow)
    bool tsa = COM5025_GetOutputPin(data->com5025, COM5025_PIN_OUT_TSA);
    if (tsa != data->txTransferStatus.bits.transmitterUnderrun) {
        data->txTransferStatus.bits.transmitterUnderrun = tsa;
        if (tsa) {
            needsInterruptCheck = true;
        }
    }

    // Update RQTS if transmitter active state changed
    if (needsRQTSUpdate) {
        HDLC_UpdateRQTS(self);
    }

    // Check for interrupt triggers if any pin changed that affects interrupts
    if (needsInterruptCheck) {
        HDLC_CheckTriggerInterrupt(self);
    }

    // Handle DMA operations if enabled
    if (data->txTransferControl.bits.enableTransmitterDMA) {
        if (tbmt) {
            // DMA engine transmitter operation (equivalent to C# dmaEngine.Transmitter.DMA_SendChar(false))
            if (data->dmaEngine && data->dmaEngine->transmitter) {
                DMATransmitter_SendChar(data->dmaEngine->transmitter, false);
            }
        }
    }
}

static void HDLC_SetBaudRate(Device *self, HDLCBaudRate baudRate)
{
    if (!self) return;

    HDLCData *data = (HDLCData *)self->deviceData;
    data->baudRate = baudRate;

    // Configure actual timing based on baud rate (equivalent to C# CPU_TICKS_DIVIDER)
    // If ND-100 Clock is 40 MHz, calculate the timing divider for each baud rate
    switch (baudRate) {
        case HDLC_BAUD_307200:
            data->cpuTicksPerTx = 130;  // 40MHz / 307200
            break;
        case HDLC_BAUD_153600:
            data->cpuTicksPerTx = 260;  // 40MHz / 153600
            break;
        case HDLC_BAUD_76800:
            data->cpuTicksPerTx = 521;  // 40MHz / 76800
            break;
        case HDLC_BAUD_38400:
            data->cpuTicksPerTx = 1042; // 40MHz / 38400
            break;
        case HDLC_BAUD_19200:
            data->cpuTicksPerTx = 2083; // 40MHz / 19200
            break;
        case HDLC_BAUD_9600:
            data->cpuTicksPerTx = 4167; // 40MHz / 9600
            break;
        case HDLC_BAUD_4800:
            data->cpuTicksPerTx = 8333; // 40MHz / 4800
            break;
        case HDLC_BAUD_2400:
            data->cpuTicksPerTx = 16667; // 40MHz / 2400
            break;
        case HDLC_BAUD_1200:
            data->cpuTicksPerTx = 33333; // 40MHz / 1200
            break;
        default:
            data->cpuTicksPerTx = 4167;  // Default to 9600 bps
            break;
    }
}

// Core HDLC functionality implementation complete.
// Additional functions are implemented in their respective modules:
// - COM5025 chip: chipCOM5025.c
// - Modem: modem.c
// - DMA Engine: dmaEngine.c, dmaTransmitter.c, dmaReceiver.c
// - Control Blocks: dmaControlBlocks.c