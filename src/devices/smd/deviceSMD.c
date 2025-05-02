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
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include "../devices_types.h"
#include "../devices_protos.h"

#include "deviceSMD.h"

// #define DEBUG_DETAIL
//#define DEBUG_HIGH_LEVEL

static void SMD_Reset(Device *self)
{
    SMDData *data = (SMDData *)self->deviceData;
    if (!data)
        return;

    data->statusRegister.raw = 0;
    data->controlRegister.raw = 0;
    data->errorRegister.raw = 0;
    data->driveAddress.raw = 0;

    data->bufferPointer = 0;
    data->sector = 1;
    data->track = 0;
    data->selectedDrive = -1;
    data->sectorAutoIncrement = false;
}

static uint16_t SMD_Read(Device *self, uint32_t address)
{

    SMDData *data = (SMDData *)self->deviceData;
    if (!data || !data->regs.selectedDisk)
        return 0;
    uint32_t reg = Device_RegisterAddress(self, address);
    uint16_t value = 0;

#ifdef DEBUG_DETAIL
    printf("SMD::SMD_Read called [%o] reg = %o\n", address, reg);
#endif

    switch (reg)
    {
    case SMD_READ_MEMORY_ADDRESS:
        if (data->controlRegister.bits.registerMultiplexBit)
        {
            // Read Word Counter
            //
            // The Word counter register is read the same way as the memory address register.
            // After a transfer, the upper/lower memory address(or word count) control bit(flip—flop) is reset.
            // A Read Status instruction(DEV.NO. + 4) or a Device Clear will also reset this bit

            if ((!data->regs.wcrFlipFlop) || (!data->regs.hasFlipFlops))
            {
                data->regs.wcrFlipFlop = true;
                value = data->regs.wordCounter;
            }
            else
            {
                data->regs.wcrFlipFlop = false;
                value = data->regs.wordCounterHI;
            }
        }
        else
        {
            // Read Core Address
            //
            // The Memory Address regster is read by two successive IOX instructions.
            // The first one gets the lower 16 bits(Address bits 0—15 into the A - reg. 0—15), and the second one gets the upper bits
            // (Address bits 16 - 23 into A—reg. 0 - 7). When reading the most significant bits, the upper byte of the A—reg. is undefined and has tobe masked.

            if ((!data->regs.marFlipFlop) || (!data->regs.hasFlipFlops))
            {
                data->regs.marFlipFlop = true;
                value = data->regs.coreAddress;
            }
            else
            {
                data->regs.marFlipFlop = false;
                value = data->regs.coreAddressHiBits;
            }
        }
        break;

    case SMD_READ_SEEK_CONDITION:
        if (data->controlRegister.bits.registerMultiplexBit)
        {
            value = data->regs.eccCount;
        }
        else
        {
            /*
            READ SEEK CONDITION

            Bits 0 - 7: Seek Complete
                Seek complete status for units 0-7. True if the unit has moved the heads to the correct cylinder or a seek error has occured and
                the heads are under the sector number prior to the one specified by the block address loaded before the initiate seek commands for that
                unit has first been issued.

                Thus, after an initiate seek command is given, the Seek Comptete bit for that unit will appear once per revolution after the unit is positioned on the
                correct cylinder, or a seek error has occurred. The condition will last until a transfer command is given.

            Bits 8 - 10: Unit Selected
                The unit number is loaded by the last control word.

            Bit 11: Seek Error
                Seek error for the selected unit.This signal indicates that the unit was unable to complete a move within 600 ms,
                or that the heads have moved to a position outside the recording field, or that an address greater than the maximum number of tracks has been selected.

                This signal will only be cleared by performing a Return to Zero command on the unit,

            Bit 12 Not defined.
                In the docs for the 15 MHZ  Controller, it says "Bit 12 = Always 1" (maybe this is to distingiush 10Mhz vs 15 Mhz drives?)

            Bit 13: ECC Correctable
                After the hardware ECC operation M8 has been performed after a data error, this bit signals that the error is correctable and that the ECC Count and
                ECC Pattern Registers contain valid information for correction of the data.The bit is reset by Reset ECC
                (ECC Control register bit 0) or Device Clear.

            Bit 14: ECC Parity Error(STS bit no. 7)
                This bit signals that a hardwere faut condition exists in the ECC polynomials.
                This condition will also set bit 7 of the status word register and hence tigger an error interrupt if this is enabled.
                The error is reset by the Reset ECC signal (ECC Controt register bit 0) or by Device Clear Signal (CWR bit 4).
                The error is forced set when ECC Control Register bit 1 is active (Force Parity Error).

            Bit 15: Address Field
                This bit indicates that the last field read from the disk was the address field within a sector (used for ECC processing after a data check only).
*/

            if ((data->controllerType == CONTR_SMD_15MHZ) || (data->controllerType == CONTR_SMD_10MHZ))
            {
                // Bit 12 = Always 1 for 15Mhz SMD. This bit was always 0 on the NORD—10 controller.
                // If this is 1 then SINTRAN M will not r/w/boot from DISC-75-1 ??
                data->seekCondition.bits.isSMD15Mhz = 1;
            }
            else
            {
                data->seekCondition.bits.isSMD15Mhz = 0;
            }
            data->seekCondition.bits.unitSelected = data->regs.selectedUnit;
            value = data->seekCondition.raw;

            /*
            value |= (data->regs.seekCompleteBits & 0xF); // Bits 0-3

            value |= (data->regs.selectedUnit & 0x03) << 8; // Bits 8-10
            if (data->regs.seekError)
                value |= (1 << 11); // Bit 11

            */
            if ((data->controllerType == CONTR_SMD_15MHZ) || (data->controllerType == CONTR_SMD_10MHZ))
            {
                value |= (1 << 12); // 12 = Always 1 for 15Mhz SMD. This bit was always 0 on the NORD—10 controller.
                                    // If this is 1 then SINTRAN M will not r/w/boot from DISC-75-1
            }

            // 13: ECC Correctable
            // 14: ECC parity Field
            // 15: Address Field
            return value;
        }
        break;

    case SMD_READ_STATUS_REGISTER:
        if (data->controlRegister.bits.registerMultiplexBit)
        {
            data->regs.eccPatternRegister = 0;
            // Read ECC Pattern Register

            /*
            +----+----+----+----+----+--------------+
            | 15 | 14 | 13 | 12 | 11 | 1O    -    O |
            +----+----+----+----+----+--------------+
            | 1  | 0  |  1 |  1 | 1  | Error pattern|
            +----+----+----+----+----+--------------+
            Bits
                            0 - 10 Error pattern.
                            11—13 Always 1.
                            14 Always 0.To distinguish from the old HD—100 SMD controller.
                            15 Always 1.Read~back of Control Word bit 15.
            */

            // Bits 0-10
            // eccPatternRegister |= eccErrorPattern & 0x3FF; NOT USED.. yet

            // Bits 11-13, Always one
            data->regs.eccPatternRegister |= (0b111 << 11);

            // Bits 14, Always 0

            // Bits 14 - Always 0 in the new - To distinguish from the old HD-100 SMD controller
            if ((data->controllerType == CONTR_BIG_DISC) || (data->controllerType == CONTR_ECC_DISC))
                data->regs.eccPatternRegister |= (1 << 14);

            // Bits 15, Always 1
            data->regs.eccPatternRegister |= (1 << 15);

            value = data->regs.eccPatternRegister;

            // printf("SMD::SMD_ReadStatusRegister ECC alled [%o] = %o\n", address, value);
        }
        else
        {
            // hardwareError =  inclusive or of errror conditions (bits 5,6,7,8 and 13)
            data->statusRegister.bits.hardwareError =
                data->statusRegister.bits.illegalLoad |
                data->statusRegister.bits.timeOut |
                data->statusRegister.bits.comparerError |
                data->statusRegister.bits.addressMismatch |
                data->seekCondition.bits.seekError;

            if (data->regs.selectedDisk)
            {
                data->statusRegister.bits.onCylinder = data->regs.selectedDisk->onCylinder;
                data->statusRegister.bits.diskUnitNotReady = data->regs.selectedDisk->diskUnitNotReady;
            }
            else
            {
                data->statusRegister.bits.onCylinder = 0;
                data->statusRegister.bits.diskUnitNotReady = 1; // Bit 13 = Always 1 if no disk is selected (disk unit not ready)
            }

            value = data->statusRegister.raw;

            // printf("SMD::SMD_ReadStatusRegister called [%o] = %o\n", address, value);

            ClearFlipFlops(&data->regs);
        }
        break;

    case SMD_READ_BLOCK_ADDRESS:
        if (data->controlRegister.bits.registerMultiplexBit)
        {
            value = data->regs.blockAddressII;
        }
        else
        {
            value = data->regs.blockAddressI;
        }
        break;
    }

#ifdef DEBUG_DETAIL
    printf("SMD::SMD_Read reg = %o returned %o\n", reg, value);
#endif

    return value;
}

