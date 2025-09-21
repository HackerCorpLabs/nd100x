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

#include <string.h>
#include <stdio.h>
#include "chipCOM5025.h"
#include "chipCOM5025Registers.h"
#include "hdlc_crc.h"

// Static function declarations
static void COM5025_SetOutputPin(COM5025State *chip, COM5025SignalPinOut pin, bool value);
static void COM5025_ClearAllInputPins(COM5025State *chip);
static void COM5025_SetTransmitterBufferEmpty(COM5025State *chip);
static void COM5025_ClearTransmitterBufferEmpty(COM5025State *chip);
static void COM5025_SetUnderflow(COM5025State *chip);
static void COM5025_WriteTransmitterDataBuffer(COM5025State *chip, uint8_t data);
static void COM5025_MoveTDBtoTSR(COM5025State *chip);
static void COM5025_WriteDataToTSR(COM5025State *chip, uint8_t data);
static void COM5025_WriteFlagToTSR(COM5025State *chip, uint8_t flagByte);
static void COM5025_TransmitByteOutput(COM5025State *chip, uint8_t data, bool isData);
static void COM5025_SendOneByte(COM5025State *chip, uint8_t data);
static bool COM5025_TSR_Empty(COM5025State *chip);
static uint8_t COM5025_MapBits2CharLen(uint8_t bits);
static void COM5025_SetReceiverStatus(COM5025State *chip, uint16_t newRxStatus);

// Global state for the chip (could be per-device instance)
static COM5025Registers registers;

void COM5025_Init(COM5025State *chip)
{
    if (!chip) return;

    memset(chip, 0, sizeof(COM5025State));
    COM5025Registers_Init(&registers);

    chip->mode = COM5025_MODE_BOP;
    chip->characterLength = 8;
    chip->crcRegister = 0xFFFF;

    // Initialize register pointer
    chip->registers = &registers;
}

void COM5025_Reset(COM5025State *chip)
{
    if (!chip) return;

    // RESET SIGNAL
    // This input should be pulsed high after power turn on. This will: clear all flags, and
    // status conditions, set TBMT = 1, TSO = 1 and place the device in the primary
    // BOP mode with 8 bit TX/ RX data length, CRC CCITT initialized to all 1's.

    COM5025Registers_Clear(&registers);

    // Clear input pins
    COM5025_ClearAllInputPins(chip);

    // Transmitter buffer empty
    COM5025_SetTransmitterBufferEmpty(chip);

    // Transmitter Serial Output => "MARK" (high)
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSO, true);
}

void COM5025_MasterReset(COM5025State *chip)
{
    if (!chip) return;

    COM5025_Reset(chip);

    chip->inputPins[COM5025_PIN_IN_MR] = false;
    chip->inputPins[COM5025_PIN_IN_RXENA] = false;
    chip->inputPins[COM5025_PIN_IN_TXENA] = false;
    chip->inputPins[COM5025_PIN_IN_MSEL] = false;
}

uint8_t COM5025_ReadByte(COM5025State *chip, COM5025RegistersByte reg)
{
    if (!chip) return 0;

    uint8_t data = 0;

    switch (reg) {
        case COM5025_REG_BYTE_RECEIVER_DATA_BUFFER:
            data = (uint8_t)registers.receiverDataBuffer;
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RDA, false); // turn off "Receiver Data Available"
            break;

        case COM5025_REG_BYTE_RECEIVER_STATUS:
            data = (uint8_t)(registers.receiverStatus >> 8);
            COM5025_SetReceiverStatus(chip, registers.receiverStatus & COM5025_RX_STATUS_MASK_CLEAR_ON_RSR);
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, false);
            break;

        case COM5025_REG_BYTE_TRANSMITTER_DATA:
            data = registers.transmitterDataBuffer;
            break;

        case COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL:
            data = (uint8_t)(registers.txStatusAndControl >> 8);
            break;

        case COM5025_REG_BYTE_SYNC_ADDRESS:
            data = registers.syncSecondaryAddress;
            break;

        case COM5025_REG_BYTE_MODE_CONTROL:
            data = (uint8_t)(registers.modeControl >> 8);
            break;

        case COM5025_REG_BYTE_NOT_USED:
            data = 0;
            break;

        case COM5025_REG_BYTE_DATA_LENGTH_SELECT:
            data = (uint8_t)(registers.dataLengthSelect >> 8);
            break;

        default:
            break;
    }

    return data;
}

