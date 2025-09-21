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

// Include for COM5025State and DMAControlBlocks
#include "chipCOM5025.h"
#include "dmaControlBlocks.h"

// DMA Receiver callback function types
typedef void (*DMAReceiverSetInterruptCallback)(uint8_t bit);

// DMA Receiver status enumeration
typedef enum {
    DMA_RECEIVE_OK,
    DMA_RECEIVE_FAILED,
    DMA_RECEIVE_BUFFER_FULL,
    DMA_RECEIVE_NO_BUFFER
} DMAReceiveStatus;

// DMA Receiver state structure
typedef struct {
    // State management
    bool active;
    uint32_t currentAddress;
    int bytesReceived;
    bool burstMode;

    // Hardware references
    COM5025State *com5025;
    DMAControlBlocks *dmaCB;
    struct Device *hdlcDevice; // Reference to HDLC device for register access

    // Callbacks
    DMAReceiverSetInterruptCallback onSetInterruptBit;

} DMAReceiver;

// Core DMA Receiver functions
void DMAReceiver_Init(DMAReceiver *receiver, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice);
void DMAReceiver_Destroy(DMAReceiver *receiver);
void DMAReceiver_Clear(DMAReceiver *receiver);
void DMAReceiver_Tick(DMAReceiver *receiver);

// State management
void DMAReceiver_SetReceiverState(DMAReceiver *receiver);

// Data processing
void DMAReceiver_ProcessByte(DMAReceiver *receiver, uint8_t data);
void DMAReceiver_DataAvailableFromCOM5025(DMAReceiver *receiver);
void DMAReceiver_ReceiveByteFromCOM5025(DMAReceiver *receiver, uint8_t data);
void DMAReceiver_BlastReceiveDataBuffer(DMAReceiver *receiver, const uint8_t *data, int length);
void DMAReceiver_StatusAvailableFromCOM5025(DMAReceiver *receiver);

// Buffer management
bool DMAReceiver_FindNextReceiveBuffer(DMAReceiver *receiver);
DMAReceiveStatus DMAReceiver_ReceiveDataBufferByte(DMAReceiver *receiver, uint8_t data);

// Internal functions
void DMAReceiver_SetRXDMAFlag(DMAReceiver *receiver, uint16_t flag);
void DMAReceiver_EnableHDLCReceiver(DMAReceiver *receiver, bool enable);

// Callback setup functions
void DMAReceiver_SetInterruptCallback(DMAReceiver *receiver, DMAReceiverSetInterruptCallback callback);

#endif // DMA_RECEIVER_H