static void SMD_Write(Device *self, uint32_t address, uint16_t value)
{
    uint32_t reg = Device_RegisterAddress(self, address);

#ifdef DEBUG_DETAIL
    printf("SMD::SMD_Write called [%o] reg = %o\n", reg, value);
#endif

    SMDData *data = (SMDData *)self->deviceData;
    // if (!data || !data->regs.selectedDisk) return;

    switch (reg)
    {
    case SMD_LOAD_MEMORY_ADDRESS:
        if (data->controlRegister.bits.registerMultiplexBit)
        {
            // Count Memory Address & Word Count: This instruction is implemented for maintenance purposes only.

            // By first loading the control word with 102010, a special test mode, each of these instructions will
            // * increment the memory address by one
            // * decrement the word count by one. (Refer to section 3.1, the DMA transfer.)

            if (data->controlRegister.bits.testMode && data->controlRegister.bits.marginalRecoveryCycle)
            {
                data->regs.coreAddress++;
                data->regs.wordCounter--;
            }
        }
        else
        {
            // Load Memory Address
            if (data->controlRegister.bits.active)
            {
                HandleError(self, DISK_ERR_ILLEGAL_WHILE_ACTIVE); // ILLEGAL_WHILE_DRIVE_IS_ACTIVE
                return;
            }

            // The Load Memory Address Register is loaded by two successive instructions. The first loads the 8 upper bits(A-reg. 0 - 7 into Address bits 16 - 23),
            // and the second one loads the lower 16 bits(A—reg. 0 - 15 into Address bits 0 - 15).
            // After a transfer, the upper/ lower memory address control bit(flip—flop) is reset.A Read Status instruction(DEV.NO. +4) or a Device Clear will also reset this bit.

            if ((data->regs.mawFlipFlop) || (!data->regs.hasFlipFlops))
            {
                data->regs.coreAddress = value;
                data->regs.mawFlipFlop = false;
            }
            else
            {
                // The first loads the upper 8 bits
                // regs.eccControlHI = (ushort)(value & 0xFF);

                data->regs.coreAddressHiBits = value & 0xFF;
                data->regs.mawFlipFlop = true;
            }
        }
        break;

    case SMD_LOAD_BLOCK_ADDRESS:
        if (data->controlRegister.bits.active)
        {
            HandleError(self, DISK_ERR_ILLEGAL_WHILE_ACTIVE);
            return;
        }

        if (data->controlRegister.bits.registerMultiplexBit)
        {
            data->regs.blockAddressII = value;
        }
        else
        {
            data->regs.blockAddressI = value;
        }
        break;

    case SMD_LOAD_CONTROL_WORD:
        if (data->statusRegister.bits.active)
            return; // Error ?

        /*
            Bit:
                    0		Enable interrupt on device not active
                    1		Enable interrupt on errors
                    2		Active
                                When control word bit 2 is activated, the content of the block address register II (cylinder number) is transfered to the servo system in the selected unit.
                                Logic in the unit will calculate the difference between the current cylinder and the new one. The difference and direction will command the servo to seek the new cylinder.
                    3		Test mode
                    4		Device clear (clear the active flip-flop) and controller error bas,
                    5		Address bit 16 - Extension of core address register
                    6		Address bit 17 - Extension of core address register

                    7-9		Unit select (maximum 4 units)
                    10		Marginal recovery cycle
                    11-14	Device operation code
                    15		Register multiplex bit
        */

#ifdef DEBUG_DETAIL
        printf("SMD::SMD_LoadControlWord called [%o] = %o\n", address, value);
#endif

        data->controlRegister.raw = value;

        data->statusRegister.bits.active = data->controlRegister.bits.active;
        data->statusRegister.bits.registerMultiplexBit = data->controlRegister.bits.registerMultiplexBit;
        data->statusRegister.bits.readyForTransfer = true;

        data->statusRegister.bits.interruptEnabled = data->controlRegister.bits.enableInterruptNotActive;
        data->statusRegister.bits.errorInterruptEnabled = data->controlRegister.bits.enableInterruptOnErrors;
        // Clear interrupt if not enabled
        if (!data->statusRegister.bits.interruptEnabled)
        {
            Device_SetInterruptStatus(self, false, self->interruptLevel);
        }

        // Old device didn't load the HI bits through writing to the address-register twice, but had a few bits in the ControlWord
        // if (deviceType == DeviceType.ECC_DISC_CONTR)
        if (!data->regs.hasFlipFlops)
        {
            data->regs.coreAddressHiBits = ((uint16_t)((value >> 5) & 0b11));
        }

        // data->regs.CWRBit = data->controlRegister.bits.registerMultiplexBit;

        SetSelectedUnit(&data->regs, data->controlRegister.bits.unitSelect);

        if (data->controlRegister.bits.deviceClear)
        {
            // Device Clear
            if (data->regs.selectedDisk)
            {
                data->regs.selectedDisk->diskUnitNotReady = 0;
            }

            data->seekCondition.bits.seekComplete |= (1 << data->regs.selectedUnit);

            data->statusRegister.bits.active = 0;
            data->regs.coreAddress = 0;
            data->regs.coreAddressHiBits = 0;
            data->regs.blockAddressI = 0;
            data->regs.blockAddressII = 0;
            data->regs.wordCounter = 0;
            data->regs.wordCounterHI = 0;

            data->statusRegister.bits.readyForTransfer = false;

            ClearFlipFlops(&data->regs);
            ClearErrors(self);
            // printf("SMD::SMD_LoadControlWord ClearFlipFlops & ClearErrors\n");
        }

        if (data->regs.selectedDisk)
        {
            data->regs.selectedDisk->onCylinder = 1;
        }

        if (data->statusRegister.bits.active)
        {
            if (!data->regs.selectedDisk)
            {
                data->statusRegister.bits.diskUnitNotReady = 1;
                HandleError(self, DISK_ERR_DRIVE_NOT_SELECTED);
                return;
            }
            data->regs.selectedDisk->onCylinder = 1;
            data->regs.selectedDisk->diskUnitNotReady = 0;
            ExecuteGO(self);
            // printf("SMD::ExecuteGo returned\n");
        }
        else
        {
            if (data->controlRegister.bits.testMode)
            {
                Device_SetInterruptStatus(self, data->statusRegister.bits.interruptEnabled, self->interruptLevel);
            }
            else
            {
                Device_SetInterruptStatus(self, data->statusRegister.bits.interruptEnabled & data->statusRegister.bits.readyForTransfer, self->interruptLevel);
            }
        }
        break;

    case SMD_LOAD_WORD_COUNTER:

        // Load ECC Control
        /*
        ECC CONTROL

        Bit 0: Reset ECC
                        This bit wil cause the ECC polynomisis to reset to the zero initial state. Ths function is only used when a dats error has occurred,
                        otherwise the polynomials automatically go to the zero state upon completion of a Read or Write. Device Cleer function will also reset ECC.

        Bit 1: TST — Force Parity Error
                        Used for maintenance purposes only, This bit will force ECC parity error to be set.

        Bit 2: Long
                        Used for maintenance purposes only. When 8 sector is read or writwen, the date field of the sector is extended by 64 bits (
                        the length of the ECC appendage plus "end of record" byte). The date and the extra bits sre read into of written from the memory of the CPU.
                        This function i¢ used to diagnose the operation of the ECC circuits and can be used with the following Device Operations: MO, M1, M2 and M3.
                        Thas bit is "echoed" in ECR bit 14.

        // NEW BITS FOR 15MHZ SMD DISK CONTROLLERS

        Bit 3: Format A
        Bit 4: Format B
        Bit 5: Format C
        Bit 6: Format D

        */

        if (data->controlRegister.bits.registerMultiplexBit)
        {
            // Load ECC Control
            if ((data->regs.wcEccwFlipFlop) || (!data->regs.hasFlipFlops))
            {
                data->regs.eccControl = value;

                // Bit 0 - Reset ECC
                if (data->regs.eccControl & 1)
                    data->regs.eccCount = 0;

                // Bit 1 - Force Parity Error
                if (data->regs.eccControl & (1 << 1))
                {
                    // Used for maintenance purposes only. (FILE-SYS-INV uses it!!)
                    // This bit will force ECC parity error to be set.

                    // TODO: WHat does this mean in practice - what now ?
                    data->statusRegister.bits.hardwareError2 = 1; // Set Bit 7 for the Status Register => Disk fault, missing read clocks, missing servoclocks, ECC parity error.
                }

                // Bit 2 - Long
                if ((data->regs.eccControl & 1 << 2) != 0)
                {
                    // Used for maintenance purposes only.

                    // When a sector is read or written, the data field of the sector is extended by 64 bits (The length of the ECC pattern plus "End of Record" byte).
                    // The data and the extra bits are read into or written from the memory of the CPU. This function is used to diagnose the
                    // operation of the ECC circuits, and can be used with the following Device operations: M0, M1, M2, M3. This bit is "echoed" in ECR bit 14.

                    // TODO: WHat does this mean
                }

                // Bit 3-5 - Format A-D. Used when formatting the drive.

                //
                data->regs.wcEccwFlipFlop = false;
            }
            else
            {
                // The first loads the upper 8 bits
                data->regs.eccControlHI = value & 0xFF;
                data->regs.wcEccwFlipFlop = true;
            }
        }
        else
        {
            // Load Word Counter
            // Load Word Counter; The Word Count register is increased from 16 to 24 bits, and is loaded by two successive instructions.
            // The first loads the 8 upper bits(A—reg. 0 -? into Word Count bits 16—32), and the second one loads the lower 16 bits(A - reg. 0—15 into Word Count bits 0—15).

            // After a transfer, the upper/lower Word Count control bit (flip—flop) is reset.A Read Status instruction(DEV.NO. +4) or a Device Clear will also reset this bit.
            // The controller is able to transfer a whole cylinder, or up to 16M words(24 bits), with a hardware increment of the head and sector addresses.

            // For the 75 Mb disk, the maximum word count is 132000(45k); starting with the head and cylinder address equal to 0.
            // The Word Count is set to an integer multiple of the number of words in a sector when device operation is M0—M3.

            // printf("SMD::SMD_LoadWordCounter called [%o] = %o\n", address, value);

            if ((data->regs.wcwFlipFlop) || (!data->regs.hasFlipFlops))
            {
                data->regs.wordCounter = value;
                data->regs.wcwFlipFlop = false;
            }
            else
            {
                data->regs.wordCounterHI = value & 0xFF;
                data->regs.wcwFlipFlop = true;
            }
        }
        break;
    }
}

