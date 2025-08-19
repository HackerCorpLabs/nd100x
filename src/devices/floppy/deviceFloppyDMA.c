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

//#define DEBUG_FLOPPY_DMA
//#define DEBUG_DETAIL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../devices_types.h"
#include "../devices_protos.h"
#include "deviceFloppyDMA.h"

/// <summary>
/// Floppy Disk Controller - 3112
/// 3112 is the 8 inch and 5.25 inch floppy controller + streamer controller card
///
/// https://www.ndwiki.org/wiki/3112
///
/// Alternative: https://www.ndwiki.org/wiki/3027 (only 8 inceh and no streamer)
///
/// Thumb Wheel
/// 0 = 1560
/// 1 = 1570
/// Implemented in according to the
/// * "ND-11.015.01 Floppy Disk Controller 3027".pdf
/// * "ND-11.012.01 FLOPPY DISK SYSTEM.PDF"
///
/// Source code of SINTRAN III J VSX
/// PDF - Print page 507 (actual PDF page 508)
/// </summary>
///

/*
 https://www.ndwiki.org/wiki/ND_floppy_disks

Format	Identification	Sector size			Sectors/t	Tracks/s	Sides/Density	Capacity pages
0b		IBM SYS-32-II	512 bytes/sector	8			77			SS/SD			154
17b		Non IBM			1024 bytes/sector	8			77			DS/DD			616

* The 8" Norsk Data format 0b was 512 bytes/sector, single sided/single density, 8 sectors per track, 77 tracks.
* The total capacity was 154 pages of 2048 bytes per page. Of this 148 pages could be allocated for files.
*
* The 5 1/4" Norsk Data format 17b was 1024 bytes/sector, double sided/double density, 8 sectors per track, 77 tracks per side.
* The total capacity was 616 pages. Of this 612 pages could be allocated for files.
*
* Note that "double density" is actually so-called "1.2MB high density" format in PC terminology, sometimes called "DS/HD" or "MF2-HD". The 5 1/4" media is typically labelled "DS,HD 96TPI".
*
* The floppy controller could use any of its supported formats on any media, 8" or 5 1/4", so it is also possible to have a format 17b 8" floppy disk.
* This became feasible when media with higher density became available.
*/

static void FloppyDMA_Reset(Device *self)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return;

    data->status1.raw = 0;
    data->status2.raw = 0;
    data->controlWord.raw = 0;

    data->data = 0;
    data->sector = 0;
    data->track = 0;
    data->drive = 0;

    data->command = 0;
    data->pointerHI = 0;
    data->pointerLO = 0;
    data->readFormat = 0;
    // data->diskFile = NULL;

    // data->status1.bits.errorCode = FLOPPY_ERR_OK;
    // data->status1.bits.readyForTransfer = true;
}

static uint16_t CalculateStatusRegister1(Device *self)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return 0;

    data->status1.bits.dualDensity = 1; // 1=HIGH=DUAL DENSITY CONTROLLER (tells ND that we are using DMA and not PIO controller) = NEW CONTROLLER
    data->status1.bits.inclusiveOrBits = (data->status1.bits.hardError | data->status1.bits.deletedRecord | data->status1.bits.retryOnController);
    return data->status1.raw;
}

static uint16_t FloppyDMA_Read(Device *self, uint32_t address)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return 0;

    uint32_t reg = Device_RegisterAddress(self, address);
    uint16_t value = 0;

    switch (reg)
    {
    case FLOPPY_DMA_READ_DATA:
        value = 0x1; // TODO: What is the correct value?
        break;

    case FLOPPY_DMA_READ_STATUS1:
        value = CalculateStatusRegister1(self);
        break;

    case FLOPPY_DMA_READ_STATUS2:
        value = data->status2.raw;
        break;
    }

#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA_Read: reg=%d, value=%o\n", reg, value);
#endif

    return value;
}

static void FloppyDMA_Write(Device *self, uint32_t address, uint16_t value)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return;

    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA_Write: reg=%d, value=%o\n", reg, value);
