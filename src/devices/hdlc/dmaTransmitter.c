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

#include "dmaTransmitter.h"
#include "dmaControlBlocks.h"
#include "chipCOM5025.h"
#include "dmaEngine.h"
#include "dmaEnum.h"
#include "deviceHDLC.h"
#include "hdlcFrame.h"
#include "../devices_types.h"

void DMATransmitter_Init(DMATransmitter *transmitter, void *com5025, DMAControlBlocks *dmaCB, struct Device *hdlcDevice)
{
    if (!transmitter) return;

    memset(transmitter, 0, sizeof(DMATransmitter));

    transmitter->com5025 = (COM5025State *)com5025;
    transmitter->dmaCB = dmaCB;
    transmitter->hdlcDevice = hdlcDevice;
    transmitter->active = false;
    transmitter->currentAddress = 0;
    transmitter->bytesSent = 0;
    transmitter->burstMode = false;
    transmitter->stopDelayTicks = 0;
    transmitter->onSendHDLCFrame = NULL;
    transmitter->onSetInterruptBit = NULL;
}

void DMATransmitter_Destroy(DMATransmitter *transmitter)
{
    if (!transmitter) return;

    // Clear callbacks
    transmitter->onSendHDLCFrame = NULL;
    transmitter->onSetInterruptBit = NULL;
}

void DMATransmitter_Clear(DMATransmitter *transmitter)
{
    if (!transmitter) return;

    transmitter->active = false;
    transmitter->currentAddress = 0;
    transmitter->bytesSent = 0;
}

void DMATransmitter_Tick(DMATransmitter *transmitter)
{
    if (!transmitter) return;

    if (transmitter->burstMode) {
        DMATransmitter_TickSendBlastEngine(transmitter);
    } else {
        DMATransmitter_TickSendEngine(transmitter);
    }
}

void DMATransmitter_SetTXDMAFlag(DMATransmitter *transmitter, uint16_t flag)
{
    if (!transmitter || !transmitter->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)transmitter->hdlcDevice->deviceData;
    if (!hdlcData) return;

    char irqReason[256] = "";
    hdlcData->txTransferStatus.raw |= flag;

    // Check for transmission finished
    if (flag & TTS_TRANSMISSION_FINISHED) {
        hdlcData->txTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "TransmissionFinished ");
    }

    // Check for block end interrupt
    if (hdlcData->txTransferControl.bits.blockEndIE &&
        hdlcData->txTransferStatus.bits.blockEnd) {
        hdlcData->txTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "BlockEnd ");
    }

    // Check for frame end interrupt
    if (hdlcData->txTransferControl.bits.frameEndIE &&
        hdlcData->txTransferStatus.bits.frameEnd) {
        hdlcData->txTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "FrameEnd ");
    }

    // Check for list end interrupt
    if (hdlcData->txTransferControl.bits.listEndIE &&
        hdlcData->txTransferStatus.bits.listEnd) {
        hdlcData->txTransferStatus.bits.dmaModuleRequest = 1;
        strcat(irqReason, "ListEnd ");
    }

#ifdef DMA_DEBUG
    printf("DMA SetTXDMAFlag: %04X IRQ[%s]\\n", flag, irqReason);
    printf("TransmitterStatus: %04X\\n", hdlcData->txTransferStatus.raw);
#endif

    // Trigger interrupt if conditions are met
    if (hdlcData->txTransferControl.bits.dmaModuleIE &&
        hdlcData->txTransferStatus.bits.dmaModuleRequest) {
        if (transmitter->onSetInterruptBit) {
            transmitter->onSetInterruptBit(12); // IRQ 12 for transmitter
        }
    }
}