static uint16_t SMD_Tick(Device *self)
{
    if (!self)
        return 0;

    Device_TickIODelay(self);

#if _wft_ // TODO: Remove this ??
    SMDData *data = (SMDData *)self->deviceData;
    if (data && data->regs.selectedDisk)
    {
        // Handle ongoing operations
        if (data->statusRegister.bits.active)
        {
            // Simulate operation completion
            data->statusRegister.bits.active = false;
            data->statusRegister.bits.readyForTransfer = true;
            data->statusRegister.bits.seekCompleteBits |= (1 << data->regs.selectedUnit);

            ClearFlipFlops(&data->regs);
            data->statusRegister.raw |= (1 << 3); // Ready for transfer
            data->statusRegister.raw |= (1 << 6); // Read/Write complete
            data->statusRegister.raw |= (1 << 7); // Seek complete
        }
    }
#endif
    return self->interruptBits;
}

static uint16_t SMD_Ident(Device *self, uint16_t level)
{
    if (!self)
        return 0;

#ifdef DEBUG_DETAIL
    printf("SMD::IDENT called with level %d\n", level);
#endif

    if ((self->interruptBits & (1 << level)) != 0)
    {
        SMDData *data = (SMDData *)self->deviceData;
        data->statusRegister.bits.interruptEnabled = 0;
        Device_SetInterruptStatus(self, false, level);
        return self->identCode;
    }
    return 0;
}