void COM5025_WriteByte(COM5025State *chip, COM5025RegistersByte reg, uint8_t value)
{
    if (!chip) return;

    switch (reg) {
        case COM5025_REG_BYTE_RECEIVER_DATA_BUFFER:
            // The Receiver Data Buffer is a read-only register
            break;

        case COM5025_REG_BYTE_RECEIVER_STATUS:
            // Register is Read only!
            break;

        case COM5025_REG_BYTE_TRANSMITTER_DATA:
            COM5025_WriteTransmitterDataBuffer(chip, value);
            break;

        case COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL:
            // TERR bit is READ ONLY
            value &= 0x7F;
            if (registers.txStatusAndControl & COM5025_TX_STATUS_TERR)
                value |= 1 << 7;

            registers.txStatusAndControl = (uint16_t)value << 8;

            if (registers.txStatusAndControl & COM5025_TX_STATUS_TSOM) {
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSA, false); // Clear underflow
                registers.txStatusAndControl &= ~COM5025_TX_STATUS_TERR;
            }
            break;

        case COM5025_REG_BYTE_SYNC_ADDRESS:
            registers.syncSecondaryAddress = value;
            break;

        case COM5025_REG_BYTE_MODE_CONTROL:
            COM5025Registers_SetModeControl(&registers, (uint16_t)value << 8);
            break;

        case COM5025_REG_BYTE_NOT_USED:
            break;

        case COM5025_REG_BYTE_DATA_LENGTH_SELECT:
            registers.dataLengthSelect = (uint16_t)value << 8;
            registers.txdl = COM5025_MapBits2CharLen((value >> 5) & 0x07);
            registers.rxdl = COM5025_MapBits2CharLen(value & 0x07);
            break;

        default:
            break;
    }
}

uint16_t COM5025_ReadWord(COM5025State *chip, COM5025RegistersWord reg)
{
    if (!chip) return 0;

    uint16_t data = 0;

    switch (reg) {
        case COM5025_REG_WORD_RECEIVER_STATUS:
            data = (uint16_t)(registers.receiverStatus | registers.receiverDataBuffer);
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RDA, false); // turn off "Receiver Data Available"
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, false); // turn off "Receiver Status Available"
            break;

        case COM5025_REG_WORD_TRANSMITTER_STATUS:
            data = (uint16_t)(registers.txStatusAndControl | (uint8_t)registers.transmitterDataBuffer);
            break;

        case COM5025_REG_WORD_MODE_CONTROL_SYNC_ADDRESS:
            data = (uint16_t)(registers.modeControl | registers.syncSecondaryAddress);
            break;

        case COM5025_REG_WORD_DATA_LENGTH_SELECT:
            data = registers.dataLengthSelect;
            break;

        default:
            break;
    }

    return data;
}

void COM5025_WriteWord(COM5025State *chip, COM5025RegistersWord reg, uint16_t value)
{
    if (!chip) return;

    uint8_t lo = (uint8_t)(value & 0xFF);
    uint16_t hi = value & 0xFFFF;

    switch (reg) {
        case COM5025_REG_WORD_RECEIVER_STATUS:
            // It's read-only, so do nothing here
            break;

        case COM5025_REG_WORD_TRANSMITTER_STATUS:
            COM5025_WriteTransmitterDataBuffer(chip, lo);

            // TERR bit is READ ONLY - make sure it doesn't change
            uint16_t flags = hi;
            if (registers.txStatusAndControl & COM5025_TX_STATUS_TERR)
                flags |= COM5025_TX_STATUS_TERR;
            else
                flags &= ~COM5025_TX_STATUS_TERR;

            registers.txStatusAndControl = flags;
            break;

        case COM5025_REG_WORD_MODE_CONTROL_SYNC_ADDRESS:
            registers.syncSecondaryAddress = lo;
            COM5025Registers_SetModeControl(&registers, hi);
            break;

        case COM5025_REG_WORD_DATA_LENGTH_SELECT:
            registers.dataLengthSelect = hi;
            break;
    }
}

