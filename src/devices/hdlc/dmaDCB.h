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

#ifndef DMA_DCB_H
#define DMA_DCB_H

#include <stdint.h>
#include <stdbool.h>
#include "hdlc_constants.h"
#include "dmaEnum.h"

/**
 * @brief DMA Control Block (DCB)
 *
 * In memory transmit and receive buffers
 * This structure represents a DMA Control Block used for HDLC data transfers.
 */
typedef struct {
    /**
     * @brief Where in memory was this buffer description loaded from
     */
    uint32_t bufferAddress;

    /**
     * @brief For this Buffer description, what is the offset (in blocks) from the start list pointer
     */
    uint16_t offsetFromLP;

    /**
     * @brief KEY value containing flags and control information
     */
    uint16_t keyValue;

    /**
     * @brief Byte count - Number of information bytes
     */
    uint16_t byteCount;

    /**
     * @brief Data Address - Most significant part
     */
    uint16_t mostAddress;

    /**
     * @brief Data Address - Least significant part
     */
    uint16_t leastAddress;

    /**
     * @brief Displacement value
     */
    uint16_t displacement;

    /**
     * @brief List Pointer value
     */
    uint32_t listPointer;

    // Helper fields for DMA transfer

    /**
     * @brief Must be set to the Memory address we should read from
     */
    uint32_t dmaAddress;

    /**
     * @brief Number of bytes read during DMA transfer
     */
    int dmaBytesRead;

    /**
     * @brief Number of bytes written during DMA transfer
     */
    int dmaBytesWritten;

    /**
     * @brief Last DMA read from ND. Will be -1 if its not read
     */
    int dmaReadData;

} DCB;

// Function declarations

/**
 * @brief Initialize a DCB structure to default values
 * @param dcb Pointer to DCB structure to initialize
 */
void DCB_Init(DCB *dcb);

/**
 * @brief Clear a DCB structure (same as Init)
 * @param dcb Pointer to DCB structure to clear
 */
void DCB_Clear(DCB *dcb);

/**
 * @brief Get the Key flags from the KeyValue
 * @param dcb Pointer to DCB structure
 * @return Key flags masked with KEYFLAG_MASK_KEY
 */
KeyFlags DCB_GetKey(const DCB *dcb);

/**
 * @brief Check if DCB has Receiver Start of Message flag
 * @param dcb Pointer to DCB structure
 * @return true if RSOM flag is set, false otherwise
 */
bool DCB_HasRSOMFlag(const DCB *dcb);

/**
 * @brief Check if DCB has Receiver End of Message flag
 * @param dcb Pointer to DCB structure
 * @return true if REOM flag is set, false otherwise
 */
bool DCB_HasREOMFlag(const DCB *dcb);

/**
 * @brief Get the DataFlow Cost from KeyValue
 * @param dcb Pointer to DCB structure
 * @return DataFlow Cost value
 */
uint16_t DCB_GetDataFlowCost(const DCB *dcb);

/**
 * @brief Get the combined 24-bit data memory address
 * @param dcb Pointer to DCB structure
 * @return Combined address from mostAddress and leastAddress
 */
uint32_t DCB_GetDataMemoryAddress(const DCB *dcb);

/**
 * @brief Set the 24-bit data memory address
 * @param dcb Pointer to DCB structure
 * @param address 24-bit address to set
 */
void DCB_SetDataMemoryAddress(DCB *dcb, uint32_t address);

// Accessor functions

void DCB_SetBufferAddress(DCB *dcb, uint32_t address);
uint32_t DCB_GetBufferAddress(const DCB *dcb);

void DCB_SetOffsetFromLP(DCB *dcb, uint16_t offset);
uint16_t DCB_GetOffsetFromLP(const DCB *dcb);

void DCB_SetKeyValue(DCB *dcb, uint16_t keyValue);
uint16_t DCB_GetKeyValue(const DCB *dcb);

void DCB_SetByteCount(DCB *dcb, uint16_t byteCount);
uint16_t DCB_GetByteCount(const DCB *dcb);

void DCB_SetDisplacement(DCB *dcb, uint16_t displacement);
uint16_t DCB_GetDisplacement(const DCB *dcb);

void DCB_SetListPointer(DCB *dcb, uint32_t listPointer);
uint32_t DCB_GetListPointer(const DCB *dcb);

// DMA helper functions

void DCB_SetDMAAddress(DCB *dcb, uint32_t address);
uint32_t DCB_GetDMAAddress(const DCB *dcb);

void DCB_SetDMABytesRead(DCB *dcb, int bytesRead);
int DCB_GetDMABytesRead(const DCB *dcb);

void DCB_SetDMABytesWritten(DCB *dcb, int bytesWritten);
int DCB_GetDMABytesWritten(const DCB *dcb);

void DCB_SetDMAReadData(DCB *dcb, int data);
int DCB_GetDMAReadData(const DCB *dcb);

bool DCB_IsDMAReadDataValid(const DCB *dcb);
void DCB_ClearDMAReadData(DCB *dcb);

// Debug/utility functions

/**
 * @brief Print DCB contents for debugging
 * @param dcb Pointer to DCB structure to print
 */
void DCB_Print(const DCB *dcb);

#endif // DMA_DCB_H