static int SMD_Boot(Device *self, uint16_t device_id)
{
    SMDData *data = (SMDData *)self->deviceData;
    ControllerRegs *regs = &data->regs;

    regs->selectedUnit = 0;
    regs->selectedDisk = &regs->disks[regs->selectedUnit];

    // Open the disk file if neccesary
    if (!data->regs.selectedDisk->file)
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Failed to open file %s/%s: %s\n", cwd, data->regs.selectedDisk->diskFileName, strerror(errno));
        } else {
            printf("Failed to open file %s: %s (getcwd failed: %s)\n", data->regs.selectedDisk->diskFileName, strerror(errno), strerror(errno));
        }
        HandleError(self, DISK_ERR_READ_ERROR); // READ_ERROR
        return -1;
    }

    // Seek to the beginning of the disk
    int seekRes = Device_IO_Seek(self, data->regs.selectedDisk->file, 0);
    if (seekRes < 0)
    {
        printf("Failed to seek to the beginning of the disk\n");
        HandleError(self, DISK_ERR_SEEK_ERROR); // SEEK_ERROR
        return -1;
    }
    int wordCounter = 2048; // Load 2 KW of data from the disk (4096 bytes) to memory starting at address 0

    for (int i = 0; i < wordCounter; i++)
    {
        // Read word from disk
        uint32_t readData;
        readData = Device_IO_ReadWord(self, data->regs.selectedDisk->file);
        if (readData < 0)
        {
            HandleError(self, DISK_ERR_READ_ERROR); // READ_ERROR
            return -1;
        }

        // Write to memory (DMA)
        Device_DMAWrite(i, (uint16_t)readData);
    }

    // Return boot addrees. For BPUN this might be different!
    return 0;
}