void COM5025_SetInputPin(COM5025State *chip, COM5025SignalPinIn pin, bool value)
{
    if (!chip || pin >= COM5025_MAX_IN_PINS) return;

    bool oldValue = chip->inputPins[pin];
    chip->inputPins[pin] = value;

    switch (pin) {
        case COM5025_PIN_IN_MR:
            if (value && !oldValue) {
                COM5025_MasterReset(chip);
            }
            break;

        case COM5025_PIN_IN_RXENA:
            if (!value && oldValue) {
                // Disable receiver
                // A low level disables the RDP and resets RDA, RSA and RXACT.
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RDA, false);
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, false);
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RXACT, false);

                COM5025_SetReceiverStatus(chip, registers.receiverStatus & COM5025_RX_STATUS_MASK_CLEAR_ON_RECEIVER_DISABLE);
            }
            break;

        case COM5025_PIN_IN_TXENA:
            if (value) {
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TXACT, true);
            } else {
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TXACT, false);
            }
            break;

        case COM5025_PIN_IN_MSEL:
            chip->maintenanceMode = value;
            break;

        case COM5025_PIN_IN_RCP:
        case COM5025_PIN_IN_RSI:
            break;
    }
}

bool COM5025_GetInputPin(COM5025State *chip, COM5025SignalPinIn pin)
{
    if (!chip || pin >= COM5025_MAX_IN_PINS) return false;
    return chip->inputPins[pin];
}

bool COM5025_GetOutputPin(COM5025State *chip, COM5025SignalPinOut pin)
{
    if (!chip || pin >= COM5025_MAX_OUT_PINS) return false;
    return chip->outputPins[pin];
}

void COM5025_ClockReceiver(COM5025State *chip)
{
    if (!chip || !chip->inputPins[COM5025_PIN_IN_RXENA]) return;

    bool bit = chip->inputPins[COM5025_PIN_IN_RSI];
    COM5025_ProcessBit(chip, bit);
}

void COM5025_ClockTransmitter(COM5025State *chip)
{
    if (!chip || !chip->inputPins[COM5025_PIN_IN_TXENA]) return;

    // Transmitter is enabled AND active
    if (chip->inputPins[COM5025_PIN_IN_TXENA] && chip->outputPins[COM5025_PIN_OUT_TXACT]) {
        if (COM5025_TSR_Empty(chip)) { // TSR is empty
            // Do we have data to fill up the TSR? Is transmitbufferEmpty = False?
            if (!chip->outputPins[COM5025_PIN_OUT_TBMT]) {
                COM5025_MoveTDBtoTSR(chip); // also sets TransmitBufferEmpty = true
            }
        }

        if (registers.transmitterShiftRegisterBit > 0) {
            // Check for five consecutive 1s for bit stuffing
            bool found5ones = (registers.tsrCountOnes == 5);

            // Send LSB
            bool transmitSerialOutputPin = (registers.transmitterShiftRegister & 1) != 0;

            // If we are sending DATA, bit-stuffing is enabled. When sending Flags not.
            if (registers.tsrEnableBitStuffing) {
                if (transmitSerialOutputPin) {
                    registers.tsrCountOnes++;
                } else {
                    registers.tsrCountOnes = 0;
                }
            }

            if (found5ones) {
                // Send a Zero!
                registers.tsrCountOnes = 0;
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSO, false);
            } else {
                // Normal shift
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSO, transmitSerialOutputPin);

                // shift TSR
                registers.transmitterShiftRegister = (uint8_t)(registers.transmitterShiftRegister >> 1);

                // Reduce remaining number of bits to send
                registers.transmitterShiftRegisterBit--;

                if (registers.transmitterShiftRegisterBit == 0) {
                    // Clear shift register
                    registers.transmitterShiftRegister = 0;

                    // trigger DMA after SYN byte has been sent
                    if (!registers.tsrEnableBitStuffing) {
                        COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TBMT, true);
                    }
                }
            }
        } else {
            // Transmitter is not enabled or active: TSO => "MARK" (high)
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSO, true);
        }
    }

    chip->clockCounter++;
}