#endif

    switch (reg)
    {
    case FLOPPY_DMA_LOAD_CONTROL:
    {
        data->controlWord.raw = value;

        data->status1.bits.interruptEnabled = data->controlWord.bits.enableInterrupt;

        if (data->controlWord.bits.deviceClear)
        {
            data->drive = -1;
            data->status1.bits.readyForTransfer = true;
        }

        Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);

        if (data->controlWord.bits.activateAutoload)
        {
            ExecuteAutoload(self, data->drive);
        }
        else if (data->controlWord.bits.executeCommand)
        {
            if (data->controlWord.bits.testMode)
            {
                ExecuteTest(self, data->controlWord.bits.testData);
            }
            else
            {
                if (data->controlWord.bits.enableStreamer)
                {
                    // TODO: Implement streamer
                }
                else
                {
                    OpenFloppyFile(self, 0);
                    ExecuteFloppyGo(self);
                    CloseFloppyFile(self, 0);
                }
            }
        }
    }
    break;

    case FLOPPY_DMA_LOAD_POINTER_HI:
        data->pointerHI = value;
        break;

    case FLOPPY_DMA_LOAD_POINTER_LO:
        data->pointerLO = value;
        break;
    }
}

static uint16_t FloppyDMA_Tick(Device *self)
{
    if (!self)
        return 0;

    Device_TickIODelay(self);
    return self->interruptBits;
}

static uint16_t FloppyDMA_Ident(Device *self, uint16_t level)
{
    if (!self)
        return 0;

    if ((self->interruptBits & (1 << level)) != 0)
    {
        FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
        data->status1.bits.interruptEnabled = false;
        Device_SetInterruptStatus(self, false, level);
        return self->identCode;
    }
    return 0;
}

// Load floppy monitor (FLO-LOAD, almost like BPUN from the first sector)
static void ExecuteAutoload(Device *self, int drive)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return;

#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA: Executing Autoload\n");
#endif

    /* TODO: DMA TRANSFER PROM bootcode to ND-100 Memory */
    Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)AutoLoadEnd, 0, self->interruptLevel);
}

// Execute test mode
// Se page 16 of 3027 manual
static void ExecuteTest(Device *self, int testData)
{
#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA: Executing test %d\n", testData);
#endif
}

