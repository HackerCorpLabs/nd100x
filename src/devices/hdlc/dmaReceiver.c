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
    receiver->bytesReceived = 0;
    receiver->processTcpBufDelay = 0;
    receiver->onSetInterruptBit = NULL;

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
    receiver->bytesReceived = 0;
    TcpReceiveBuffer_Clear(&receiver->tcpReceiveBuffer);
}

// ---------------------------------------------------------------------------
// Tick: called every CPU cycle from DMAEngine_Tick.
// Processes ONE complete HDLC frame from the ring buffer per tick.
// Rate-limited with processTcpBufDelay to give SINTRAN time to handle IRQs.
// ---------------------------------------------------------------------------
void DMAReceiver_Tick(DMAReceiver *receiver)
{
    if (!receiver) return;

    if (receiver->processTcpBufDelay > 0) {
        receiver->processTcpBufDelay--;
        return;
    }

    if (TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer) > 0) {
        DMAReceiver_ProcessBufferedData(receiver);
        // Delay after processing - gives SINTRAN time to process IRQ
        // and send RR with correct N(R)
        receiver->processTcpBufDelay = 10000;
    }
}

// ---------------------------------------------------------------------------
// ReceiveDataFromModem: non-blocking enqueue into ring buffer.
// Called from modem layer when TCP data arrives. Returns immediately.
// ---------------------------------------------------------------------------
void DMAReceiver_ReceiveDataFromModem(DMAReceiver *receiver, const uint8_t *data, int length)
{
    if (!receiver || !data || length <= 0) return;

    int enqueued = TcpReceiveBuffer_Enqueue(&receiver->tcpReceiveBuffer, data, length);

#ifdef RX_BLAST_LOGGING
    if (enqueued < length) {
        printf("DMAReceive: TCP_RX_BUFFER_FULL: Only queued %d/%d bytes\n", enqueued, length);
    }
#else
    (void)enqueued;
#endif
}

// ---------------------------------------------------------------------------
// ProcessBufferedData: pull bytes from ring buffer until ONE complete HDLC
// frame is found, then process it and return.
// Returns bytes processed (0 = incomplete frame, waiting for more data).
// ---------------------------------------------------------------------------
int DMAReceiver_ProcessBufferedData(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice) return 0;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return 0;

    if (!hdlcData->rxTransferControl.bits.enableReceiverDMA)
        return 0;

    if (!receiver->dmaCB || !receiver->dmaCB->rxDCB)
        return 0;

    int maxBytes = TcpReceiveBuffer_Available(&receiver->tcpReceiveBuffer);
    int bytesProcessed = 0;

    while (bytesProcessed < maxBytes) {
        // Ensure we have a valid DMA buffer
        if (receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB);
        }

        if (!receiver->dmaCB->rxDCB ||
            receiver->dmaCB->rxDCB->keyValue != KEYFLAG_EMPTY_RECEIVER_BLOCK) {
            DMAReceiver_SetRXDMAFlag(receiver, RTS_RECEIVER_OVERRUN | RTS_LIST_EMPTY);
            return bytesProcessed;
        }

        uint8_t dataByte;
        if (!TcpReceiveBuffer_DequeueByte(&receiver->tcpReceiveBuffer, &dataByte))
            break;

        bool frameComplete = HDLCFrame_ProcessByte(receiver->dmaCB->hdlcReceiveFrame, dataByte);
        bytesProcessed++;

        if (frameComplete) {
            DMAReceiver_ProcessCompleteFrame(receiver);

            if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
            }
            return bytesProcessed;
        }
    }

    return 0; // Not a full packet yet
}

// ---------------------------------------------------------------------------
// ProcessCompleteFrame: validate FCS, write frame into DMA buffers, set flags.
// ---------------------------------------------------------------------------
bool DMAReceiver_ProcessCompleteFrame(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB || !receiver->dmaCB->hdlcReceiveFrame)
        return false;

    HDLCFrame *frame = receiver->dmaCB->hdlcReceiveFrame;
    const uint8_t *frameData = HDLCFrame_GetFrameData(frame);
    int frameLength = HDLCFrame_GetFrameLength(frame);

    if (!HDLCFrame_IsCRCValid(frame)) {
#ifdef RX_BLAST_LOGGING
        printf("DMAReceive: FRAME_FCS_ERROR: discarded (%d bytes)\n", frameLength);
#endif
        DMAReceiver_ClearReceiveFrameState(receiver);
        return true;
    }

    int dataLength = frameLength - 2; // Exclude 2-byte CRC
    if (dataLength <= 0) {
        DMAReceiver_ClearReceiveFrameState(receiver);
        return true;
    }

    bool writeSuccess = true;
    for (int j = 0; j < dataLength; j++) {
        DMAReceiveStatus rstat = DMAReceiver_ReceiveDataBufferByte(receiver, frameData[j]);

        if (rstat == DMA_RECEIVE_BUFFER_FULL) {
            if (!DMAReceiver_FindNextReceiveBuffer(receiver)) {
                DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY | RTS_RECEIVER_OVERRUN);
                writeSuccess = false;
                break;
            }
            j--; // Retry this byte in new buffer
        } else if (rstat != DMA_RECEIVE_OK) {
            writeSuccess = false;
            break;
        }
    }

    if (writeSuccess) {
        COM5025_WriteByte(receiver->com5025, COM5025_REG_BYTE_RECEIVER_STATUS, 0x03);
        DMAControlBlocks_MarkBufferReceived(receiver->dmaCB, 0x03);

        DMAReceiver_SetRXDMAFlag(receiver,
            RTS_FRAME_END | RTS_BLOCK_END | RTS_DATA_AVAILABLE |
            RTS_RECEIVER_ACTIVE | RTS_SYNC_FLAG_RECEIVED);
    }

    DMAReceiver_ClearReceiveFrameState(receiver);
    return writeSuccess;
}