void COM5025_ProcessBit(COM5025State *chip, bool bit)
{
    if (!chip) return;

    chip->receiverShiftRegister = (chip->receiverShiftRegister << 1) | (bit ? 1 : 0);
    chip->bitCounter++;

    if (chip->mode == COM5025_MODE_BOP) {
        if ((chip->receiverShiftRegister & 0xFF) == HDLC_FRAME_DELIMITER) {
            chip->flagDetected = true;
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_SFR, true);

            if (chip->bitCounter >= chip->characterLength) {
                chip->receiverStatusRegister |= COM5025_RX_STATUS_REOM;
                COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, true);
            }

            chip->bitCounter = 0;
        } else if ((chip->receiverShiftRegister & 0xFF) == HDLC_GO_AHEAD) {
            chip->abortDetected = true;
            chip->receiverStatusRegister |= COM5025_RX_STATUS_RAB_GA;
            COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, true);
            chip->bitCounter = 0;
        }
    }

    if (chip->bitCounter >= chip->characterLength) {
        chip->receiverDataBuffer = chip->receiverShiftRegister & ((1 << chip->characterLength) - 1);
        COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RDA, true);
        COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RXACT, true);

        chip->bitCounter = 0;
        chip->receiverShiftRegister = 0;
    }
}

void COM5025_ReceiveData(COM5025State *chip, const uint8_t *data, int length)
{
    if (!chip || !data || length <= 0) return;

    for (int i = 0; i < length; i++) {
        uint8_t byte = data[i];
        COM5025Registers_QueueReceivedData(&registers, byte);
    }
}

void COM5025_TransmitData(COM5025State *chip, uint8_t data)
{
    if (!chip) return;

    chip->transmitterDataRegister = data;
    chip->transmitterShiftRegister = data;
    chip->bitCounter = chip->characterLength;
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TBMT, false);
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TXACT, true);
}

// Static helper functions

static void COM5025_SetOutputPin(COM5025State *chip, COM5025SignalPinOut pin, bool value)
{
    if (!chip || pin >= COM5025_MAX_OUT_PINS) return;

    bool oldValue = chip->outputPins[pin];
    chip->outputPins[pin] = value;

    // Notify callback of pin value change
    if (oldValue != value && chip->onPinValueChanged) {
        chip->onPinValueChanged(chip->callbackContext, pin, value);
    }
}

static void COM5025_ClearAllInputPins(COM5025State *chip)
{
    if (!chip) return;
    for (int i = 0; i < COM5025_MAX_IN_PINS; i++) {
        chip->inputPins[i] = false;
    }
}

static void COM5025_SetTransmitterBufferEmpty(COM5025State *chip)
{
    if (!chip) return;
    // let host know Transmit buffer is empty and ready for filling
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TBMT, true);
}

static void COM5025_ClearTransmitterBufferEmpty(COM5025State *chip)
{
    if (!chip) return;
    // Transmitter buffer NOT empty anymore (we have data!)
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TBMT, false);
}

static void COM5025_SetUnderflow(COM5025State *chip)
{
    if (!chip) return;
    // Underflow, set high when TDB not loaded in time to maintain continuous transmission
    registers.txStatusAndControl |= COM5025_TX_STATUS_TERR;
    COM5025_SetOutputPin(chip, COM5025_PIN_OUT_TSA, true);
}

static void COM5025_WriteTransmitterDataBuffer(COM5025State *chip, uint8_t data)
{
    if (!chip) return;

    registers.transmitterDataBuffer = data;

    // When you write a byte to the Transmitter Data Buffer (TDB), the TBMT signal is cleared
    // TBMT = 0 on any write access to TDB or 'TX Status and Control Register'
    COM5025_ClearTransmitterBufferEmpty(chip);
}

static void COM5025_MoveTDBtoTSR(COM5025State *chip)
{
    if (!chip) return;

    COM5025_WriteDataToTSR(chip, registers.transmitterDataBuffer);
    registers.transmitterDataBuffer = 0;

    // Tell host that TDB is empty and ready to accept the next byte
    COM5025_SetTransmitterBufferEmpty(chip);
}

