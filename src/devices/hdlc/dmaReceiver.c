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
#include <stdint.h>
#include <stdbool.h>

#include "dmaReceiver.h"
#include "dmaControlBlocks.h"
#include "chipCOM5025.h"
#include "deviceHDLC.h"
#include "../devices_types.h"

void DMAReceiver_Init(DMAReceiver *receiver, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice)
{
    if (!receiver) return;

    memset(receiver, 0, sizeof(DMAReceiver));

    receiver->com5025 = (COM5025State *)com5025;
    receiver->dmaCB = dmaCB;
    receiver->hdlcDevice = hdlcDevice;
    receiver->active = false;
    receiver->currentAddress = 0;
    receiver->bytesReceived = 0;
    receiver->burstMode = false;
    receiver->onSetInterruptBit = NULL;
}

void DMAReceiver_Destroy(DMAReceiver *receiver)
{
    if (!receiver) return;

    // Clear callbacks
    receiver->onSetInterruptBit = NULL;
}

void DMAReceiver_Clear(DMAReceiver *receiver)
{
    if (!receiver) return;

    receiver->active = false;
    receiver->currentAddress = 0;
    receiver->bytesReceived = 0;
}

void DMAReceiver_Tick(DMAReceiver *receiver)
{
    if (!receiver || !receiver->active) return;

    // Check if data is available from COM5025
    if (receiver->com5025) {
        // This would be called when COM5025 signals data available
        // In practice, the COM5025 would trigger an interrupt or callback
        // when data is ready, rather than polling here
    }
}

void DMAReceiver_SetReceiverState(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Called by CommandReceiverStart and CommandReceiverContinue
    // Enable the HDLC receiver and ensure DMA is ready for incoming data

    // Enable receiver DMA - this allows the receiver to process incoming data
    hdlcData->rxTransferControl.bits.enableReceiverDMA = 1;

    // Enable the HDLC receiver hardware
    DMAReceiver_EnableHDLCReceiver(receiver, true);

    // Clear any previous receiver overrun or error states
    hdlcData->rxTransferStatus.bits.receiverOverrun = 0;
    hdlcData->rxTransferStatus.bits.listEmpty = 0;

    // Set receiver as active and ready to receive
    hdlcData->rxTransferStatus.bits.receiverActive = 1;

    // Find the first empty receive buffer if not already loaded
    if (receiver->dmaCB) {
        // TODO: Load RX buffer from DMA control blocks
        // DMAControlBlocks_LoadRXBuffer(receiver->dmaCB);
    }

    // Ensure we have a valid empty buffer to start receiving into
    DMAReceiver_FindNextReceiveBuffer(receiver);

    receiver->active = true;

#ifdef DMA_DEBUG
    printf("DMA SetReceiverState: EnableReceiverDMA=%d, ReceiverActive=%d\n",
           hdlcData->rxTransferControl.bits.enableReceiverDMA,
           hdlcData->rxTransferStatus.bits.receiverActive);
#endif
}

void DMAReceiver_ProcessByte(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver) return;

    // Process received byte through HDLC frame assembly
    // In the C# version, this calls ReceiveByteFromCOM5025
    DMAReceiver_ReceiveByteFromCOM5025(receiver, data);
}

void DMAReceiver_DataAvailableFromCOM5025(DMAReceiver *receiver)
{
    if (!receiver || !receiver->com5025) return;

    // Read data from COM5025 receiver data buffer
    uint8_t data = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_DATA_BUFFER);
    DMAReceiver_ReceiveByteFromCOM5025(receiver, data);
}

void DMAReceiver_ReceiveByteFromCOM5025(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // In burst mode we do not use the COM5025 to transmit data, we send it directly via TCP for speed
    if (receiver->burstMode) {
        return;
    }

    // Check if buffer is full and needs to be switched
    if (receiver->dmaCB) {
        // TODO: Check if buffer is full
        // if (dmaRegs.RX_DCB.dmaBytesWritten >= dmaRegs.Parameters.MaxReceiverBlockLength)
        bool bufferFull = false; // Placeholder

        if (bufferFull) {
            // Buffer filled up? Mark it as complete and find next receive buffer
            uint8_t rxStatus = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS);
            // TODO: Mark buffer as received
            // DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, rxStatus);

            // Tell ND that the Block has ended
            uint16_t flags = (1 << 8) | (1 << 9) | (1 << 0) | (1 << 2) | (1 << 3); // BlockEnd | FrameEnd | DataAvailable | ReceiverActive | SyncFlagReceived
            DMAReceiver_SetRXDMAFlag(receiver, flags);

            // Move to next buffer
            DMAReceiver_FindNextReceiveBuffer(receiver);
        }
    }

    // Write the byte to the current buffer
    DMAReceiver_ReceiveDataBufferByte(receiver, data);
}

void DMAReceiver_SetInterruptCallback(DMAReceiver *receiver, DMAReceiverSetInterruptCallback callback)
{
    if (!receiver) return;
    receiver->onSetInterruptBit = callback;
}

// Additional function implementations