static void ExecuteGO(Device *self)
{

#ifdef DEBUG_DETAIL
    printf("SMD::ExecuteGO called\n");
#endif

    if (!self)
        return;

    SMDData *data = (SMDData *)self->deviceData;
    if (!data->regs.selectedDisk)
        return;
    ControllerRegs *regs = &data->regs;

    // Read out information from floppy - is it write protceted ?
    // TODO: ?? regs.selectedDisk.diskIsWriteProtected = device.read_only;

    // Extract CHS values from block addresses
    int sector = data->regs.blockAddressI & 0xFF;
    int head = (data->regs.blockAddressI >> 8) & 0xFF;
    int cylinder = data->regs.blockAddressII;

    // Convert CHS to LBA
    long lba = ConvertCHStoLBA(&data->regs, cylinder, head, sector);
    long position = lba * data->regs.selectedDisk->bytesPrSector;

    // Clear seek complete bit for this drive
    data->seekCondition.bits.seekComplete &= ~(1 << data->regs.selectedUnit);

    // Check for address mismatch
    long maxPosition = ConvertCHStoLBA(&data->regs,
                                       data->regs.selectedDisk->maxCylinders,
                                       data->regs.selectedDisk->headsPrCylinder,
                                       data->regs.selectedDisk->sectorsPrTrack) *
                       data->regs.selectedDisk->bytesPrSector;

    if ((position > maxPosition ||
         head >= data->regs.selectedDisk->maxCylinders ||
         sector >= data->regs.selectedDisk->sectorsPrTrack) &&
        !data->controlRegister.bits.testMode)
    {
        HandleError(self, DISK_ERR_ADDRESS_MISMATCH); // ADDRESS_MISMATCH
        return;
    }

    // Check if disk is write protected for write operations
    if (data->regs.selectedDisk->diskIsWriteProtected &&
        (data->controlRegister.bits.deviceOperation == DEVICE_OP_WRITE_TRANSFER ||
         data->controlRegister.bits.deviceOperation == DEVICE_OP_WRITE_FORMAT))
    {
        data->regs.selectedDisk->diskUnitNotReady = true;
        HandleError(self, DISK_ERR_WRITE_PROTECT_ERROR); // WRITE_PROTECT_ERROR
        return;
    }

    if (!data->regs.selectedDisk->file)
    {
        data->regs.selectedDisk->file = fopen(data->regs.selectedDisk->diskFileName, "rb+");
        if (!data->regs.selectedDisk->file)
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Failed to open file %s/%s: %s\n", cwd, data->regs.selectedDisk->diskFileName, strerror(errno));
            } else {
                printf("Failed to open file %s: %s (getcwd failed: %s)\n", data->regs.selectedDisk->diskFileName, strerror(errno), strerror(errno));
            }
            HandleError(self, DISK_ERR_READ_ERROR); // READ_ERROR
            return;
        }
    }

#ifdef DEBUG_DETAIL
    printf("SMD::ExecuteGO Seek to %ld\n", position);