static void ExecuteFloppyGoCallback(Device *self)
{
#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA: Executing Floppy GO Callback!\n");
#endif

    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data) return;

    uint32_t sector = 1;
    uint32_t track = 0;
    uint32_t bytes_pr_sector = 512;
    uint32_t sectors_pr_track = 18; // Double Density has 8 sectors per track

    // Find ND100 memory address
    data->commandBlockAddress = data->pointerLO | (data->pointerHI << 16);

    // Read command block
    data->commandBlock.fields.commandWord = Device_DMARead(data->commandBlockAddress);
    data->commandBlock.fields.diskAddress = Device_DMARead(data->commandBlockAddress + 1);
    data->commandBlock.fields.memoryAddressHi = Device_DMARead(data->commandBlockAddress + 2) & 0xFF; // Bits 23-16
    data->commandBlock.fields.memoryAddressLo = Device_DMARead(data->commandBlockAddress + 3);        // Bits 15-0

    uint32_t memAddress = data->commandBlock.fields.memoryAddressLo | (data->commandBlock.fields.memoryAddressHi << 16); // Calculate memory address for read/write IO

    data->commandBlock.fields.optionsWordCountHi = Device_DMARead(data->commandBlockAddress + 4); // Bit 15 = 1 => WordCount, 0=SectorCount. Bit 7-0 WordCountHi
    bool isWC = (data->commandBlock.fields.optionsWordCountHi & (1 << 15)) != 0;

    data->commandBlock.fields.wordSectorCount = Device_DMARead(data->commandBlockAddress + 5); // Word count / Sector count

    data->commandBlock.fields.status1 = Device_DMARead(data->commandBlockAddress + 6);
    uint32_t wordCount = data->commandBlock.fields.wordSectorCount | (data->commandBlock.fields.optionsWordCountHi & 0xFF) << 16;

    data->command = (FloppyFunction)(data->commandBlock.fields.commandWord & 0b111111);
    data->drive = (data->commandBlock.fields.commandWord >> 6) & 0b11;
    uint32_t floppyFormat = (data->commandBlock.fields.commandWord >> 8) & 0b11;
    uint32_t doubleSided = (data->commandBlock.fields.commandWord >> 10) & 0b1;
    uint32_t doubleDensity = (data->commandBlock.fields.commandWord >> 11) & 0b1;
    uint32_t copyDestination = (data->commandBlock.fields.commandWord >> 14) & 0b1; // When copying from one device to another ?


    
    // Get information on file size and readonly
    if (self->blockCallbacks.diskInfoFunc)
    {
        bool isWriteProtected = false;
        size_t imageSize = 0;

        self->blockCallbacks.diskInfoFunc(self, &imageSize, &isWriteProtected,  data->drive );
        data->diskFileSize = imageSize;
        data->readOnly = isWriteProtected;        
    }

    // TODO:
    // Read out information from floppy - is it write protceted

    // bool floppyIsWriteProtected = device.read_only;
    // long floppySize = device.FileLength;

    // Calculate sector size based on format
    switch (floppyFormat)
    {
    case 0:
        bytes_pr_sector = 512;
        break;
    case 1:
        bytes_pr_sector = 256;
        break;
    case 2:
        bytes_pr_sector = 123;
        break;
    case 3:
        bytes_pr_sector = 1024;
        break;
    }

    // Reflect current sector size on the device so block callbacks know bytes per block
    self->blockSizeBytes = bytes_pr_sector;

    sector = (data->commandBlock.fields.diskAddress % (sectors_pr_track - 1)) + 1;
    track = data->commandBlock.fields.diskAddress / sectors_pr_track;

    uint32_t position = data->commandBlock.fields.diskAddress * bytes_pr_sector;
    uint32_t wordsToRead = wordCount;
    if (!isWC) wordsToRead *= (bytes_pr_sector >> 1);

    data->status1.bits.errorCode = FLOPPY_ERR_OK;
    data->status1.bits.readyForTransfer = false;
    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);

    data->status1.bits.readyForTransfer = false;
    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);


#ifdef DEBUG_FLOPPY_DMA    
    printf("Command block received from memory at 0x%08X\n", data->commandBlockAddress);
    printf("------------------------------------------------\n");
    for (int i = 0; i < 12; i++)
    {
        printf("Command block %d: 0x%4X\n", i,data->commandBlock.raw[i]);
    }
    printf("------------------------------------------------\n");

#endif

    data->commandBlock.fields.status1 = 0;
    data->commandBlock.fields.status2 = (data->drive << 8); // Selected unit is reported back in bits 8-9
    int wordsTransfered = 0;

    // Number of blocks to transfer where each block is blockSizeBytes bytes (typically 512/1024)
    uint32_t blockCounter = (wordsToRead * 2) / self->blockSizeBytes;
    uint32_t buffer_ptr = 0;

    uint8_t *buffer;
    int blocksRead = -1;