static void COM5025_WriteDataToTSR(COM5025State *chip, uint8_t data)
{
    if (!chip) return;

    // Transmit data for the BYTE oriented connection, and calculate CRC
    COM5025_TransmitByteOutput(chip, data, true);

    // Shift register logic starts here
    registers.transmitterShiftRegister = data;
    registers.transmitterShiftRegisterBit = 8;
    registers.tsrCountOnes = 0;
    registers.tsrEnableBitStuffing = true; // enable bit stuffing
}

static void COM5025_WriteFlagToTSR(COM5025State *chip, uint8_t flagByte)
{
    if (!chip) return;

    // Transmit data for the BYTE oriented way, and calculate CRC
    COM5025_TransmitByteOutput(chip, flagByte, false);

    // Shift register logic starts here
    registers.transmitterShiftRegister = flagByte;
    registers.transmitterShiftRegisterBit = 8;
    registers.tsrCountOnes = 0;
    registers.tsrEnableBitStuffing = false; // disable bit stuffing

    // NOTE! Do not set TDR empty flag, writing FLAG doesn't touch the TDR register
}

static void COM5025_TransmitByteOutput(COM5025State *chip, uint8_t data, bool isData)
{
    if (!chip) return;

    if (isData)
        COM5025Registers_AggregateTXCrc(&registers, data);

    // if we are sending data, we might need to do byte stuffing
    if (isData) {
        // If the byte is a control octet (Frame Boundary or Escape Octet), it needs to be escaped
        if ((COM5025Registers_IsProtocolModeCCP(&registers) == false) &&
            (data == HDLC_FRAME_DELIMITER || data == HDLC_ASYNC_ESCAPE_OCTET)) {
            // Send Escape Octet
            COM5025_SendOneByte(chip, HDLC_ASYNC_ESCAPE_OCTET);

            // Send the data byte with bit 5 inverted
            COM5025_SendOneByte(chip, (uint8_t)(data ^ HDLC_ASYNC_INVERT_OCTET));
        } else {
            // For any other data, send as-is
            COM5025_SendOneByte(chip, data);
        }
    } else {
        COM5025_SendOneByte(chip, data);
    }
}

static void COM5025_SendOneByte(COM5025State *chip, uint8_t data)
{
    if (!chip) return;

    // Loopback?
    if (chip->inputPins[COM5025_PIN_IN_MSEL]) { // maintenance mode?
        COM5025Registers_QueueReceivedData(&registers, data);
    }

    // Send it! (even if maintenance mode is enabled, we want to know the output)
    if (chip->onTransmitterOutput) {
        chip->onTransmitterOutput(chip->callbackContext, data);
    }
}

static bool COM5025_TSR_Empty(COM5025State *chip)
{
    if (!chip) return true;
    return registers.transmitterShiftRegisterBit == 0;
}

static uint8_t COM5025_MapBits2CharLen(uint8_t bits)
{
    if (bits == 0)
        return 8;
    else
        return bits;
}

static void COM5025_SetReceiverStatus(COM5025State *chip, uint16_t newRxStatus)
{
    if (!chip) return;

    // Apply the mask to ignore RSOM in both statuses
    uint16_t maskedOriginalStatus = registers.receiverStatus & ~COM5025_RX_STATUS_RSOM;
    uint16_t maskedNewStatus = newRxStatus & ~COM5025_RX_STATUS_RSOM;

    // Find bits that were 0 and are now 1, excluding RSOM
    uint16_t bitsFrom0To1 = (~maskedOriginalStatus & maskedNewStatus);

    COM5025Registers_SetReceiverStatus(&registers, newRxStatus);

    // Do we have bits going to 1?
    if (bitsFrom0To1 != 0) {
        COM5025_SetOutputPin(chip, COM5025_PIN_OUT_RSA, true); // trigger RSA
    }
}

// Callback setup functions
void COM5025_SetTransmitterOutputCallback(COM5025State *chip, void (*callback)(void *context, uint8_t data), void *context)
{
    if (!chip) return;
    chip->onTransmitterOutput = callback;
    chip->callbackContext = context;
}

void COM5025_SetPinValueChangedCallback(COM5025State *chip, void (*callback)(void *context, COM5025SignalPinOut pin, bool value), void *context)
{
    if (!chip) return;
    chip->onPinValueChanged = callback;
    chip->callbackContext = context;
}