#endif


    int seekRes = Device_IO_Seek(self, data->regs.selectedDisk->file, position);
    if (seekRes < 0)
    {

#ifdef DEBUG_DETAIL
        printf("SMD::ExecuteGO Seek failed\n");
#endif
        HandleError(self, DISK_ERR_SEEK_ERROR); // SEEK_ERROR
        return;
    }

    uint32_t wordCounter = (uint32_t)(data->regs.wordCounterHI << 16 | data->regs.wordCounter);
    uint32_t coreAddress = (uint32_t)(data->regs.coreAddressHiBits << 16 | data->regs.coreAddress);

    if ((position == 387072) && (wordCounter == 9216))
    {
        printf("SMD::ExecuteGO WC[%d] Core Address [%d]\n", wordCounter, coreAddress);
    }

    // Handle different device operations
    switch (data->controlRegister.bits.deviceOperation)
    {
    case DEVICE_OP_READ_TRANSFER:

#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_READ_TRANSFER WC[%d] Core Address [%d]\n", wordCounter, coreAddress);
#endif
        while (wordCounter > 0)
        {
            // Read word from disk
            uint32_t readData;
            readData = Device_IO_ReadWord(self, data->regs.selectedDisk->file);
            if (readData < 0)
            {
                HandleError(self, DISK_ERR_READ_ERROR); // READ_ERROR
                return;
            }

            // Write to memory (DMA)
            Device_DMAWrite(coreAddress, (uint16_t)readData);

            coreAddress = IncrementCoreAddress(regs);
            wordCounter = DecrementWordCounter(regs);
        }

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_WRITE_TRANSFER:

#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_WRITE_TRANSFER WC[%d] Core Address [%d]\n", wordCounter, coreAddress);
#endif
        while (wordCounter > 0)
        {
            // Read from memory (DMA)
            uint32_t writeData;
            writeData = Device_DMARead(coreAddress);

            if (writeData < 0)
            {
                HandleError(self, DISK_ERR_READ_ERROR); // DMA READ ERROR??
                return;
            }
            // Write word to disk
            if (Device_IO_WriteWord(self, data->regs.selectedDisk->file, (uint16_t)writeData) < 0)
            {
                HandleError(self, DISK_ERR_READ_ERROR); // WRITE_ERROR
                return;
            }

            coreAddress = IncrementCoreAddress(regs);
            wordCounter = DecrementWordCounter(regs);
        }
        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_READ_PARITY:

#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_READ_PARITY WC[%d] Core Address [%d]\n", wordCounter, coreAddress);
#endif
        // Read and check parity without transferring data
        while (wordCounter > 0)
        {
            // Read word from disk
            uint32_t readData;
            readData = Device_IO_ReadWord(self, data->regs.selectedDisk->file);
            if (readData < 0)
            {
                HandleError(self, DISK_ERR_READ_ERROR); // READ_ERROR
                return;
            }

            coreAddress = IncrementCoreAddress(regs);
            wordCounter = DecrementWordCounter(regs);
        }

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_COMPARE_TRANSFER:

#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_COMPARE_TRANSFER WC[%d] Core Address [%d]\n", wordCounter, coreAddress);
#endif

        while (wordCounter > 0)
        {
            // Read from disk
            uint16_t diskData;
            
            diskData = Device_IO_ReadWord(self, data->regs.selectedDisk->file);

            // Read from memory (DMA)
            int32_t memData;
            memData = Device_DMARead(coreAddress);

            // Compare data
            if (diskData != memData)
            {
                HandleError(self, DISK_ERR_COMPARER_ERROR); // COMPARER_ERROR
                return;
            }

            coreAddress = IncrementCoreAddress(regs);
            wordCounter = DecrementWordCounter(regs);
        }

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_INITIATE_SEEK:
#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_INITIATE_SEEK: %d\n", position);
#endif
        // Seek operation initiated
        data->seekCondition.bits.seekError = 0;

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_WRITE_FORMAT:
        // Format operation
        // TODO: Implement disk formatting

#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_WRITE_FORMAT: NOT IMPLEMENTED\n");
#endif
        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_SEEK_COMPLETE:
#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_SEEK_COMPLETE\n");
#endif
        regs->selectedDisk->onCylinder = true;
        data->seekCondition.bits.seekError = 0;
        data->seekCondition.bits.seekComplete = 1 << regs->selectedUnit;

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_RETURN_TO_ZERO:
#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_RETURN_TO_ZERO\n");
#endif
        data->seekCondition.bits.seekError = 0;
        regs->selectedDisk->onCylinder = 1;
        data->seekCondition.bits.seekComplete = 1 << regs->selectedUnit;

        Device_QueueIODelay(self, IODELAY_HDD_SMD, (IODelayedCallback)SMDReadEnd, data->regs.selectedDisk->unit, self->interruptLevel);
        break;

    case DEVICE_OP_RUN_ECC:
#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_RUN_ECC: NOT IMPLEMENTED\n");
#endif
        // Run ECC operation
        // TODO: Implement ECC operation
        break;

    case DEVICE_OP_SELECT_RELEASE:
#ifdef DEBUG_HIGH_LEVEL
        printf("SMD::DEVICE_OP_SELECT_RELEASE\n");
#endif
        // Release disk selection
        regs->selectedDisk = NULL;
        break;
    }
}