bool DMATransmitter_SendAllBuffers(DMATransmitter *transmitter)
{
    if (!transmitter || !transmitter->dmaCB) return true;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;
    uint16_t tmpTSB = 0;

    if (dmaCB->txListPointer == 0) return true;

    // Create HDLC frame for building complete frames
    HDLCFrame frame;
    HDLCFrame_Init(&frame);

    DMAControlBlocks_LoadTXBuffer(dmaCB);

    while (true) {
        if (!dmaCB->txDCB) return true;

        // Skip already transmitted blocks
        while (dmaCB->txDCB &&
               dmaCB->txDCB->keyValue == KEYFLAG_ALREADY_TRANSMITTED_BLOCK) {
            DMAControlBlocks_LoadNextTXBuffer(dmaCB);
        }

        if (!dmaCB->txDCB) return true;

        if (dmaCB->txDCB->keyValue == KEYFLAG_BLOCK_TO_BE_TRANSMITTED) {
#ifdef DMA_DEBUG
            printf("Found block to be sent at offset %d\\n", dmaCB->txListPointerOffset);
#endif

            // Check for start of new frame
            if (DCB_HasRSOMFlag(dmaCB->txDCB)) {
                // Clear outbound buffer for new frame
                if (dmaCB->outboundBuffer) {
                    dmaCB->outboundBufferSize = 0;
                }
            }

            // Read all bytes from this block
            uint16_t bytesToSend = dmaCB->txDCB->byteCount + dmaCB->txDCB->displacement;
            while (dmaCB->txDCB->dmaBytesRead < bytesToSend) {
                // Add bytes to persistent outbound buffer
                uint8_t data = DMAControlBlocks_ReadNextByteDMA(dmaCB, false);
                if (dmaCB->outboundBuffer && dmaCB->outboundBufferSize < dmaCB->outboundBufferCapacity) {
                    dmaCB->outboundBuffer[dmaCB->outboundBufferSize++] = data;
                }
            }

            tmpTSB |= TTS_BLOCK_END;

            // Check for end of frame
            if (DCB_HasREOMFlag(dmaCB->txDCB)) {
                // Frame is complete, create HDLC frame and send it
                if (dmaCB->outboundBuffer && dmaCB->outboundBufferSize > 0) {
                    uint8_t frameBuffer[HDLC_MAX_FRAME_SIZE + 10];
                    int frameLength = HDLCFrame_BuildFrame(dmaCB->outboundBuffer,
                                                         dmaCB->outboundBufferSize,
                                                         frameBuffer,
                                                         sizeof(frameBuffer));

                    if (frameLength > 0 && transmitter->onSendHDLCFrame) {
                        // Set up frame data for callback
                        HDLCFrame callbackFrame;
                        HDLCFrame_Init(&callbackFrame);
                        memcpy(callbackFrame.frameBuffer, frameBuffer, frameLength);
                        callbackFrame.frameLength = frameLength;
                        callbackFrame.frameComplete = true;

                        transmitter->onSendHDLCFrame(&callbackFrame);
                    }
                }

                // Clear outbound buffer
                dmaCB->outboundBufferSize = 0;
                tmpTSB |= TTS_FRAME_END;
            }

            // Mark buffer as sent
            DMAControlBlocks_MarkBufferSent(dmaCB);

            // Check if there are more DCBs to process
            if (!DMAControlBlocks_LoadNextTXBuffer(dmaCB)) {
                // No more DCBs - transmission finished
                tmpTSB |= TTS_TRANSMISSION_FINISHED | TTS_LIST_END;
                DMATransmitter_SetTXDMAFlag(transmitter, tmpTSB);
                DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_STOPPED);
                dmaCB->txListPointerOffset = 0;
                return true;
            } else {
                // More DCBs to process
                DMATransmitter_SetTXDMAFlag(transmitter, tmpTSB);
                tmpTSB = 0; // Reset for next DCB
            }
        } else {
            // End of list or invalid key
#ifdef DMA_DEBUG
            if (dmaCB->txDCB->keyValue == 0x00) {
                printf("ListEnd: End of list found at offset %d\\n", dmaCB->txListPointerOffset);
            } else {
                printf("ListEnd: No more blocks at offset %d, KeyValue[0x%04X]\\n",
                       dmaCB->txListPointerOffset, dmaCB->txDCB->keyValue);
            }
#endif
            DMATransmitter_SetTXDMAFlag(transmitter, TTS_TRANSMISSION_FINISHED | TTS_LIST_END);
            DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_STOPPED);
            dmaCB->txListPointerOffset = 0;
            return true;
        }
    }
}

