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

//#define DMA_DEBUG
//#define RX_BLAST_LOGGING

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
    receiver->processTcpBufDelay = 0;
    receiver->onSetInterruptBit = NULL;

    // Initialize ring buffer for TCP receive data
    TcpReceiveBuffer_Init(&receiver->tcpReceiveBuffer, TCP_RECV_BUF_DEFAULT_CAPACITY);
}

void DMAReceiver_Destroy(DMAReceiver *receiver)
{
    if (!receiver) return;

    TcpReceiveBuffer_Destroy(&receiver->tcpReceiveBuffer);
    receiver->onSetInterruptBit = NULL;
}

void DMAReceiver_Clear(DMAReceiver *receiver)
{
    if (!receiver) return;

    receiver->active = false;
    receiver->currentAddress = 0;
    receiver->bytesReceived = 0;
    TcpReceiveBuffer_Clear(&receiver->tcpReceiveBuffer);
}

// ---------------------------------------------------------------------------
// Tick: called every CPU cycle from DMAEngine_Tick
// Pull-based: processes ONE complete HDLC frame from the ring buffer per tick.
// ---------------------------------------------------------------------------
void DMAReceiver_Tick(DMAReceiver *receiver)
{
    if (!receiver) return;

    // Rate-limiting delay to give SINTRAN time to process IRQs
    if (receiver->processTcpBufDelay > 0) {
        receiver->processTcpBufDelay--;
        return;
    }

    // Pull and process one packet from buffer
    if (TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer) > 0) {
        DMAReceiver_ProcessBufferedData(receiver);
        // Delay after processing - simulates HDLC frame transmission time
        // At 1 Mbps, ~350us per frame = 14000 ticks at 40 MHz CPU
        // Gives SINTRAN time to process IRQ and send RR with correct N(R)
        receiver->processTcpBufDelay = 10000;
    }
}

// ---------------------------------------------------------------------------
// ReceiveDataFromModem: non-blocking enqueue of TCP data into ring buffer
// Called from modem layer when TCP data arrives.
// ---------------------------------------------------------------------------
void DMAReceiver_ReceiveDataFromModem(DMAReceiver *receiver, const uint8_t *data, int length)
{
    if (!receiver || !data || length <= 0) return;

    int bytesEnqueued = TcpReceiveBuffer_Enqueue(&receiver->tcpReceiveBuffer, data, length);

#ifdef RX_BLAST_LOGGING
    if (bytesEnqueued < length) {
        printf("DMAReceive: TCP_RX_BUFFER_FULL: Only queued %d/%d bytes\n", bytesEnqueued, length);
    } else {
        printf("DMAReceive: TCP_RX_QUEUED: %d bytes, buffer has %d available\n",
               bytesEnqueued, TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer));
    }
#else
    (void)bytesEnqueued;
#endif
}

// ---------------------------------------------------------------------------
// ProcessBufferedData: pull bytes from ring buffer until ONE complete HDLC
// frame is found, then process it and return.
//
// Returns bytes processed (0 = incomplete frame, waiting for more data).
// ---------------------------------------------------------------------------
int DMAReceiver_ProcessBufferedData(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice) return 0;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return 0;

    // Check if receiver DMA is enabled
    if (!hdlcData->rxTransferControl.bits.enableReceiverDMA)
        return 0;

    // Check if we have a valid RX buffer
    if (!receiver->dmaCB || !receiver->dmaCB->rxDCB) {
#ifdef RX_BLAST_LOGGING
        if (TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer) > 0) {
            printf("DMAReceive: PROCESS_SKIP: No RX buffer, %d bytes waiting\n",
                   TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer));
        }
#endif
        return 0;
    }

    int maxBytes = TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer);
    int bytesProcessed = 0;

    // Pull bytes from buffer and feed to HDLC frame state machine
    // Continue until we have a complete frame OR buffer is empty
    while (bytesProcessed < maxBytes) {

        // Ensure we have a valid DMA buffer
        if (receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            if (DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
#ifdef RX_BLAST_LOGGING
                printf("DMAReceive: BUFFER_SWITCH: Switched to buffer offset [%d]\n",
                       receiver->dmaCB->rxListPointerOffset);
#endif
            }
        }

        if (!receiver->dmaCB->rxDCB ||
            receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
#ifdef RX_BLAST_LOGGING
            printf("DMAReceive: BUFFER_EXHAUSTED: No more receive buffers\n");
#endif
            DMAReceiver_SetRXDMAFlag(receiver, RTS_RECEIVER_OVERRUN | RTS_LIST_EMPTY);
            return bytesProcessed;
        }

        // Pull next byte from ring buffer
        uint8_t dataByte;
        if (!TcpReceiveBuffer_DequeueByte(&receiver->tcpReceiveBuffer, &dataByte)) {
            break; // Buffer empty (shouldn't happen given maxBytes check)
        }

        // Feed byte to HDLC frame state machine
        bool frameComplete = HDLCFrame_ProcessByte(receiver->dmaCB->hdlcReceiveFrame, dataByte);
        bytesProcessed++;

        // Check if we have a complete frame
        if (frameComplete) {
#ifdef RX_BLAST_LOGGING
            printf("DMAReceive: PACKET_COMPLETE: Received complete HDLC frame after %d bytes\n",
                   bytesProcessed);
#endif

            // Process the complete frame
            DMAReceiver_ProcessCompleteFrame(receiver);

            // Load next RX buffer for next HDLC frame
            if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
            }

            // Exit - next tick will start fresh with new frame
            return bytesProcessed;
        }
    }

    // Buffer empty but no complete frame yet