#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA_cb: Command: %d\n", data->command);
#endif
    switch (data->command)
    {
    case FLOPPY_FUNC_READ_DATA:
        // Data is read from the floppy disk to the ND-100 memory.The start address is given as the logical sector address,
        // and a choice can be made between the word count and the number of sectors to indicate the length of the transfer.
        // The parameters for the transfer are given in the command field in the ND - 100 memory.
        //  The transfer will always start at the beginning of a sector, but the number of words to be read may be preset to any number of words.

#ifdef DEBUG_DETAIL
        printf("Starting ReadData_cb on drive position %d, wordsToRead: %d\r\n", position, wordsToRead);
#endif

        buffer = (uint8_t *)malloc(blockCounter * self->blockSizeBytes);
        if (!buffer)
        {
            data->status1.bits.errorCode = FLOPPY_ERR_CRC;
            data->status1.bits.deviceActive = false;
            data->status1.bits.readyForTransfer = true;
            Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);                        
            return;
        }

        // Read all blocks from floppy  disk file into buffer
        blocksRead = self->blockCallbacks.readFunc(self, buffer, blockCounter, data->commandBlock.fields.diskAddress, data->drive);
        if ((blocksRead < 0) || (blocksRead != blockCounter))
        {
            data->status1.bits.errorCode = FLOPPY_ERR_CRC;
            data->status1.bits.deviceActive = false;
            data->status1.bits.readyForTransfer = true;
            Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);                        

            free(buffer);
            return;
        }

        while (wordsToRead > 0)
        {
            // Read word from disk
            uint32_t readData = Device_IO_BufferReadWord(self, buffer, buffer_ptr++);

            // DMA Write to RAM memory
            Device_DMAWrite(memAddress, (uint16_t)readData);

            memAddress++;
            wordsToRead--;
            wordsTransfered++;
        }
        free(buffer);

        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_DATA:
// Same procedure as READ DATA, except that transfer is now from the ND - 100 to the diskette.
#ifdef DEBUG_DETAIL
        printf("Starting WriteData_cb on drive position %d\r\n", position);