void DMATransmitter_SendChar(DMATransmitter *transmitter, bool isFirstChar)
{
    if (!transmitter || !transmitter->dmaCB || !transmitter->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)transmitter->hdlcDevice->deviceData;
    if (!hdlcData) return;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;

    // In burst mode, don't send char by char via COM5025 chip
    if (transmitter->burstMode) return;

    // Check if transmitter DMA is enabled
    if (!hdlcData->txTransferControl.bits.enableTransmitterDMA) return;
    if (!hdlcData->txTransferControl.bits.transmitterEnabled) return;

    // Check if we're in sending state
    if (dmaCB->dmaSenderState != DMA_SENDER_SENDING_BLOCK) return;

    // Validate current buffer key
    if (dmaCB->dmaSendBlockState != DMA_BLOCK_SEND_FRAME_END) {
        if (!dmaCB->txDCB ||
            dmaCB->txDCB->keyValue != KEYFLAG_BLOCK_TO_BE_TRANSMITTED) {
#ifdef DMA_DEBUG
            printf("DMA_SendChar: Unexpected TX key\\n");
#endif
            DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_BLOCK_READY_TO_SEND);
            return;
        }
    }

    // Handle first character in block
    if (dmaCB->dmaSendBlockState == DMA_BLOCK_IDLE) {
        if (dmaCB->txDCB->dmaBytesRead == 0) {
            DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_START_BLOCK);
            return;
        }
    }

    switch (dmaCB->dmaSendBlockState) {
        case DMA_BLOCK_IDLE:
            break;

        case DMA_BLOCK_START_BLOCK:
            // Check if this is start of new frame
            if (DCB_HasRSOMFlag(dmaCB->txDCB)) {
                // Start of message - clear TSOM in status register
                uint16_t txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
                txStatus &= ~COM5025_TX_STATUS_TSOM;
                COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
            }

            // Clear TEOM in status register
            uint16_t txStatus2 = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
            txStatus2 &= ~COM5025_TX_STATUS_TEOM;
            COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus2);
            DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_SEND_DATA);
            break;

        case DMA_BLOCK_SEND_DATA: {
            uint16_t bytesToSend = dmaCB->txDCB->byteCount + dmaCB->txDCB->displacement;

            // Check if we've sent all bytes in this block
            if (dmaCB->txDCB->dmaBytesRead >= bytesToSend) {
                // Mark buffer as sent
                DMAControlBlocks_MarkBufferSent(dmaCB);

#ifdef DMA_DEBUG
                printf("Sent block [%06X:%d]\\n",
                       dmaCB->txDCB->bufferAddress, dmaCB->txListPointerOffset);
#endif

                if (DCB_HasREOMFlag(dmaCB->txDCB)) {
                    // End of message - set TEOM
                    // Set TEOM in status register
                    uint16_t txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
                    txStatus |= COM5025_TX_STATUS_TEOM;
                    COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
                    DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_SEND_FRAME_END);
                    break;
                } else {
                    // Multi-block frame - signal block end and continue
                    DMATransmitter_SetTXDMAFlag(transmitter, TTS_BLOCK_END);

                    if (DMAControlBlocks_LoadNextTXBuffer(dmaCB)) {
                        bytesToSend = dmaCB->txDCB->byteCount + dmaCB->txDCB->displacement;
                    }
                }
            }

            // Send byte to COM5025 if not at end of buffer
            if (dmaCB->txDCB->dmaBytesRead < bytesToSend) {
                uint8_t value = DMAControlBlocks_ReadNextByteDMA(dmaCB, false);
                COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_DATA, value);
                transmitter->bytesSent++;
            } else {
#ifdef DMA_DEBUG
                printf("End of buffer reached in DMA_Send\\n");
#endif
                DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_SEND_FRAME_END);
            }
            break;
        }

        case DMA_BLOCK_SEND_FRAME_END:
#ifdef DMA_DEBUG
            printf("End of frame SYNC char has been sent. Disabling transmitter\\n");
