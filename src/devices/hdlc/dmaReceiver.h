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

#ifndef DMA_RECEIVER_H
#define DMA_RECEIVER_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
struct Device;

#include "chipCOM5025.h"
#include "dmaControlBlocks.h"
#include "tcpReceiveBuffer.h"

// DMA Receiver callback function types (with context for forwarding)
typedef void (*DMAReceiverSetInterruptCallback)(void *context, uint8_t bit);

// DMA Receiver status enumeration
typedef enum {
    DMA_RECEIVE_OK,
    DMA_RECEIVE_FAILED,
    DMA_RECEIVE_BUFFER_FULL,
    DMA_RECEIVE_NO_BUFFER
} DMAReceiveStatus;

// DMA Receiver state structure
typedef struct DMAReceiver {
    // Total bytes received across all buffers (for statistics)    
    int bytesReceived;

    // Tick-based delay for rate-limiting frame processing
    int processTcpBufDelay;

    // Ring buffer for TCP receive data (pull-based architecture)
    TcpReceiveBuffer tcpReceiveBuffer;

    // Hardware references
    COM5025State *com5025;
    DMAControlBlocks *dmaCB;
    struct Device *hdlcDevice;

    // Callbacks (with context)
    DMAReceiverSetInterruptCallback onSetInterruptBit;
    void *callbackContext;

} DMAReceiver;

// Core functions
void DMAReceiver_Init(DMAReceiver *receiver, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice);
void DMAReceiver_Destroy();
void DMAReceiver_Clear();
void DMAReceiver_Tick();

// State management
void DMAReceiver_SetReceiverState();

// Data processing - TCP ring buffer path (burst mode, always active)
void DMAReceiver_ReceiveDataFromModem( const uint8_t *data, int length);
int DMAReceiver_ProcessBufferedData();
bool DMAReceiver_ProcessCompleteFrame();
void DMAReceiver_ClearReceiveFrameState();

// Buffer management
bool DMAReceiver_FindNextReceiveBuffer();
DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(uint8_t data);

// Flag and interrupt management
void DMAReceiver_SetRXDMAFlag(uint16_t flag);
void DMAReceiver_EnableHDLCReceiver(bool enable);
void DMAReceiver_SetInterruptCallback( DMAReceiverSetInterruptCallback callback);

#endif // DMA_RECEIVER_H