static bool SMDReadEnd(Device *self, int drive)
{
    if (!self)
        return false;
    SMDData *data = (SMDData *)self->deviceData;
    if (!data)
        return false;

#ifdef DEBUG_DETAIL
    printf("SMD::SMDReadEnd drive %d\n", drive);
#endif

    data->statusRegister.bits.active = 0;
    data->statusRegister.bits.readyForTransfer = 1;

    ClearFlipFlops(&data->regs);

    data->seekCondition.bits.seekComplete = 1 << drive;

    if (data->statusRegister.bits.interruptEnabled)
        return true; // returning true triggers GenerateInterrupt()
    return false;
}

static void ClearFlipFlops(ControllerRegs *regs)
{
    regs->wcwFlipFlop = false;
    regs->wcEccwFlipFlop = false;
    regs->wcrFlipFlop = false;
    regs->mawFlipFlop = false;
    regs->marFlipFlop = false;
}

static long ConvertCHStoLBA(ControllerRegs *regs, int cylinder, int head, int sector)
{
    if (!regs || !regs->selectedDisk)
        return -1;

    // LBA = (C × HPC + H) × SPT + (S − 1)
    if ((cylinder == 0) && (head == 0) && (sector == 0))
        return 0; // invalid, but used by SeekToZero

    return (cylinder * regs->selectedDisk->headsPrCylinder + head) * regs->selectedDisk->sectorsPrTrack + (sector); // was (sector-1), but for this BigDisk driver sector 0 is the start sector (not 1)
}

static void ClearErrors(Device *self)
{
    if (!self)
        return;
    SMDData *data = (SMDData *)self->deviceData;

    data->statusRegister.bits.hardwareError = 0;
    data->statusRegister.bits.hardwareError2 = 0;
    data->statusRegister.bits.illegalLoad = 0;
    data->statusRegister.bits.timeOut = 0;
    data->statusRegister.bits.comparerError = 0;
    data->statusRegister.bits.addressMismatch = 0;
    data->seekCondition.bits.seekError = 0;
}
static void SetSelectedUnit(ControllerRegs *regs, uint8_t unit)
{
    if (!regs)
        return;
    regs->selectedUnit = unit & 0x03; // Only allow units 0-3
    regs->selectedDisk = &regs->disks[regs->selectedUnit];
}

static void HandleError(Device *self, DiskError error)
{
    if (!self)
        return;
    SMDData *data = (SMDData *)self->deviceData;

#ifdef DEBUG_DETAIL
    printf("SMD::HandleError called %d\n", error);
#endif
    switch (error)
    {
    case DISK_ERR_NO_DISK_ATTACHED: // NO_DISK_ATTACHED
        data->statusRegister.bits.diskUnitNotReady = 1;
        break;

    case DISK_ERR_ADDRESS_MISMATCH: // ADDRESS_MISMATCH
        data->statusRegister.bits.addressMismatch = 1;
        break;

    case DISK_ERR_SEEK_ERROR:
        data->statusRegister.bits.diskUnitNotReady = 1;
        break;

    case DISK_ERR_READ_ERROR:
        data->statusRegister.bits.diskUnitNotReady = 1;
        break;

    case DISK_ERR_COMPARER_ERROR:
        data->statusRegister.bits.comparerError = 1;
        break;

    case DISK_ERR_DRIVE_NOT_SELECTED: // DRIVE_NOT_SELECTED
        data->statusRegister.bits.diskUnitNotReady = 1;
        break;

    case DISK_ERR_ILLEGAL_WHILE_ACTIVE: // ILLEGAL_WHILE_DRIVE_IS_ACTIVE
        data->statusRegister.bits.illegalLoad = 1;
        break;

    case DISK_ERR_WRITE_PROTECT_ERROR: // WRITE_PROTECT_ERROR
        // regs->writeProtectError = true;
        data->statusRegister.bits.diskUnitNotReady = 1;
        break;
    }
}

static uint32_t IncrementCoreAddress(ControllerRegs *regs)
{
    if (!regs)
        return 0;
    uint32_t address = (regs->coreAddressHiBits << 16) | regs->coreAddress;
    address++;
    regs->coreAddress = address & 0xFFFF;
    regs->coreAddressHiBits = (address >> 16) & 0xFF;
    return address;
}

static uint32_t DecrementWordCounter(ControllerRegs *regs)
{
    if (!regs)
        return 0;
    uint32_t counter = (regs->wordCounterHI << 16) | regs->wordCounter;
    counter--;
    regs->wordCounter = counter & 0xFFFF;
    regs->wordCounterHI = (counter >> 16) & 0xFF;
    return counter;
}