#endif
            COM5025_SetInputPin(transmitter->com5025, COM5025_PIN_IN_TXENA, false);

            DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_SEND_FRAME_END);
            DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_FRAME_SENT);

            // Clear TSOM and TEOM if frame is complete
            if (DCB_HasREOMFlag(dmaCB->txDCB)) {
                // Clear TSOM in status register
                uint16_t txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
                txStatus &= ~COM5025_TX_STATUS_TSOM;
                COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
                // Clear TEOM in status register
            txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
            txStatus &= ~COM5025_TX_STATUS_TEOM;
            COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
            }
            break;

        default:
            break;
    }
}

void DMATransmitter_ProcessBlockReadyToSend(DMATransmitter *transmitter)
{
    if (!transmitter || !transmitter->dmaCB) return;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;

    // Check if we need to find a new block to send
    if (!dmaCB->txDCB ||
        dmaCB->txDCB->keyValue != KEYFLAG_BLOCK_TO_BE_TRANSMITTED) {

        // Re-read current buffer description
        DMAControlBlocks_LoadTXBuffer(dmaCB);

        // Scan forward for blocks to transmit
        while (dmaCB->txDCB &&
               dmaCB->txDCB->keyValue != KEYFLAG_BLOCK_TO_BE_TRANSMITTED) {
            if (!DMAControlBlocks_LoadNextTXBuffer(dmaCB)) break;

            // Check for end of list
            if (dmaCB->txDCB->keyValue == 0x00) {
                dmaCB->txListPointerOffset = 0;
                DMAControlBlocks_LoadTXBuffer(dmaCB);
                return;
            }
        }
    }

    // Do we have blocks to send?
    if (dmaCB->txDCB &&
        dmaCB->txDCB->keyValue == KEYFLAG_BLOCK_TO_BE_TRANSMITTED) {

        DMATransmitter_SetBlockSendState(transmitter, DMA_BLOCK_IDLE);
        DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_SENDING_BLOCK);

        // Enable COM5025 to start sending
        if (dmaCB->txDCB && DCB_HasRSOMFlag(dmaCB->txDCB)) {
            // Set TSOM in status register
            uint16_t txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
            txStatus |= COM5025_TX_STATUS_TSOM;
            COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
        }

        // Clear TEOM in status register
        uint16_t txStatus = COM5025_ReadByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL);
        txStatus &= ~COM5025_TX_STATUS_TEOM;
        COM5025_WriteByte(transmitter->com5025, COM5025_REG_BYTE_TRANSMITTER_STATUS_CONTROL, txStatus);
    }
}

void DMATransmitter_ProcessFrameSent(DMATransmitter *transmitter)
{
    if (!transmitter || !transmitter->dmaCB) return;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;
    uint16_t tmpTSB = 0;

#ifdef DMA_DEBUG
    printf("Starting: ProcessFrameSent\\n");
#endif

    // Set frame end flag if this was end of message
    if (dmaCB->txDCB && DCB_HasREOMFlag(dmaCB->txDCB)) {
        tmpTSB |= TTS_FRAME_END;
    }

    // Always set block end
    tmpTSB |= TTS_BLOCK_END;

    // Check if there are more buffers to send
    if (!dmaCB->txDCB || dmaCB->txDCB->keyValue == 0x00) {
        // No more buffers
        tmpTSB |= TTS_TRANSMISSION_FINISHED | TTS_LIST_END;
    } else {
        // Find next buffer to send
        if (!DMAControlBlocks_LoadNextTXBuffer(dmaCB)) {
            tmpTSB |= TTS_TRANSMISSION_FINISHED | TTS_LIST_END;
        }
    }

    // Set the flags
    if (tmpTSB != 0) {
        DMATransmitter_SetTXDMAFlag(transmitter, tmpTSB);
    }

    // Update DMA engine state
    if (!dmaCB->txDCB || dmaCB->txDCB->keyValue == 0x00) {
        DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_STOPPED);
        dmaCB->txListPointerOffset = 0;
    } else {
        DMATransmitter_SetEngineSenderState(transmitter, DMA_SENDER_BLOCK_READY_TO_SEND);
    }