bool DMAReceiver_FindNextReceiveBuffer(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB) return false;

    DMAControlBlocks *dmaCB = receiver->dmaCB;

    // Check if current buffer is already empty
    if (dmaCB->rxDCB && dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        return true;
    }

    // Scan for next empty receive buffer
    while (true) {
        // Load next RX buffer description
        if (!DMAControlBlocks_LoadNextRXBuffer(dmaCB)) {
            break;
        }

        if (!dmaCB->rxDCB) {
            return false;
        }

        // Skip full receiver blocks
        if (dmaCB->rxDCB->keyValue == KEYFLAG_FULL_RECEIVER_BLOCK) {
            continue;
        }

        // Found empty or other state
        break;
    }

    if (!dmaCB->rxDCB) {
        return false;
    }

    // Handle list wraparound
    if (dmaCB->rxDCB->keyValue == KEYFLAG_NEW_LIST_POINTER) {
        dmaCB->rxListPointerOffset = 0;
        DMAControlBlocks_LoadRXBuffer(dmaCB);

        if (dmaCB->rxDCB && dmaCB->rxDCB->keyValue == KEYFLAG_FULL_RECEIVER_BLOCK) {
            // List wrapped but first buffer is full - list empty condition
            DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
            return false;
        }
    }

    // Check if we found an empty buffer
    if (!dmaCB->rxDCB || dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        dmaCB->rxListPointerOffset = 0;
        return false;
    }

    return true;
}

DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver || !receiver->dmaCB) return DMA_RECEIVE_NO_BUFFER;

    DMAControlBlocks *dmaCB = receiver->dmaCB;

    if (!dmaCB->rxDCB) {
        return DMA_RECEIVE_NO_BUFFER;
    }

    // Check if we have a valid empty buffer
    if (dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        // Write byte to memory via DMA
        DMAControlBlocks_WriteNextByteDMA(dmaCB, data, true);
        receiver->bytesReceived++;
    }

    // Check if buffer is full
    if (dmaCB->rxDCB->dmaBytesWritten >= dmaCB->rxDCB->byteCount) {
        // Buffer is full - mark as received and signal buffer end
        uint8_t rxStatus = 0x01; // RSOM (Receiver Start Of Message)
        DMAControlBlocks_MarkBufferReceived(dmaCB, rxStatus);

        // Signal block end but not frame end
        uint16_t flags = RTS_BLOCK_END | RTS_RECEIVER_ACTIVE |
                        RTS_SYNC_FLAG_RECEIVED | RTS_DATA_AVAILABLE;
        DMAReceiver_SetRXDMAFlag(receiver, flags);

        dmaCB->rxDCB = NULL;
        return DMA_RECEIVE_BUFFER_FULL;
    }

    return DMA_RECEIVE_OK;
}

void DMAReceiver_SetRXDMAFlag(DMAReceiver *receiver, uint16_t flag)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Set the flags in receiver transfer status
    hdlcData->rxTransferStatus.raw |= flag;

    // Check for conditions that trigger DMA module request
    bool triggerDMARequest = false;
    char irqReason[256] = "";

    if (hdlcData->rxTransferStatus.bits.listEmpty) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "ListEmpty ");
        triggerDMARequest = true;
    }

    if (hdlcData->rxTransferControl.bits.blockEndIE && hdlcData->rxTransferStatus.bits.blockEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "BlockEnd ");
        triggerDMARequest = true;
    }

    if (hdlcData->rxTransferControl.bits.frameEndIE && hdlcData->rxTransferStatus.bits.frameEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "FrameEnd ");
        triggerDMARequest = true;
    }

    if (hdlcData->rxTransferControl.bits.listEndIE && hdlcData->rxTransferStatus.bits.listEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "ListEnd ");
        triggerDMARequest = true;
    }

#ifdef DMA_DEBUG
    printf("DMA SetRXDMAFlag: %04X IRQ[%s]\\n", flag, irqReason);
    printf("ReceiverStatus: %04X\\n", hdlcData->rxTransferStatus.raw);
#endif

    // Trigger interrupt if conditions are met
    if (hdlcData->rxTransferControl.bits.dmaModuleIE && hdlcData->rxTransferStatus.bits.dmaModuleRequest) {
        if (receiver->onSetInterruptBit) {
            receiver->onSetInterruptBit(13); // IRQ 13 for receiver
        }
    }
}

void DMAReceiver_EnableHDLCReceiver(DMAReceiver *receiver, bool enable)
{
    if (!receiver || !receiver->hdlcDevice || !receiver->com5025) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Enable/disable receiver in control register
    hdlcData->rxTransferControl.bits.enableReceiver = enable ? 1 : 0;

    // Set COM5025 RXENA pin
    COM5025_SetInputPin(receiver->com5025, COM5025_PIN_IN_RXENA, enable);
}