Device *CreateSMDDevice(uint8_t thumbwheel)
{
    Device *dev = (Device *)malloc(sizeof(Device));
    if (!dev)
        return NULL;

    SMDData *data = (SMDData *)malloc(sizeof(SMDData));
    if (!data)
    {
        free(dev);
        return NULL;
    }

    memset(dev, 0, sizeof(Device));
    memset(data, 0, sizeof(SMDData));

    // Initialize device base structure
    Device_Init(dev, thumbwheel, DEVICE_CLASS_BLOCK, 2048);

    dev->deviceData = data;

    data->controllerType = CONTR_SMD_15MHZ; // 10 and 15Mhz has flip-flops
    if (data->controllerType == CONTR_SMD_10MHZ || data->controllerType == CONTR_SMD_15MHZ)
    {
        data->regs.hasFlipFlops = true;
    }
    else
    {
        data->regs.hasFlipFlops = false;
    }

    // Initialize device properties
    data->bytes_pr_sector = 256; // Standard SMD sector size
    data->sectors_pr_track = 32; // Standard SMD sectors per track

    // Set up function pointers
    dev->Read = SMD_Read;
    dev->Write = SMD_Write;
    dev->Tick = SMD_Tick;
    dev->Reset = SMD_Reset;
    dev->Ident = SMD_Ident;
    dev->Boot = SMD_Boot;
    // Initialize device state
    SMD_Reset(dev);

    data->regs.maxUnits = 4; // Max disks
    data->regs.disks = malloc(sizeof(DiskInfo) * data->regs.maxUnits);
    if (!data->regs.disks)
    {

        free(data);
        free(dev);
        return NULL;
    }
    memset(data->regs.disks, 0, sizeof(DiskInfo) * data->regs.maxUnits);

    // Initialize disk info with default values
    for (int i = 0; i < data->regs.maxUnits; i++)
    {
        DiskSMD_Init(&data->regs.disks[i], i, NULL);
    }

    // Set up device address and interrupt settings based on thumbwheel
    switch (thumbwheel)
    {
    case 0:
        strcpy(dev->memoryName, "SMD 1540");
        dev->identCode = 017; // octal 017
        dev->startAddress = 01540;
        break;
    case 1:
        strcpy(dev->memoryName, "SMD 1550");
        dev->identCode = 020; // Octal 020
        dev->startAddress = 01550;
        break;
    case 2:
        strcpy(dev->memoryName, "SMD 540");
        dev->identCode = 023; // Octal 02
        dev->startAddress = 0540;
        break;
    case 3:
        strcpy(dev->memoryName, "SMD 550");
        dev->identCode = 06; // Octal 06
        dev->startAddress = 0550;
        break;
    default:
        printf("SMD: Unknown thumbwheel value: %d\n", thumbwheel);
        free(data);
        free(dev);
        return NULL;
    }

    dev->interruptLevel = 11; // disk
    dev->endAddress = dev->startAddress + 7;

    printf("SMD Device object created.\n");

    return dev;
}


/*
TPE>fun
Disc name: disc-75-1
Unit (0 to 3 oct): 0
On this Disc type, Function will destroy data in the last
cylinder in the spare track buffer pool !
(Including the Alternative Track Table.)

Do you still want to continue (YES or NO): Y

  1. Data way to controller test  === End of test ===
  2. Memory Address Register test === End of test ===
  3. Block Address Register test  === End of test ===
  4. Test-Mode test
Error after Compare-In-Test-Mode
Status and Failing Bits:  040010 001020
=== End of test ===
  5. Status Register bits test
Error after Return-To-Zero Seek, Bit 16b (on cylinder) remained 1 !
   Status 040070b, Unit 0
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true


Error. Status bit 14b is 0 when Bit 5b is 1 !
   Status 040070b, Unit 0
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true


Error after Write Format Word Count 10000b, Bit 6b (timeout) became 0.
   Status 040070b, Unit 0
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true


Error after Read (from NOT specified unit), Status Bit 7b is 0 !

   Status 040474b, Unit 4
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Write-Incorrect-Format
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Read (parity error expected), Status Bit 11b is 0 !
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Read
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Read (overrun expected), Status Bit 13b remains 0 !        7)
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Write (overrun expected), Status Bit 13b remains 0 !        7)
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Status bit 15b became 0 (Unit ready) after selection.
   Status 040474b, Unit 1
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Status bit 15b became 0 (Unit ready) after selection.
   Status 040474b, Unit 2
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Status bit 15b became 0 (Unit ready) after selection.
   Status 040474b, Unit 3
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Initiate Seek,
status bit 16b (on cylinder) became 1 immediately.
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Status bit 17b became 0 !
=== End of test ===
  6. Operation test
***ERROR***
   Status 040474b
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Not on-cylinder, or active.
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Read
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Write
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after Read
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


WRITE DOES NOT WORK (?).
After read data, complement it, write it back, read it again,
the second block of data is equal to the first !!

READ OR WRITE DOES NOT WORK (?).
After read data, complement it, write it back, read it again,
the complement of the second block of data is unequal to the first !!
Seek Complete Search (with no previous seek),
Status Bit 2b (active) remained 1 !!
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch

=== End of test ===
  7. Read-Seek-Condition test
***ERROR***
   Status 040474b
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error. Not on-cylinder, or active.
   Status 040474b, Unit 0
   - Controller active
   - Controller finished
   - Illegal load, i.e. load while status bit 2 is true
   - Address mismatch


Error after READ-SEEK-Condition, for a specified unit,
Bits 12b-10b (unit no.) are incorrect.
Seek-Condition 000001 Unit No.: 0


Error after READ-SEEK-Condition, for a specified unit,
With no previous Initiate-Seek,
Bits 7b-0b (Seek-Complete) are nonzero !
Seek-Condition 000001 Unit No.: 0


Error after READ-SEEK-Condition, for a specified unit,
With previous illegal block address,
Bit 13b (Seek Error) is 0 !
Seek-Condition 010401 Unit No.: 0

=== End of test ===
*/