#ifdef DMA_DEBUG
    printf("Ending: ProcessFrameSent\\n");
#endif
}

void DMATransmitter_TickSendBlastEngine(DMATransmitter *transmitter)
{
    if (!transmitter || !transmitter->dmaCB || !transmitter->hdlcDevice) return;

    HDLCData *hdlcData = (HDLCData *)transmitter->hdlcDevice->deviceData;
    if (!hdlcData) return;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;

    switch (dmaCB->dmaSenderState) {
        case DMA_SENDER_STOPPED:
            break;

        case DMA_SENDER_BLOCK_READY_TO_SEND:
            // Wait for DMA enabled AND transmitter enabled
            if (hdlcData->txTransferControl.bits.transmitterEnabled &&
                hdlcData->txTransferControl.bits.enableTransmitterDMA) {

                // Add delays to prevent blasting too fast
                if (dmaCB->dmaWaitTicks > 0) {
                    dmaCB->dmaWaitTicks--;
                    if (dmaCB->dmaWaitTicks == 0) {
                        if (DMATransmitter_SendAllBuffers(transmitter)) {
                            dmaCB->dmaWaitTicks = -1; // disable
                        } else {
                            dmaCB->dmaWaitTicks = 50; // Wait before next block
                        }
                    }
                }
            }
            break;

        case DMA_SENDER_SENDING_BLOCK:
            break;

        case DMA_SENDER_FRAME_SENT:
            break;

        default:
            break;
    }
}

void DMATransmitter_TickSendEngine(DMATransmitter *transmitter)
{
    if (!transmitter || !transmitter->dmaCB) return;

    DMAControlBlocks *dmaCB = transmitter->dmaCB;

    switch (dmaCB->dmaSenderState) {
        case DMA_SENDER_STOPPED:
            // Could add periodic checking for new buffers here
            break;

        case DMA_SENDER_BLOCK_READY_TO_SEND:
            DMATransmitter_ProcessBlockReadyToSend(transmitter);
            break;

        case DMA_SENDER_SENDING_BLOCK:
            // Data is sent via DMA_SendChar callback from COM5025
            break;

        case DMA_SENDER_FRAME_SENT:
            DMATransmitter_ProcessFrameSent(transmitter);
            break;

        default:
            break;
    }
}

void DMATransmitter_SetBlockSendState(DMATransmitter *transmitter, int state)
{
    if (!transmitter || !transmitter->dmaCB) return;

#ifdef DMA_DEBUG
    printf("Set_DmaBlockSendState: %d\\n", state);
#endif
    transmitter->dmaCB->dmaSendBlockState = state;
}

void DMATransmitter_SetEngineSenderState(DMATransmitter *transmitter, int state)
{
    if (!transmitter || !transmitter->dmaCB) return;

#ifdef DMA_DEBUG
    printf("Set_DmaEngineSenderState: %d\\n", state);
#endif
    transmitter->dmaCB->dmaSenderState = state;

    if (state == DMA_SENDER_BLOCK_READY_TO_SEND) {
        transmitter->dmaCB->dmaWaitTicks = 500; // Wait before starting
    }
}

void DMATransmitter_SetSenderState(DMATransmitter *transmitter, int senderState)
{
    if (!transmitter) return;

    DMATransmitter_SetEngineSenderState(transmitter, senderState);

    switch (senderState) {
        case DMA_SENDER_BLOCK_READY_TO_SEND:
            transmitter->active = true;
            break;
        case DMA_SENDER_STOPPED:
            transmitter->active = false;
            break;
        default:
            break;
    }
}

void DMATransmitter_SetSendFrameCallback(DMATransmitter *transmitter, DMATransmitterSendFrameCallback callback)
{
    if (!transmitter) return;
    transmitter->onSendHDLCFrame = callback;
}

void DMATransmitter_SetInterruptCallback(DMATransmitter *transmitter, DMATransmitterSetInterruptCallback callback)
{
    if (!transmitter) return;
    transmitter->onSetInterruptBit = callback;
}