void DMAReceiver_BlastReceiveDataBuffer(DMAReceiver *receiver, const uint8_t *data, int length)
{
    if (!receiver || !data || length <= 0) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

#ifdef DMA_DEBUG
    printf("TCP_RX: %d bytes received\\n", length);
#endif

    // Check if receiver DMA is enabled
    if (!hdlcData->rxTransferControl.bits.enableReceiverDMA) {
        // Clear HDLC frame to prevent contamination
        if (receiver->dmaCB && receiver->dmaCB->hdlcReceiveFrame) {
            HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
        }
        return;
    }

    if (!receiver->dmaCB || !receiver->dmaCB->rxDCB) {
        // No RX buffer available
        if (receiver->dmaCB && receiver->dmaCB->hdlcReceiveFrame) {
            HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
        }
        return;
    }

    // Process each byte through HDLC frame assembly
    for (int i = 0; i < length; i++) {
        // Ensure we have a valid buffer
        if (!receiver->dmaCB->rxDCB ||
            receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
                // No more buffers available
                if (receiver->dmaCB->hdlcReceiveFrame) {
                    HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
                }
                DMAReceiver_SetRXDMAFlag(receiver, RTS_RECEIVER_OVERRUN);
                return;
            }
        }

        // Process byte through HDLC frame state machine
        bool frameComplete = HDLCFrame_ProcessByte(receiver->dmaCB->hdlcReceiveFrame, data[i]);

        if (frameComplete) {
            // Frame is complete, check if CRC is valid
            if (HDLCFrame_IsCRCValid(receiver->dmaCB->hdlcReceiveFrame)) {
                // Get frame data (excluding FCS)
                const uint8_t *frameData = HDLCFrame_GetFrameData(receiver->dmaCB->hdlcReceiveFrame);
                int frameLength = HDLCFrame_GetFrameLength(receiver->dmaCB->hdlcReceiveFrame);

                    // Write frame data to DMA buffer (excluding CRC bytes)
                    bool writeSuccess = true;
                    for (int j = 0; j < frameLength - 2; j++) {
                        DMAReceiveStatus status = DMAReceiver_ReceiveDataBufferByte(receiver, frameData[j]);
                        if (status == DMA_RECEIVE_BUFFER_FULL) {
                            // Find next buffer for remaining bytes
                            if (!DMAReceiver_FindNextReceiveBuffer(receiver)) {
                                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY | RTS_RECEIVER_OVERRUN);
                                writeSuccess = false;
                                break;
                            }
                        } else if (status != DMA_RECEIVE_OK) {
                            writeSuccess = false;
                            break;
                        }
                    }

                    if (writeSuccess) {
                        // Mark buffer as received with RSOM and REOM
                        DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, 0x03);

                        // Update COM5025 receiver status
                        COM5025_WriteByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS, 0x03);

                        // Signal frame end and block end
                        uint16_t flags = RTS_FRAME_END | RTS_BLOCK_END |
                                        RTS_DATA_AVAILABLE | RTS_RECEIVER_ACTIVE |
                                        RTS_SYNC_FLAG_RECEIVED;
                        DMAReceiver_SetRXDMAFlag(receiver, flags);
                    }
                } else {
                    // CRC error - discard frame
#ifdef DMA_DEBUG
                    printf("HDLC Frame CRC error - frame discarded\\n");
#endif
                }

                // Clear frame for next reception
                HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
            }
        }
    }

void DMAReceiver_StatusAvailableFromCOM5025(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice || !receiver->com5025) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Clear status available flag
    hdlcData->rxTransferStatus.bits.statusAvailable = 0;

    // In burst mode, don't use COM5025 status register
    if (receiver->burstMode) {
        return;
    }

    // Read receiver status from COM5025
    uint16_t rxStatus = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS);
    uint16_t rxFlags = rxStatus << 8; // Convert to ReceiverStatusFlags format

    // Check for REOM (Receiver End Of Message)
    if (rxFlags & 0x0200) { // REOM bit
        // Check if we have a valid buffer
        if (!receiver->dmaCB->rxDCB ||
            receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            // No valid buffer - receiver overrun
            DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY | RTS_RECEIVER_OVERRUN);
            return;
        }

        // Mark buffer as received
        DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, (uint8_t)rxStatus);

        // Signal frame end and block end
        uint16_t flags = RTS_FRAME_END | RTS_BLOCK_END |
                        RTS_RECEIVER_ACTIVE | RTS_SYNC_FLAG_RECEIVED;
        DMAReceiver_SetRXDMAFlag(receiver, flags);

        // Load next receiver buffer
        DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB);

        // Handle list wraparound
        if (receiver->dmaCB->rxDCB &&
            receiver->dmaCB->rxDCB->keyValue == KEYFLAG_NEW_LIST_POINTER) {

            receiver->dmaCB->rxListPointerOffset = 0;
            DMAControlBlocks_LoadRXBuffer(receiver->dmaCB);

            // Update receiver state
            DMAReceiver_SetReceiverState(receiver);

            // Check if new buffer is full (list empty condition)
            if (receiver->dmaCB->rxDCB &&
                receiver->dmaCB->rxDCB->keyValue == KEYFLAG_FULL_RECEIVER_BLOCK) {
                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
            }
        }
    }

    // Check for ROR (Receiver Overrun)
    if (rxFlags & 0x0400) { // ROR bit
        // Handle receiver overrun
        DMAReceiver_SetRXDMAFlag(receiver, RTS_RECEIVER_OVERRUN);
    }
}