#ifdef RX_BLAST_LOGGING
    if (bytesProcessed > 0) {
        printf("DMAReceive: PACKET_INCOMPLETE: Processed %d bytes, waiting for more\n",
               bytesProcessed);
    }
#endif

    // Return 0: not a full packet yet
    return 0;
}

// ---------------------------------------------------------------------------
// ProcessCompleteFrame: write a complete HDLC frame into DMA buffers
// ---------------------------------------------------------------------------
bool DMAReceiver_ProcessCompleteFrame(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB || !receiver->dmaCB->hdlcReceiveFrame)
        return false;

    HDLCFrame *frame = receiver->dmaCB->hdlcReceiveFrame;

    // Get frame data excluding FCS
    const uint8_t *frameData = HDLCFrame_GetFrameData(frame);
    int frameLength = HDLCFrame_GetFrameLength(frame);

    // Validate FCS/CRC
    if (!HDLCFrame_IsCRCValid(frame)) {
#ifdef RX_BLAST_LOGGING
        printf("DMAReceive: FRAME_FCS_ERROR: Frame discarded (%d bytes)\n", frameLength);
#endif
        DMAReceiver_ClearReceiveFrameState(receiver);
        return true; // Continue processing next frame
    }

    // Frame data length excluding 2-byte CRC
    int dataLength = frameLength - 2;
    if (dataLength <= 0) {
        DMAReceiver_ClearReceiveFrameState(receiver);
        return true;
    }

#ifdef RX_BLAST_LOGGING
    printf("DMAReceive: FRAME_TO_BUFFER: Writing %d bytes to buffer %d\n",
           dataLength, receiver->dmaCB->rxListPointerOffset);
#endif

    // Write frame data to DMA buffer byte by byte
    bool writeSuccess = true;
    for (int j = 0; j < dataLength; j++) {
        DMAReceiveStatus rstat = DMAReceiver_ReceiveDataBufferByte(receiver, frameData[j]);

        switch (rstat) {
            case DMA_RECEIVE_OK:
                break;
            case DMA_RECEIVE_BUFFER_FULL:
                // Buffer was full BEFORE write - byte NOT written
                // Find next buffer and retry this byte
                if (!DMAReceiver_FindNextReceiveBuffer(receiver)) {
                    DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY | RTS_RECEIVER_OVERRUN);
                    writeSuccess = false;
                }
                if (writeSuccess) {
                    j--; // Retry this byte in the new buffer
                }
                break;
            case DMA_RECEIVE_FAILED:
            case DMA_RECEIVE_NO_BUFFER:
                writeSuccess = false;
                break;
        }

        if (!writeSuccess) {
#ifdef RX_BLAST_LOGGING
            printf("DMAReceive: FRAME_WRITE_FAILED: byte %d\n", j);
#endif
            break;
        }
    }

    if (writeSuccess) {
        // Update COM5025 receiver status: RSOM and REOM
        COM5025_WriteByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS, 0x03);

        // Mark buffer as received with RSOM and REOM
        DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, 0x03);

#ifdef RX_BLAST_LOGGING
        printf("DMAReceive: FRAME_SUCCESS: Buffer offset [%d] marked received\n",
               receiver->dmaCB->rxListPointerOffset);
#endif

        // Signal frame end and block end
        uint16_t flags = RTS_FRAME_END | RTS_BLOCK_END |
                        RTS_DATA_AVAILABLE | RTS_RECEIVER_ACTIVE |
                        RTS_SYNC_FLAG_RECEIVED;
        DMAReceiver_SetRXDMAFlag(receiver, flags);
    }

    DMAReceiver_ClearReceiveFrameState(receiver);
    return writeSuccess;
}