#endif

        buffer = (uint8_t *)malloc(blockCounter * self->blockSizeBytes);
        if (!buffer)
        {
            data->status1.bits.errorCode = FLOPPY_ERR_CRC;
            data->status1.bits.deviceActive = false;
            data->status1.bits.readyForTransfer = true;
            Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);                        
            
            return;
        }

        while (wordsToRead > 0)
        {
            uint16_t word = Device_DMARead(memAddress);

            // Write word to disk buffer
            if (Device_IO_BufferWriteWord(self, buffer, buffer_ptr++, (uint16_t)word) < 0)
            {
                data->status1.bits.errorCode = FLOPPY_ERR_CRC;
                data->status1.bits.deviceActive = false;
                data->status1.bits.readyForTransfer = true;
                Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);                        
    
                free(buffer);
                return;
            }

            memAddress++;
            wordsToRead--;
            wordsTransfered++;
        }

        // Write all blocks to floppy disk file from buffer
        int blocksWrite = self->blockCallbacks.writeFunc(self, buffer, blockCounter, data->commandBlock.fields.diskAddress, data->drive);
        if ((blocksWrite < 0) || (blocksWrite != blockCounter))
        {
            data->status1.bits.errorCode = FLOPPY_ERR_CRC;
            data->status1.bits.deviceActive = false;
            data->status1.bits.readyForTransfer = true;
            Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);                        

            free(buffer);
            return;
        }

        free(buffer);

        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FIND_EOF:
        printf("Starting FindEOF on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_EOF:
        printf("Starting WriteEOF on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FORMAT_FLOPPY:
        printf("Starting FormatFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_READ_FORMAT:
#ifdef DEBUG_DETAIL
        printf("Starting ReadFormat on drive position %d\r\n", position);
#endif
        //*"Read format" returns format in Status word 2
        // Format read from diskette, valid for read format command or when eror 12
        // Bit 0-1 Bytes pr sector
        //	00 = 512 bytes /sector
        //	01 = 256 Bytes /sector
        //	10 = 123 bytes /sector
        //	11 = 1024 bytes/sector

        // Calculate format based on file size
        if (data->diskFileSize == 315392)
        {
            // 8" disk
            data->commandBlock.fields.status2 |= 0; // 512 bytes/sector
        }
        else if (data->diskFileSize >= 1261568)
        {
            // 5.25" 1.2MB disk
            data->commandBlock.fields.status2 |= 0x3;      // 1024 bytes/sector
            data->commandBlock.fields.status2 |= (1 << 2); // Double sided
            data->commandBlock.fields.status2 |= (1 << 3); // Double density
        }
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_READ_DELETED:
        printf("Starting ReadDeletedRecord on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_DELETED:
        printf("Starting WriteDeletedRecord on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_COPY_FLOPPY:
        printf("Starting CopyFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FORMAT_TRACK:
        printf("Starting FormatTrack on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_CHECK_FLOPPY:
        /*
            Data is read to the controller's local memory to test for CRC-errors.
            The test halts with the first error discovered. The address of the erroneous sector is held in LAST MEMADDR in the status field.

            The failing sector is in the least significant part of LAST MEMADDR.

            Status field
            +------+-----------------+--------------------+
            | 15   | 14     -      8 | 7                0 |
            +------+-----------------+--------------------+
            | Side |   Track Number  |   Sector number    |
            +------+-----------------+--------------------+

         */
        printf("Starting CheckFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_IDENTIFY:
        printf("Starting Identify on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    default:
        printf("FloppyDMA: Unknown command: %d\n", data->command);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;
    }

    /**************************/
    /* Update command block   */
    /**************************/

    // Status 1
    data->commandBlock.fields.status1 = data->status1.raw;
    Device_DMAWrite(data->commandBlockAddress + 6, data->commandBlock.fields.status1);

    // Status 2
    Device_DMAWrite(data->commandBlockAddress + 7, data->commandBlock.fields.status2);

    // Send back last mem address
    Device_DMAWrite(data->commandBlockAddress + 8, (uint16_t)((memAddress >> 16) & 0xFF)); // HI
    Device_DMAWrite(data->commandBlockAddress + 9, (uint16_t)(memAddress & 0xFFFF));       // LO

    // Update words to read
    Device_DMAWrite(data->commandBlockAddress + 10, (uint16_t)((wordsTransfered >> 16) & 0xFF)); // HI
    Device_DMAWrite(data->commandBlockAddress + 11, (uint16_t)(wordsTransfered & 0xFFFF));       // LO

    // For now, just simulate completion
    Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
}

static void ExecuteFloppyGo(Device *self)
{

    if (!self)
        return;

    if ((self->blockCallbacks.readFunc) && (self->blockCallbacks.writeFunc))
    {
        return ExecuteFloppyGoCallback(self);
    }

#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA: Executing Floppy GO!\n");
#endif
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return;

    uint32_t sector = 1;
    uint32_t track = 0;
    uint32_t bytes_pr_sector = 512;
    uint32_t sectors_pr_track = 18; // Double Density has 8 sectors per track

    // Find ND100 memory address
    data->commandBlockAddress = data->pointerLO | (data->pointerHI << 16);

    // Read command block
    data->commandBlock.fields.commandWord = Device_DMARead(data->commandBlockAddress);
    data->commandBlock.fields.diskAddress = Device_DMARead(data->commandBlockAddress + 1);
    data->commandBlock.fields.memoryAddressHi = Device_DMARead(data->commandBlockAddress + 2) & 0xFF; // Bits 23-16
    data->commandBlock.fields.memoryAddressLo = Device_DMARead(data->commandBlockAddress + 3);        // Bits 15-0

    uint32_t memAddress = data->commandBlock.fields.memoryAddressLo | (data->commandBlock.fields.memoryAddressHi << 16); // Calculate memory address for read/write IO

    data->commandBlock.fields.optionsWordCountHi = Device_DMARead(data->commandBlockAddress + 4); // Bit 15 = 1 => WordCount, 0=SectorCount. Bit 7-0 WordCountHi
    bool isWC = (data->commandBlock.fields.optionsWordCountHi & (1 << 15)) != 0;

    data->commandBlock.fields.wordSectorCount = Device_DMARead(data->commandBlockAddress + 5); // Word count / Sector count

    data->commandBlock.fields.status1 = Device_DMARead(data->commandBlockAddress + 6);
    uint32_t wordCount = data->commandBlock.fields.wordSectorCount | (data->commandBlock.fields.optionsWordCountHi & 0xFF) << 16;

    data->command = (FloppyFunction)(data->commandBlock.fields.commandWord & 0b111111);
    data->drive = (data->commandBlock.fields.commandWord >> 6) & 0b11;
    uint32_t floppyFormat = (data->commandBlock.fields.commandWord >> 8) & 0b11;
    uint32_t doubleSided = (data->commandBlock.fields.commandWord >> 10) & 0b1;
    uint32_t doubleDensity = (data->commandBlock.fields.commandWord >> 11) & 0b1;
    uint32_t copyDestination = (data->commandBlock.fields.commandWord >> 14) & 0b1; // When copying from one device to another ?

    // TODO:
    // Read out information from floppy - is it write protceted

    // bool floppyIsWriteProtected = device.read_only;
    // long floppySize = device.FileLength;

    // Calculate sector size based on format
    switch (floppyFormat)
    {
    case 0:
        bytes_pr_sector = 512;
        break;
    case 1:
        bytes_pr_sector = 256;
        break;
    case 2:
        bytes_pr_sector = 123;
        break;
    case 3:
        bytes_pr_sector = 1024;
        break;
    }

    // Reflect current sector size on the device so block callbacks know bytes per block
    self->blockSizeBytes = bytes_pr_sector;

    sector = (data->commandBlock.fields.diskAddress % (sectors_pr_track - 1)) + 1;
    track = data->commandBlock.fields.diskAddress / sectors_pr_track;

    uint32_t position = data->commandBlock.fields.diskAddress * bytes_pr_sector;
    uint32_t wordsToRead = wordCount;
    if (!isWC)
        wordsToRead *= (bytes_pr_sector >> 1);

    data->status1.bits.errorCode = FLOPPY_ERR_OK;
    data->status1.bits.readyForTransfer = false;
    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);

    data->status1.bits.readyForTransfer = false;
    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);

#ifdef DEBUG_FLOPPY_DMA
    printf("Command block received from memory at 0x%08X\n", data->commandBlockAddress);
    printf("------------------------------------------------\n");
    for (int i = 0; i < 12; i++)
    {
        printf("Command block %d: 0x%4X\n", i, data->commandBlock.raw[i]);
    }
    printf("------------------------------------------------\n");

#endif
    if (!data->diskFile)
    {
        printf("Error: FloppyDMA: No disk file opened for floppy!\n");
    }
    int seekRes = Device_IO_Seek(self, data->diskFile, position);
    if (seekRes < 0)
    {
        printf("FloppyDMA: Seek error: %d\n", seekRes);
        // Report error
        data->status1.bits.errorCode = FLOPPY_ERR_TRACK_OUT_OF_RANGE;
        data->status1.bits.deviceActive = false;
        data->status1.bits.readyForTransfer = true;

        Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);
        return;
    }

    data->commandBlock.fields.status1 = 0;
    data->commandBlock.fields.status2 = (data->drive << 8); // Selected unit is reported back in bits 8-9
    int wordsTransfered = 0;

#ifdef DEBUG_FLOPPY_DMA
    printf("FloppyDMA: Command: %d\n", data->command);
#endif
    switch (data->command)
    {
    case FLOPPY_FUNC_READ_DATA:
        // Data is read from the floppy disk to the ND-100 memory.The start address is given as the logical sector address,
        // and a choice can be made between the word count and the number of sectors to indicate the length of the transfer.
        // The parameters for the transfer are given in the command field in the ND - 100 memory.
        //  The transfer will always start at the beginning of a sector, but the number of words to be read may be preset to any number of words.

#ifdef DEBUG_DETAIL
        printf("Starting read data on drive position %d, wordsToRead: %d\r\n", position, wordsToRead);
#endif

        while (wordsToRead > 0)
        {
            int readData = Device_IO_ReadWord(self, data->diskFile);
            if (readData == -1)
            {
                data->status1.bits.errorCode = FLOPPY_ERR_CRC;
                data->status1.bits.deviceActive = false;
                data->status1.bits.readyForTransfer = true;
                Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);
                return;
            }

            // DMA Write
            Device_DMAWrite(memAddress, (uint16_t)readData);

            memAddress++;
            wordsToRead--;
            wordsTransfered++;
        }
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_DATA:
// Same procedure as READ DATA, except that transfer is now from the ND - 100 to the diskette.
#ifdef DEBUG_DETAIL
        printf("Starting WriteData on drive position %d\r\n", position);
#endif

        while (wordsToRead > 0)
        {
            uint16_t word = Device_DMARead(memAddress);

            if (!Device_IO_WriteWord(self, data->diskFile, word))
            {
                data->status1.bits.errorCode = FLOPPY_ERR_CRC;
                data->status1.bits.deviceActive = false;
                data->status1.bits.readyForTransfer = true;
                Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);
                return;
            }

            memAddress++;
            wordsToRead--;
            wordsTransfered++;
        }
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FIND_EOF:
        printf("Starting FindEOF on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_EOF:
        printf("Starting WriteEOF on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FORMAT_FLOPPY:
        printf("Starting FormatFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_READ_FORMAT:
#ifdef DEBUG_DETAIL
        printf("Starting ReadFormat on drive position %d\r\n", position);
#endif
        //*"Read format" returns format in Status word 2
        // Format read from diskette, valid for read format command or when eror 12
        // Bit 0-1 Bytes pr sector
        //	00 = 512 bytes /sector
        //	01 = 256 Bytes /sector
        //	10 = 123 bytes /sector
        //	11 = 1024 bytes/sector

        // Calculate format based on file size
        if (data->diskFileSize == 315392)
        {
            // 8" disk
            data->commandBlock.fields.status2 |= 0; // 512 bytes/sector
        }
        else if (data->diskFileSize >= 1261568)
        {
            // 5.25" 1.2MB disk
            data->commandBlock.fields.status2 |= 0x3;      // 1024 bytes/sector
            data->commandBlock.fields.status2 |= (1 << 2); // Double sided
            data->commandBlock.fields.status2 |= (1 << 3); // Double density
        }
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_READ_DELETED:
        printf("Starting ReadDeletedRecord on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_WRITE_DELETED:
        printf("Starting WriteDeletedRecord on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_COPY_FLOPPY:
        printf("Starting CopyFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_FORMAT_TRACK:
        printf("Starting FormatTrack on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_CHECK_FLOPPY:
        /*
            Data is read to the controller's local memory to test for CRC-errors.
            The test halts with the first error discovered. The address of the erroneous sector is held in LAST MEMADDR in the status field.

            The failing sector is in the least significant part of LAST MEMADDR.

            Status field
            +------+-----------------+--------------------+
            | 15   | 14     -      8 | 7                0 |
            +------+-----------------+--------------------+
            | Side |   Track Number  |   Sector number    |
            +------+-----------------+--------------------+

         */
        printf("Starting CheckFloppy on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    case FLOPPY_FUNC_IDENTIFY:
        printf("Starting Identify on drive position %d\r\n", position);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;

    default:
        printf("FloppyDMA: Unknown command: %d\n", data->command);
        Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
        break;
    }

    /**************************/
    /* Update command block   */
    /**************************/

    // Status 1
    data->commandBlock.fields.status1 = data->status1.raw;
    Device_DMAWrite(data->commandBlockAddress + 6, data->commandBlock.fields.status1);

    // Status 2
    Device_DMAWrite(data->commandBlockAddress + 7, data->commandBlock.fields.status2);

    // Send back last mem address
    Device_DMAWrite(data->commandBlockAddress + 8, (uint16_t)((memAddress >> 16) & 0xFF)); // HI
    Device_DMAWrite(data->commandBlockAddress + 9, (uint16_t)(memAddress & 0xFFFF));       // LO

    // Update words to read
    Device_DMAWrite(data->commandBlockAddress + 10, (uint16_t)((wordsTransfered >> 16) & 0xFF)); // HI
    Device_DMAWrite(data->commandBlockAddress + 11, (uint16_t)(wordsTransfered & 0xFFFF));       // LO

    // For now, just simulate completion
    Device_QueueIODelay(self, IODELAY_FLOPPY, (IODelayedCallback)ReadEnd, data->drive, self->interruptLevel);
}

static bool ReadEnd(Device *self, int drive)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return false;

    data->status1.bits.deviceActive = false;
    data->status1.bits.readyForTransfer = true;

    data->commandBlock.fields.status1 = CalculateStatusRegister1(self);
    Device_DMAWrite(data->commandBlockAddress + 6, data->commandBlock.fields.status1);

    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);
    return false;
}

static bool AutoLoadEnd(Device *self, int drive)
{
    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return false;

    data->status1.bits.deviceActive = false;
    data->status1.bits.readyForTransfer = true;
    Device_SetInterruptStatus(self, data->status1.bits.interruptEnabled && data->status1.bits.readyForTransfer, self->interruptLevel);
    return false;
}

static bool OpenFloppyFile(Device *self, int drive)
{

    // If we have hooked up callbacks, we don't need to open a file
    if ((self->blockCallbacks.readFunc) && (self->blockCallbacks.writeFunc)) return true;

    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return false;

    if (data->diskFile)
    {
        fclose(data->diskFile);
        data->diskFile = NULL;
    }

    // Open floppy file
    if ((data->diskFile = fopen(data->floppyName, "rb+")) == NULL) // open for read/write in binary mode
    {
        printf("Unable to open file %s\n", data->floppyName);
        return false;
    }
    else
    {
        // Get file size
        fseek(data->diskFile, 0, SEEK_END);
        data->diskFileSize = ftell(data->diskFile);
        fseek(data->diskFile, 0, SEEK_SET); // Reset file position back to start
    }
    return true;
}

static bool CloseFloppyFile(Device *self, int drive)
{

    // If we have hooked up callbacks, we don't need to close a file
    if ((self->blockCallbacks.readFunc) && (self->blockCallbacks.writeFunc)) return true;

    FloppyDMAData *data = (FloppyDMAData *)self->deviceData;
    if (!data)
        return false;

    if (data->diskFile)
    {
        fclose(data->diskFile);
        data->diskFile = NULL;
    }
    return true;
}

Device *CreateFloppyDMADevice(uint8_t thumbwheel)
{
    Device *dev = (Device *)malloc(sizeof(Device));
    if (!dev)
        return NULL;

    FloppyDMAData *data = (FloppyDMAData *)malloc(sizeof(FloppyDMAData));
    if (!data)
    {
        free(dev);
        return NULL;
    }

    memset(dev, 0, sizeof(Device));
    memset(data, 0, sizeof(FloppyDMAData));

    // Initialize device base structure
    Device_Init(dev, thumbwheel, DEVICE_CLASS_BLOCK, 1024);

    // Initialize device state
    FloppyDMA_Reset(dev);

    // Set up device address and interrupt settings based on thumbwheel
    switch (thumbwheel)
    {
    case 0:
        strcpy(dev->memoryName, "Floppy DMA 0");
        dev->identCode = 021; // octal 021
        dev->startAddress = 01560;
        dev->endAddress = 01567;
        break;
    case 1:
        strcpy(dev->memoryName, "Floppy DMA 1");
        dev->identCode = 022; // Octal 022
        dev->startAddress = 01570;
        dev->endAddress = 01577;
        break;
    default:
        printf("Floppy DMA: Unknown thumbwheel value: %d\n", thumbwheel);
        free(data);
        free(dev);
        return NULL;
    }

    dev->interruptLevel = 11; // floppy

    data->floppyName = "FLOPPY.IMG"; // TODO: Make this configurable
    data->diskFileSize = 0;

    // Set up function pointers
    dev->Read = FloppyDMA_Read;
    dev->Write = FloppyDMA_Write;
    dev->Tick = FloppyDMA_Tick;
    dev->Reset = FloppyDMA_Reset;
    dev->Ident = FloppyDMA_Ident;
    dev->deviceData = data;

    printf("Floppy DMA [%s] Device object created. Address[%o-%o] Ident code: [%o] Level: [%d]\n", dev->memoryName, dev->startAddress, dev->endAddress, dev->identCode, dev->interruptLevel);

    return dev;
}