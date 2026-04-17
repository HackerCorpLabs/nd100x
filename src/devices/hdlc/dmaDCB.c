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

#include "dmaDCB.h"
#include "hdlc_constants.h"

void DCB_Init(DCB *dcb)
{
    if (!dcb) return;

    memset(dcb, 0, sizeof(DCB));

    // Initialize to default values
    dcb->bufferAddress = 0;
    dcb->offsetFromLP = 0;
    dcb->keyValue = 0;
    dcb->byteCount = 0;
    dcb->mostAddress = 0;
    dcb->leastAddress = 0;
    dcb->displacement = 0;
    dcb->listPointer = 0;

    // Initialize helper fields for DMA transfer
    dcb->dmaAddress = 0;
    dcb->dmaBytesRead = 0;
    dcb->dmaBytesWritten = 0;
    dcb->dmaReadData = -1; // -1 indicates not read
}

void DCB_Clear(DCB *dcb)
{
    if (!dcb) return;

    DCB_Init(dcb);
}

KeyFlags DCB_GetKey(const DCB *dcb)
{
    if (!dcb) return 0;

    return (KeyFlags)(dcb->keyValue & KEYFLAG_MASK_KEY);
}

bool DCB_HasRSOMFlag(const DCB *dcb)
{
    if (!dcb) return false;

    return (dcb->keyValue & KEYFLAG_RCOST_RSOM) != 0;
}

bool DCB_HasREOMFlag(const DCB *dcb)
{
    if (!dcb) return false;

    return (dcb->keyValue & KEYFLAG_RCOST_REOM) != 0;
}

uint16_t DCB_GetDataFlowCost(const DCB *dcb)
{
    if (!dcb) return 0;

    return (uint16_t)(dcb->keyValue & KEYFLAG_MASK_DATAFLOW_COST);
}

uint32_t DCB_GetDataMemoryAddress(const DCB *dcb)
{
    if (!dcb) return 0;

    // Originally 18 bit memory address.. Lets be a bit more open to potentially bigger memories like 24bit
    return (uint32_t)(dcb->mostAddress & 0x00FF) << 16 | dcb->leastAddress;
}

void DCB_SetDataMemoryAddress(DCB *dcb, uint32_t address)
{
    if (!dcb) return;

    dcb->leastAddress = (uint16_t)(address & 0xFFFF);
    dcb->mostAddress = (uint16_t)((address >> 16) & 0x00FF);
}

void DCB_SetBufferAddress(DCB *dcb, uint32_t address)
{
    if (!dcb) return;

    dcb->bufferAddress = address;
}

uint32_t DCB_GetBufferAddress(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->bufferAddress;
}

void DCB_SetOffsetFromLP(DCB *dcb, uint16_t offset)
{
    if (!dcb) return;

    dcb->offsetFromLP = offset;
}

uint16_t DCB_GetOffsetFromLP(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->offsetFromLP;
}

void DCB_SetKeyValue(DCB *dcb, uint16_t keyValue)
{
    if (!dcb) return;

    dcb->keyValue = keyValue;
}

uint16_t DCB_GetKeyValue(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->keyValue;
}

void DCB_SetByteCount(DCB *dcb, uint16_t byteCount)
{
    if (!dcb) return;

    dcb->byteCount = byteCount;
}

uint16_t DCB_GetByteCount(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->byteCount;
}

void DCB_SetDisplacement(DCB *dcb, uint16_t displacement)
{
    if (!dcb) return;

    dcb->displacement = displacement;
}

uint16_t DCB_GetDisplacement(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->displacement;
}

void DCB_SetListPointer(DCB *dcb, uint32_t listPointer)
{
    if (!dcb) return;

    dcb->listPointer = listPointer;
}

uint32_t DCB_GetListPointer(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->listPointer;
}

// DMA helper functions

void DCB_SetDMAAddress(DCB *dcb, uint32_t address)
{
    if (!dcb) return;

    dcb->dmaAddress = address;
}

uint32_t DCB_GetDMAAddress(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->dmaAddress;
}

void DCB_SetDMABytesRead(DCB *dcb, int bytesRead)
{
    if (!dcb) return;

    dcb->dmaBytesRead = bytesRead;
}

int DCB_GetDMABytesRead(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->dmaBytesRead;
}

void DCB_SetDMABytesWritten(DCB *dcb, int bytesWritten)
{
    if (!dcb) return;

    dcb->dmaBytesWritten = bytesWritten;
}

int DCB_GetDMABytesWritten(const DCB *dcb)
{
    if (!dcb) return 0;

    return dcb->dmaBytesWritten;
}

void DCB_SetDMAReadData(DCB *dcb, int data)
{
    if (!dcb) return;

    dcb->dmaReadData = data;
}

int DCB_GetDMAReadData(const DCB *dcb)
{
    if (!dcb) return -1;

    return dcb->dmaReadData;
}

bool DCB_IsDMAReadDataValid(const DCB *dcb)
{
    if (!dcb) return false;

    return dcb->dmaReadData != -1;
}

void DCB_ClearDMAReadData(DCB *dcb)
{
    if (!dcb) return;

    dcb->dmaReadData = -1;
}

// Debug/utility functions

void DCB_Print(const DCB *dcb)
{
    if (!dcb) {
        printf("DCB: (null)\n");
        return;
    }

    printf("DCB {\n");
    printf("  BufferAddress: 0x%08X\n", dcb->bufferAddress);
    printf("  OffsetFromLP: %u\n", dcb->offsetFromLP);
    printf("  KeyValue: 0x%04X\n", dcb->keyValue);
    printf("  Key: 0x%04X\n", DCB_GetKey(dcb));
    printf("  HasRSOMFlag: %s\n", DCB_HasRSOMFlag(dcb) ? "true" : "false");
    printf("  HasREOMFlag: %s\n", DCB_HasREOMFlag(dcb) ? "true" : "false");
    printf("  DataFlowCost: 0x%04X\n", DCB_GetDataFlowCost(dcb));
    printf("  ByteCount: %u\n", dcb->byteCount);
    printf("  MostAddress: 0x%04X\n", dcb->mostAddress);
    printf("  LeastAddress: 0x%04X\n", dcb->leastAddress);
    printf("  DataMemoryAddress: 0x%08X\n", DCB_GetDataMemoryAddress(dcb));
    printf("  Displacement: %u\n", dcb->displacement);
    printf("  ListPointer: 0x%08X\n", dcb->listPointer);
    printf("  DMAAddress: 0x%08X\n", dcb->dmaAddress);
    printf("  DMABytesRead: %d\n", dcb->dmaBytesRead);
    printf("  DMABytesWritten: %d\n", dcb->dmaBytesWritten);
    printf("  DMAReadData: %d %s\n", dcb->dmaReadData,
           DCB_IsDMAReadDataValid(dcb) ? "(valid)" : "(invalid)");
    printf("}\n");
}