// ---------------------------------------------------------------------------
// ClearReceiveFrameState: centralized cleanup
// ---------------------------------------------------------------------------
void DMAReceiver_ClearReceiveFrameState(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB || !receiver->dmaCB->hdlcReceiveFrame)
        return;

#ifdef RX_BLAST_LOGGING
    printf("DMAReceive: FRAME_STATE_CLEAR\n");
#endif
    HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
}

// ---------------------------------------------------------------------------
// Legacy blast function - now just delegates to ring buffer enqueue
// ---------------------------------------------------------------------------
void DMAReceiver_BlastReceiveDataBuffer(DMAReceiver *receiver, const uint8_t *data, int length)
{
    DMAReceiver_ReceiveDataFromModem(receiver, data, length);
}

// ---------------------------------------------------------------------------
// SetReceiverState: called by CommandReceiverStart and CommandReceiverContinue
// ---------------------------------------------------------------------------
void DMAReceiver_SetReceiverState(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Enable receiver DMA
    hdlcData->rxTransferControl.bits.enableReceiverDMA = 1;

    // Enable HDLC receiver hardware
    DMAReceiver_EnableHDLCReceiver(receiver, true);

    // Clear previous errors
    hdlcData->rxTransferStatus.bits.receiverOverrun = 0;
    hdlcData->rxTransferStatus.bits.listEmpty = 0;

    // Set receiver as active
    hdlcData->rxTransferStatus.bits.receiverActive = 1;

    // Load first RX buffer if not loaded
    if (receiver->dmaCB && !receiver->dmaCB->rxDCB) {
        DMAControlBlocks_LoadRXBuffer(receiver->dmaCB);
    }

    // Ensure we have a valid empty buffer
    DMAReceiver_FindNextReceiveBuffer(receiver);

    receiver->active = true;

#ifdef DMA_DEBUG
    printf("DMA SetReceiverState: EnableReceiverDMA=%d, ReceiverActive=%d\n",
           hdlcData->rxTransferControl.bits.enableReceiverDMA,
           hdlcData->rxTransferStatus.bits.receiverActive);
#endif
}

// ---------------------------------------------------------------------------
// COM5025 path (character mode - non-burst)
// ---------------------------------------------------------------------------
void DMAReceiver_ProcessByte(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver) return;
    DMAReceiver_ReceiveByteFromCOM5025(receiver, data);
}

void DMAReceiver_DataAvailableFromCOM5025(DMAReceiver *receiver)
{
    if (!receiver || !receiver->com5025) return;

    uint8_t data = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_DATA_BUFFER);
    DMAReceiver_ReceiveByteFromCOM5025(receiver, data);
}

void DMAReceiver_ReceiveByteFromCOM5025(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // In burst mode, skip COM5025 path
    if (receiver->burstMode) return;

    // Check if buffer is full
    if (receiver->dmaCB && receiver->dmaCB->rxDCB) {
        if (receiver->dmaCB->rxDCB->dmaBytesWritten >=
            hdlcData->dmaEngine->parameterBuffer.maxReceiverBlockLength) {
            uint8_t rxStatus = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS);
            DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, rxStatus);

            uint16_t flags = RTS_BLOCK_END | RTS_FRAME_END |
                            RTS_DATA_AVAILABLE | RTS_RECEIVER_ACTIVE |
                            RTS_SYNC_FLAG_RECEIVED;
            DMAReceiver_SetRXDMAFlag(receiver, flags);

            if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
                return;
            }
        }
    }

    // Write byte to current buffer
    DMAReceiver_ReceiveDataBufferByte(receiver, data);
}

void DMAReceiver_SetInterruptCallback(DMAReceiver *receiver, DMAReceiverSetInterruptCallback callback)
{
    if (!receiver) return;
    receiver->onSetInterruptBit = callback;
}

// ---------------------------------------------------------------------------
// Buffer management
// ---------------------------------------------------------------------------
bool DMAReceiver_FindNextReceiveBuffer(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB) return false;

    DMAControlBlocks *dmaCB = receiver->dmaCB;

    if (dmaCB->rxDCB && dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        return true;
    }

    if (!DMAControlBlocks_LoadNextRXBuffer(dmaCB)) {
        DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
        return false;
    }

    return true;
}

DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver || !receiver->dmaCB || !receiver->hdlcDevice) return DMA_RECEIVE_NO_BUFFER;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData || !hdlcData->dmaEngine) return DMA_RECEIVE_NO_BUFFER;

    DMAControlBlocks *dmaCB = receiver->dmaCB;

    if (!dmaCB->rxDCB) return DMA_RECEIVE_NO_BUFFER;

    // Check if buffer is full BEFORE writing
    if (dmaCB->rxDCB->dmaBytesWritten >= hdlcData->dmaEngine->parameterBuffer.maxReceiverBlockLength) {
#ifdef RX_BLAST_LOGGING
        printf("DMAReceive: BUFFER_FULL: offset[%d] full (%d/%d)\n",
               dmaCB->rxListPointerOffset,
               dmaCB->rxDCB->dmaBytesWritten,
               hdlcData->dmaEngine->parameterBuffer.maxReceiverBlockLength);
#endif
        // Mark with RSOM only (frame continues to next buffer)
        DMAControlBlocks_MarkBufferReceived(dmaCB, 0x01);

        uint16_t flags = RTS_BLOCK_END | RTS_RECEIVER_ACTIVE |
                        RTS_SYNC_FLAG_RECEIVED | RTS_DATA_AVAILABLE;
        DMAReceiver_SetRXDMAFlag(receiver, flags);

        dmaCB->rxDCB = NULL;
        return DMA_RECEIVE_BUFFER_FULL;
    }

    // Buffer has space - write the byte
    if (dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        DMAControlBlocks_WriteNextByteDMA(dmaCB, data, true);
        receiver->bytesReceived++;
    }

    return DMA_RECEIVE_OK;
}

// ---------------------------------------------------------------------------
// SetRXDMAFlag: set DMA flags and raise interrupt on level 13
// ---------------------------------------------------------------------------
void DMAReceiver_SetRXDMAFlag(DMAReceiver *receiver, uint16_t flag)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // In burst mode, always add SD and DSR flags
    if (receiver->burstMode) {
        flag |= RTS_SIGNAL_DETECTOR | RTS_DATA_SET_READY;
    }

    // Stop receiver on list empty
    if (flag & RTS_LIST_EMPTY) {
        hdlcData->rxTransferStatus.bits.receiverActive = 0;
    }

    // Check if next RX buffer is valid
    if (receiver->dmaCB && !DMAControlBlocks_IsNextRXbufValid(receiver->dmaCB)) {
        flag |= RTS_LIST_EMPTY;
    }

    hdlcData->rxTransferStatus.raw |= flag;

    // Determine if DMA module request should be set
    if (hdlcData->rxTransferStatus.bits.listEmpty) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
    }

    if (hdlcData->rxTransferControl.bits.blockEndIE && hdlcData->rxTransferStatus.bits.blockEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
    }

    if (hdlcData->rxTransferControl.bits.frameEndIE && hdlcData->rxTransferStatus.bits.frameEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
    }

    if (hdlcData->rxTransferControl.bits.listEndIE && hdlcData->rxTransferStatus.bits.listEnd) {
        hdlcData->rxTransferStatus.bits.dmaModuleRequest = 1;
    }

#ifdef DMA_DEBUG
    printf("DMA SetRXDMAFlag: %04X\n", flag);
    printf("ReceiverStatus: %04X\n", hdlcData->rxTransferStatus.raw);
#endif

    // Trigger interrupt if conditions are met
    if (hdlcData->rxTransferControl.bits.dmaModuleIE && hdlcData->rxTransferStatus.bits.dmaModuleRequest) {
#ifdef RX_BLAST_LOGGING
        printf("DMAReceive: RX_INTERRUPT: Triggering IRQ 13\n");
#endif
        if (receiver->onSetInterruptBit) {
            receiver->onSetInterruptBit(13);
        }
    }
}

void DMAReceiver_EnableHDLCReceiver(DMAReceiver *receiver, bool enable)
{
    if (!receiver || !receiver->hdlcDevice || !receiver->com5025) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    hdlcData->rxTransferControl.bits.enableReceiver = enable ? 1 : 0;
    COM5025_SetInputPin(receiver->com5025, COM5025_PIN_IN_RXENA, enable);
}

void DMAReceiver_StatusAvailableFromCOM5025(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice || !receiver->com5025) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Clear status available flag
    hdlcData->rxTransferStatus.bits.statusAvailable = 0;

    // In burst mode, don't use COM5025 status register
    if (receiver->burstMode) return;

    uint16_t rxStatus = COM5025_ReadByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS);
    uint16_t rxFlags = rxStatus << 8;

    // Check for REOM
    if (rxFlags & 0x0200) {
        if (!receiver->dmaCB->rxDCB ||
            receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY | RTS_RECEIVER_OVERRUN);
            return;
        }

        DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, (uint8_t)rxStatus);

        uint16_t flags = RTS_FRAME_END | RTS_BLOCK_END |
                        RTS_RECEIVER_ACTIVE | RTS_SYNC_FLAG_RECEIVED;
        DMAReceiver_SetRXDMAFlag(receiver, flags);

        if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
            DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
        }
    }
}