void DMAReceiver_ClearReceiveFrameState(DMAReceiver *receiver)
{
    if (!receiver || !receiver->dmaCB || !receiver->dmaCB->hdlcReceiveFrame) return;
    HDLCFrame_Reset(receiver->dmaCB->hdlcReceiveFrame);
}

// ---------------------------------------------------------------------------
// SetReceiverState: called by CommandReceiverStart / CommandReceiverContinue
// ---------------------------------------------------------------------------
void DMAReceiver_SetReceiverState(DMAReceiver *receiver)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    hdlcData->rxTransferControl.bits.enableReceiverDMA = 1;
    DMAReceiver_EnableHDLCReceiver(receiver, true);

    hdlcData->rxTransferStatus.bits.receiverOverrun = 0;
    hdlcData->rxTransferStatus.bits.listEmpty = 0;
    hdlcData->rxTransferStatus.bits.receiverActive = 1;

    if (receiver->dmaCB && !receiver->dmaCB->rxDCB) {
        DMAControlBlocks_LoadRXBuffer(receiver->dmaCB);
    }

    DMAReceiver_FindNextReceiveBuffer(receiver);
    receiver->active = true;
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

    if (receiver->dmaCB->rxDCB &&
        receiver->dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        return true;
    }

    if (!DMAControlBlocks_LoadNextRXBuffer(receiver->dmaCB)) {
        DMAReceiver_SetRXDMAFlag(receiver, RTS_LIST_EMPTY);
        return false;
    }
    return true;
}

DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(DMAReceiver *receiver, uint8_t data)
{
    if (!receiver || !receiver->dmaCB) return DMA_RECEIVE_NO_BUFFER;

    DMAControlBlocks *dmaCB = receiver->dmaCB;
    if (!dmaCB->rxDCB) return DMA_RECEIVE_NO_BUFFER;

    int maxBlockLen = dmaCB->parameters ? dmaCB->parameters->maxReceiverBlockLength : 0;

    // Check if buffer is full BEFORE writing
    if (maxBlockLen > 0 && dmaCB->rxDCB->dmaBytesWritten >= maxBlockLen) {
        // Mark with RSOM only (frame continues to next buffer)
        DMAControlBlocks_MarkBufferReceived(dmaCB, 0x01);
        DMAReceiver_SetRXDMAFlag(receiver,
            RTS_BLOCK_END | RTS_RECEIVER_ACTIVE |
            RTS_SYNC_FLAG_RECEIVED | RTS_DATA_AVAILABLE);
        dmaCB->rxDCB = NULL;
        return DMA_RECEIVE_BUFFER_FULL;
    }

    if (dmaCB->rxDCB->keyValue == KEYFLAG_EMPTY_RECEIVER_BLOCK) {
        DMAControlBlocks_WriteNextByteDMA(dmaCB, data, true);
        receiver->bytesReceived++;
    }

    return DMA_RECEIVE_OK;
}

// ---------------------------------------------------------------------------
// SetRXDMAFlag: set flags and raise interrupt on level 13
// ---------------------------------------------------------------------------
void DMAReceiver_SetRXDMAFlag(DMAReceiver *receiver, uint16_t flag)
{
    if (!receiver || !receiver->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)receiver->hdlcDevice->deviceData;
    if (!hdlcData) return;

    // Always add SD and DSR flags (burst mode always active)
    flag |= RTS_SIGNAL_DETECTOR | RTS_DATA_SET_READY;

    if (flag & RTS_LIST_EMPTY) {
        hdlcData->rxTransferStatus.bits.receiverActive = 0;
    }

    if (receiver->dmaCB && !DMAControlBlocks_IsNextRXbufValid(receiver->dmaCB)) {
        flag |= RTS_LIST_EMPTY;
    }

    hdlcData->rxTransferStatus.raw |= flag;

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

    if (hdlcData->rxTransferControl.bits.dmaModuleIE &&
        hdlcData->rxTransferStatus.bits.dmaModuleRequest) {
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
