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

#ifndef DMA_TRANSMITTER_H
#define DMA_TRANSMITTER_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
struct Device;

// Include for COM5025State and DMAControlBlocks
#include "chipCOM5025.h"
#include "dmaControlBlocks.h"
#include "hdlcFrame.h"

// DMA Transmitter callback function types
typedef void (*DMATransmitterSendFrameCallback)(HDLCFrame *frame);
typedef void (*DMATransmitterSetInterruptCallback)(uint8_t bit);

// DMA Transmitter state structure
typedef struct {
    // State management
    bool active;
    uint32_t currentAddress;
    int bytesSent;
    bool burstMode;
    int stopDelayTicks;

    // Hardware references
    COM5025State *com5025;
    DMAControlBlocks *dmaCB;
    struct Device *hdlcDevice;

    // Callbacks
    DMATransmitterSendFrameCallback onSendHDLCFrame;
    DMATransmitterSetInterruptCallback onSetInterruptBit;

} DMATransmitter;

// Core DMA Transmitter functions
void DMATransmitter_Init(DMATransmitter *transmitter, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice);
void DMATransmitter_Destroy(DMATransmitter *transmitter);
void DMATransmitter_Clear(DMATransmitter *transmitter);
void DMATransmitter_Tick(DMATransmitter *transmitter);

// State management
void DMATransmitter_SetSenderState(DMATransmitter *transmitter, int senderState);

// Data transmission
bool DMATransmitter_SendAllBuffers(DMATransmitter *transmitter);
void DMATransmitter_SendChar(DMATransmitter *transmitter, bool isFirstChar);
void DMATransmitter_SetTXDMAFlag(DMATransmitter *transmitter, uint16_t flag);

// Internal state machines
void DMATransmitter_ProcessBlockReadyToSend(DMATransmitter *transmitter);
void DMATransmitter_ProcessFrameSent(DMATransmitter *transmitter);
void DMATransmitter_TickSendEngine(DMATransmitter *transmitter);
void DMATransmitter_TickSendBlastEngine(DMATransmitter *transmitter);

// State setters
void DMATransmitter_SetBlockSendState(DMATransmitter *transmitter, int state);
void DMATransmitter_SetEngineSenderState(DMATransmitter *transmitter, int state);

// Callback setup functions
void DMATransmitter_SetSendFrameCallback(DMATransmitter *transmitter, DMATransmitterSendFrameCallback callback);
void DMATransmitter_SetInterruptCallback(DMATransmitter *transmitter, DMATransmitterSetInterruptCallback callback);

#endif // DMA_TRANSMITTER_H