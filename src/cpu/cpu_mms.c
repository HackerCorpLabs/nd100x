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


#include "cpu_types.h"
#include "cpu_protos.h"

//#define DEBUG_MMS
//#define DEBUG_MMS_MAPPING

// Global MMS type variable definition
MMSType mmsType = MMS2; // Change this to force MMS type to 1 or 2
PagingTables pt; // Global paging tables structure


// Create and initialize PagingTables
// MUST!!!! to be called before using the PagingTables
bool CreatePagingTables()
{    
    pt.mmsType = mmsType;
    
    // Allocate shadow RAM based on MMS type
    if (mmsType == MMS1)
    {
        pt.shadowRamSize = 512;  // 4 page tables = 2 x 64 bit * 4 = 512 Words
        pt.shadowRamAddress = SHADOW_RAM_EXTENDED_MODE_4PT;
    }
    else
    {
        pt.shadowRamSize = 2048; // 16 page tables = 16 x 64 bit * 4 = 2048 Words
        pt.shadowRamAddress = SHADOW_RAM_EXTENDED_MODE_16PT;
    }

    pt.shadowRam = (ushort*)calloc(pt.shadowRamSize, sizeof(ushort));
    if (!pt.shadowRam)
    {
        printf("Failed to allocate shadow RAM\n");
        return false;
    }

    pt.isInitialized = 1;
    return true;
}


// Helper functions
static uint ConvertFrom16BitPTE(ushort value)
{
    uint pte = (uint)((value & 0xFE00) << 16 | (value & 0x01FF));
    return pte;
}

static ushort ConvertTo16BitPTE(uint pageTableEntry)
{
    ushort res = (ushort)(((pageTableEntry & 0xFE000000) >> 16) | (pageTableEntry & 0x000001FF));
    return res;
}


// Used for debugging
#ifdef DEBUG_MMS
static uint CalcPageTableAddress(uint address)
{
    uint pageTableAddress;
    if (STS_SEXI)
    {
        // Extended, check if we have MM-1 or MM-II
        if (pt.mmsType == MMS1)
        {
            // 4 page tables start at 177000 (0xFE00)                    
            pageTableAddress = ((address - SHADOW_RAM_EXTENDED_MODE_4PT) & 0x1FF) >> 1;
        }
        else
        {
            // 16 page tables start at 174000 (0xF800)
            pageTableAddress = ((address - SHADOW_RAM_EXTENDED_MODE_16PT) & 0x7FF) >> 1;
        }
    }
    else
    {
        // Normal mode, 4 page tables, all start at 177400 (FF00)
        pageTableAddress = (address & 0x00ff);
    }
    return pageTableAddress;
}
#endif

// Clean up PagingTables
void DestroyPagingTables()
{
    if (pt.shadowRam)
    {
        free(pt.shadowRam);
    }
}


// Calculate offset into ShadowRam array
ushort GetPTShadowAddress(uint pageTable, uint VPN, PageTableMode ptm)
{
    uint offset = 0;

    if (STS_SEXI)
    {
        // EXTENDED MODE
        switch (ptm)
        {
            case Four:
                offset = SHADOW_RAM_EXTENDED_MODE_4PT - pt.shadowRamAddress;
                break;
            case Sixteen: // ONLY for MMS2
                offset = SHADOW_RAM_EXTENDED_MODE_16PT - pt.shadowRamAddress;
                break;
        }
    }
    else
    {
        // Normal mode
        offset = SHADOW_RAM_NORMAL_MODE_4PT - pt.shadowRamAddress;
    }

    uint pageTableAddress = (pageTable << 6) | VPN;
    if (STS_SEXI)
    {
        // Extended mode, PTe is stored in 2x 16 bits memory addresses
        pageTableAddress = pageTableAddress << 1; // left shift1 == *2
    }
    pageTableAddress += offset;

    return (ushort)pageTableAddress;
}

// Write to page tables
void PT_Write(uint address, ushort value)
{
    if (!pt.shadowRam) return;
    if ((address < pt.shadowRamAddress) || (address > 0xFFFF)) return;

    uint offset = address - pt.shadowRamAddress;
    pt.shadowRam[offset] = value;

#ifdef DEBUG_MMS
    uint pageTableEntry;
    uint pageTableAddress = CalcPageTableAddress(address);
    uint pageTable = pageTableAddress >> 6;

    if (!STS_SEXI)
    {
        pageTableEntry = ConvertFrom16BitPTE(value);
    }
    else
    {
        if ((address & 0x01) == 0)
        {
            // Even address
            pageTableEntry = (uint)(value << 16 | pt.shadowRam[offset + 1]);
        }
        else
        {
            // Odd address
            pageTableEntry = (uint)(pt.shadowRam[offset - 1] << 16 | value);
        }
    }
    printf("PT W A=%o PT=%d VPN=%d SEXI=%d V=%o => 0x%08X (%s)\n",  address, pageTable, pageTableAddress & 0x3F, STS_SEXI, value,  pageTableEntry, GetPageTableEntryDebugInfo(pageTableEntry));
#endif
}

// Read from shadow mem/pagetables
ushort PT_Read(uint address)
{
    if (!pt.shadowRam) return 0;
    if ((address < pt.shadowRamAddress) || (address > 0xFFFF)) return 0;

    uint offset = address - pt.shadowRamAddress;
    ushort res = pt.shadowRam[offset];

#ifdef DEBUG_CPU
    uint pageTableEntry;
    uint pageTableAddress = CalcPageTableAddress(pt, address);
    uint pageTable = pageTableAddress >> 6;

    if (!SEXI)
    {
        pageTableEntry = ConvertFrom16BitPTE(res);
    }
    else
    {
        if ((address & 0x01) == 0)
        {
            // Even address
            pageTableEntry = (uint)(res << 16 | pt->shadowRam[offset + 1]);
        }
        else
        {
            // Odd address
            pageTableEntry = (uint)(pt->shadowRam[offset - 1] << 16 | res);
        }
    }
    printf("PT R A=%o PT=%d VPN=%d SEXI=%d V=%o <= 0x%08X (%s)\n", 
           address, pageTable, pageTableAddress & 0x3F, SEXI, res, 
           pageTableEntry, GetPageTableEntryDebugInfo(pt, pageTableEntry, SEXI));
#endif

    return res;
}

// Get page table entry
uint GetPageTableEntry(uint pageTable, uint VPN,PageTableMode ptm)
{
    if (!pt.shadowRam) return 0;
    if (pageTable >= 16) return 0;

    uint PTe = 0;
    int pageTableAddress = GetPTShadowAddress(pageTable, VPN, ptm);

    if (STS_SEXI)
    {
        PTe = (uint)(pt.shadowRam[pageTableAddress] << 16 | pt.shadowRam[pageTableAddress + 1]);
    }
    else
    {
        if (pageTable <= 3)
        {
            PTe = ConvertFrom16BitPTE(pt.shadowRam[pageTableAddress]);
        }
    }

    return PTe;
}

// Get page table entry for debugger/inspector use.
//
// GetPageTableEntry() checks STS_SEXI to decide shadow RAM format. STS_SEXI
// reflects the SEXI flag of the currently executing interrupt level, so when
// SEXI is off, PT 4-15 return 0. This is correct for normal CPU operation.
//
// The JS debugger however needs to read any page table at any time, including
// DPIT (PT#7) for SINTRAN kernel inspection. When the CPU is paused, the
// current level may not have SEXI set, even though SINTRAN loaded entries
// into PT 4-15 while running at a level with SEXI enabled. The shadow RAM
// content is still valid - the hardware format (MMS1=4PT/16-bit vs
// MMS2=16PT/32-bit) is fixed and does not depend on the runtime SEXI state.
//
// This function uses mmsType instead of STS_SEXI so the debugger can always
// read all page tables regardless of which level happens to be active.
uint GetPageTableEntryForDebugger(uint pageTable, uint VPN, PageTableMode ptm)
{
    if (!pt.shadowRam) return 0;
    if (pageTable >= 16) return 0;

    uint PTe = 0;

    if (mmsType == MMS2)
    {
        // MMS2 hardware: always 32-bit PTEs in extended 16PT area
        uint offset = SHADOW_RAM_EXTENDED_MODE_16PT - pt.shadowRamAddress;
        uint pageTableAddress = ((pageTable << 6) | VPN) << 1;
        pageTableAddress += offset;
        PTe = (uint)(pt.shadowRam[pageTableAddress] << 16 | pt.shadowRam[pageTableAddress + 1]);
    }
    else
    {
        // MMS1 hardware: only 4 page tables, 16-bit PTEs
        if (pageTable <= 3)
        {
            uint offset = SHADOW_RAM_NORMAL_MODE_4PT - pt.shadowRamAddress;
            uint pageTableAddress = (pageTable << 6) | VPN;
            pageTableAddress += offset;
            PTe = ConvertFrom16BitPTE(pt.shadowRam[pageTableAddress]);
        }
    }

    return PTe;
}

// Update page table entry
bool UpdatePageTableEntry(uint pageTable, uint VPN, PageTableMode ptm, uint PTe)
{
    if (!pt.shadowRam) return false;
    if (pageTable >= 16) return false;

    int pageTableAddress = GetPTShadowAddress(pageTable, VPN, ptm);

    if (STS_SEXI)
    {
        pt.shadowRam[pageTableAddress] = (ushort)(PTe >> 16);
        pt.shadowRam[pageTableAddress + 1] = (ushort)(PTe);
    }
    else
    {
        pt.shadowRam[pageTableAddress] = ConvertTo16BitPTE(PTe);
    }

    return true;
}

// Set page used flag
uint SetPageUsed(uint pageTable, uint VPN, PageTableMode ptm, uint PTe)
{    
    if ((PTe & PGU_FLAG) == 0)
    {
        PTe |= PGU_FLAG;
        UpdatePageTableEntry(pageTable, VPN, ptm, PTe);

#ifdef DEBUG_MMS
        printf("PageTable PGU - PT=%d VPN=%d => Entry=0x%08X (%s)\n",  pageTable, VPN, PTe, GetPageTableEntryDebugInfo(PTe));
#endif
    }
    return PTe;
}

// Set page written flag
uint SetPageWritten(uint pageTable, uint VPN,PageTableMode ptm, uint PTe)
{
    if (!pt.shadowRam) return PTe;

    if (pageTable >= 16) return PTe;

    if ((PTe & WIP_FLAG) == 0)
    {
        PTe |= WIP_FLAG;
        UpdatePageTableEntry( pageTable, VPN, ptm, PTe);

#ifdef DEBUG_MMS
        printf("PageTable WIP - PT=%d VPN=%d => Entry=0x%08X (%s)\n",  pageTable, VPN, PTe, GetPageTableEntryDebugInfo(PTe));
#endif
    }
    return PTe;
}


// Get debug info for page table entry
char* GetPageTableEntryDebugInfo(ulong PTe)
{
#ifdef DEBUG_MMS
    static char debugInfo[256];
    debugInfo[0] = '\0';

    // Map to physical page
    ushort PPN = 0;
    if (STS_SEXI)
    {
        // Use lower 14-bit
        PPN = (ushort)(PTe & 0x3FFF);
    }
    else
    {
        // "normal" mode, use only the lower 9-bits
        PPN = (ushort)(PTe & 0x1FF);
    }

    PTe = PTe >> 16;

    if ((PTe & 1 << 15) != 0) strcat(debugInfo, "[WPM]");
    if ((PTe & 1 << 14) != 0) strcat(debugInfo, "[RPM]");
    if ((PTe & 1 << 13) != 0) strcat(debugInfo, "[FPM]");
    if ((PTe & 1 << 12) != 0) strcat(debugInfo, "[WIP]");
    if ((PTe & 1 << 11) != 0) strcat(debugInfo, "[PGU]");

    int ring = (int)((PTe >> 9) & 0x03);
    char ringStr[8];
    snprintf(ringStr, sizeof(ringStr), "[R:%d]", ring);
    strcat(debugInfo, ringStr);

    char ppnStr[16];
    snprintf(ppnStr, sizeof(ppnStr), "[PPN:0x%04X]", PPN);
    strcat(debugInfo, ppnStr);

    return debugInfo;
#else
    return "";
#endif
}


// Map virtual address to physical address
int mapVirtualToPhysical(uint virtualAddress, AccessMode am, bool UseAPT)
{
    
    
    if (!pt.isInitialized)
    {
        printf("FATAL! PagingTables not initialized\n");
        exit(1);
    }

    virtualAddress = virtualAddress & 0xFFFF; // Make sure it's no more than 16-bits
    uint pageTable = 0;

    // Read PCR for the current level and calculate the ring we are executing the code under
    uint16_t pcr = gReg->reg_PCR[CurrLEVEL];
    uint8_t ring = pcr & 0x03;  // 2 lower bits of the PCR is the Ring the current level is using

    // Ring 3 is the most powerful, and for Ring 3 RAM will always be in the shadow of PageTable RAM
    if ((ring == 3) && (IsAddressShadowMemory(virtualAddress, false)))
    {
        return (int)virtualAddress; // Read/WritePhysical will handle the actual access to shadow memory
    }

    // If memory management is not enabled, don't use mapping (physical = virtual)
    if (!STS_PONI) return (int)(virtualAddress & 0xFFFF);

    // Calculate VPN and DIP
    uint DIP = virtualAddress & 0x3FF; // lower 10 bits - Displacement
    uint VPN = (virtualAddress >> 10) & 0x3F; // upper 6 bits - Virtual Page number

    PageTableMode ptm = Four; // Default to four page tables

    // Find PageTable Number and identify if we have the optional 16 page-table mode
    if ((STS_PTM) && (UseAPT))
    {
        if ((pcr & (1 << 2)) != 0 && (mmsType == MMS2))
        {
            // Sixteen page table mode
            pageTable = (pcr >> 7) & 0xF; // AlternativePageTable - 16x
            ptm = Sixteen;
        }
        else
        {
            // Four page table mode
            pageTable = (pcr >> 7) & 0x03; // AlternativePageTable - 4x
            ptm = Four;
        }
    }
    else
    {
        if ((pcr & (1 << 2)) != 0 && (mmsType == MMS2))
        {
            // Sixteen page table mode
            pageTable = (pcr >> 11) & 0xF; // PageTable - 16x
            ptm = Sixteen;
        }
        else
        {
            // Four page table mode
            pageTable = (pcr >> 9) & 0x03; // PageTable - 4x
            ptm = Four;
        }
    }

    // Find the PageTableEntry, PTe
    uint32_t pageTableEntry = GetPageTableEntry(pageTable, VPN, ptm);

#ifdef DEBUG_MMS_MAPPING
    printf("mapVirtualToPhysical - PT=%d VPN=%d => Entry=0x%08X (%s)\n",  pageTable, VPN, pageTableEntry, GetPageTableEntryDebugInfo(pageTableEntry));
#endif    

    // Check for page protection
    if (!checkPageProtection(VPN, pageTable, pageTableEntry, UseAPT, am, virtualAddress))
    {
         // We should never get here, but added a return statement anyway! (Will end up here if interrupts are disabled?)
        return -1;
    }

    uint8_t pageTableRing = (pageTableEntry >> 25) & 0x03;

#ifdef _DEGRADE_
    // Degrade ?

    // If a program in ring 3 executes instructions assigned to rings 0,1 or 2, its ring number is reduced accordingly.
    // Such accessess are detected by hardware which automatically changes the ring number in the PCR register for the current program level.
    // NB! This degrading only occurs when lower ring instruction codes are executed, but not when data is accessed
    if ((am & FETCH) && (pageTableRing < ring) && (ring == 3))
    {
        ring = pageTableRing;
        gReg->reg_PCR[CurrLEVEL] = (gReg->reg_PCR[CurrLEVEL] & 0xFFFC) | ring;
    }
#endif

    // Check for Ring Protection
    // INFO: For the ND CPU Ring 3 is most powerfull, ring 0 least powerfull.
    // If the current level has a "ring level" that is smaller than the ring level on the page, generate a fault
    //
    // The ring bits of the appropriate PCR are compared with the ring bits of the appropriate page table entry.
    // The PCR ring bits should always be greater than or equal to the PT ring bits. If not, an internal interrupt (MPV) will be generated.

    if (ring < pageTableRing)
    {        
        UpdatePGS(pageTable, VPN, am, false);                
#ifdef DEBUG_MMS
        printf("[%d] Ring Protection Violation. Ring=%d PTRing=%d Accessmode=%d PGS=%06o PT=%d VPN=%d PTe=0x%08X\n", 
               CurrLEVEL, ring, pageTableRing, am, gReg->reg_PGS, pageTable, VPN, pageTableEntry);
#endif               
        HandleMPV(virtualAddress);
        return -1;
    }

    // Map to physical page
    uint16_t PPN = 0;
    if (STS_SEXI)
    {
        // Use lower 14-bit
        PPN = (uint16_t)(pageTableEntry & 0x3FFF);
    }
    else
    {
        // "normal" mode, use only the lower 9-bits
        PPN = (uint16_t)(pageTableEntry & 0x1FF);
    }

    // Calculate physical address
    int physicalAddress = (PPN << 10) | DIP;

    // Check if memory is out of range (Zero based range, so make sure it's not greater than ND_Memsize)
    if (physicalAddress >= ND_Memsize)
    {
        UpdatePGS(pageTable, VPN, am, false);
        HandleMemoryOutOfRange(physicalAddress);
        return -1;
    }

    // Mark page used
    pageTableEntry = SetPageUsed(pageTable, VPN, ptm, pageTableEntry);
    if (am == WRITE)
    {
        // Mark page as "written to"
        pageTableEntry = SetPageWritten(pageTable, VPN, ptm, pageTableEntry);
    }

    // Check for ECC Memory Parity
    if ((gECCR & (1 << 3)) == 0) // If Bit 3 is set, ECC is disabled
    {
        int eccBits = 0;
        if ((gECCR & 1 << 0) != 0) eccBits++; // Simulate memory error in bit 0
        if ((gECCR & 1 << 1) != 0) eccBits++; // Simulate memory error in bit 15
        if ((gECCR & 1 << 4) != 0) eccBits++; // Simulate memory error in bit 6

        if (eccBits > 0)
        {
            uint16_t tmpPEA = physicalAddress & 0xFFFF;
            uint16_t tmpPES = (physicalAddress >> 16) & 0xFF;
            uint16_t errorCode = 0;

            if (eccBits == 1)
            {
                // Single bit, error table  Figure 2.18, page 2-51 in ND-06.014.02
                if ((gECCR & 1 << 0) != 0) errorCode = 3; // Simulate memory error in bit 0
                if ((gECCR & 1 << 1) != 0) errorCode = 0x1C; // Simulate memory error in bit 15
                if ((gECCR & 1 << 4) != 0) errorCode = 0x0D; // Simulate memory error in bit 6
                tmpPES |= errorCode << 8;
            }
            else
            {
                // no error code as its multiple bits	
                tmpPES |= 1 << 13; // FATAL ERROR
            }

            if (am & FETCH)
            {
                 // Error during fetch (or DEPOSIT or EXAM)
                tmpPES |= 1 << 15; // Error during fetch
            }

            setPEA(tmpPEA);
            setPES(tmpPES);

            interrupt(14, 1 << 8); // PTY - MEMORY_PARITY_ERROR bit 8            
        }
    }

#ifdef DEBUG_MMS
    if (physicalAddress == 0)
    {
        printf("mapVirtualToPhysical - PT=%d VPN=%d => Entry=0x%08X (%s)\n",  pageTable, VPN, pageTableEntry, GetPageTableEntryDebugInfo(pageTableEntry));
    }
#endif
    return (int)physicalAddress;
}

// Update PGS (Page Status) register
void UpdatePGS(uint pageTable, uint VPN, AccessMode am, bool permitViolation)
{
    ushort tmpPGS = (pageTable << 6) | VPN;

    // Permit violation (read, write, fetch protect system)
    if (permitViolation) tmpPGS |= (1 << 14);


    if (am & FETCH)
    {
        if (am & READ)
        {
            // READ_FETCH - Indirect read during effective address calculation
            //printf("PGS update on Indirect read (READ_FETCH) PT=%d VPN=%d Accessmode=%d PGS<=%o\n", pageTable, VPN, am, tmpPGS);            
        }
        else
        {
            tmpPGS |= (1 << 15);
        }
    }

    setPGS(tmpPGS);    
}

// Check page protection
bool checkPageProtection(uint VPN, uint pageTable, ulong pageTableEntry, bool UseAPT, AccessMode am, uint virtualAddress)
{
    ulong accessBits = 0;
    ulong pfMask = 7L << 29;

    if (am & READ)  accessBits |= 1L << 30; // RPM(Read Permit bit)
    if (am & WRITE) accessBits |= 1L << 31; // WPM (Write Permit bit)
    if (am & FETCH) accessBits |= 1L << 29; // FPM (Fetch Permit bit)

    // Check if page is in memory
    // Page 89 (Chapter 3) in ND-110 Functional Description
    // If the combination of WPM, RPM and FPM are all zero, this is interpreted as page not in memory and will generate an internal interrupt as page fault
    if ((pageTableEntry & pfMask) == 0)
    {
        UpdatePGS(pageTable, VPN, am, true);        
        HandlePF(virtualAddress);
        return false;
    }

    // Check access permissions
    if ((pageTableEntry & accessBits) == 0)
    {
        UpdatePGS(pageTable, VPN, am, true);
        HandleMPV(virtualAddress);
        return false;
    }

    return true;
}

// Check if address is in shadow memory
bool IsAddressShadowMemory(uint addr, bool privileged)
{
    ushort pcr = gReg->reg_PCR[CurrLEVEL];
    unsigned char ring = pcr & 0x03;
    bool mms2Enabled = ((pcr & 1 << 2) != 0); // Is MMS-2 with 16-page-tables enabled on this PCR level ?


    if ((ring == 3) || (!STS_PONI) || privileged)
    {
        
        if (STS_SEXI)
        {
            if ((mmsType == MMS2) && mms2Enabled)
            {
                if ((addr >= 0xF800) && (addr <= 0xFFFF))
                {
                    return true;
                }
            }
            else
            {
                if ((addr >= 0xFE00) && (addr <= 0xFFFF))
                {
                    return true;
                }
            }
        }
        else
        {
            if ((addr >= 0xFF00) && (addr <= 0xFFFF))
            {
                return true;
            }
        }
    }

    return false;
}

// Read from virtual memory
int ReadVirtualMemory(uint virtualAddress, bool UseAPT)
{    
    if (DISASM)
		disasm_set_isdata(virtualAddress);

    int pa = mapVirtualToPhysical(virtualAddress, READ, UseAPT);
    return ReadPhysicalMemory(pa, false);
}

// Read indirect from virtual memory (used only for effective address calculation)
int ReadIndirectVirtualMemory(uint virtualAddress, bool UseAPT)
{    
    int pa = mapVirtualToPhysical(virtualAddress, READ_FETCH, UseAPT);
    return ReadPhysicalMemory(pa, false);
}

// Fetch from virtual memory
int FetchVirtualMemory(uint virtualAddress, bool UseAPT)
{
    int pa = mapVirtualToPhysical(virtualAddress, FETCH, UseAPT);
    return ReadPhysicalMemory(pa, false);
}

// Write to virtual memory
void WriteVirtualMemory(uint virtualAddress, ushort value, bool UseAPT, WriteMode wm)
{
    if (DISASM)
		disasm_set_isdata(virtualAddress);

    int pa = mapVirtualToPhysical(virtualAddress, WRITE, UseAPT);
    WritePhysicalMemoryWM(pa, value, false,wm);
}




// Read from physical memory
int ReadPhysicalMemory(int physicalAddress, bool privileged)
{
    if (physicalAddress <0)
    {
        //printf("Memory Protection!! But it wasn't caught. Should have been aborted!\n");
        return 0x00;
    }

    if (IsAddressShadowMemory(physicalAddress, privileged))
    {
        int tmp = PT_Read(physicalAddress);
        //printf("ReadPhysicalMemory: Shadow Memory %4X = %4X\n", physicalAddress, tmp);
        return tmp;
    }

    // Check memory bounds
    if ((physicalAddress >= ND_Memsize)||(physicalAddress < 0))
    {
        HandleMemoryOutOfRange(physicalAddress);
        return 0x00;
    }

    return VolatileMemory.n_Array[physicalAddress];
}

// Wrapper for WritePhysicalMemoryWM to write a word (16 bits)
void WritePhysicalMemory(int physicalAddress, uint16_t value, bool privileged)
{
    WritePhysicalMemoryWM(physicalAddress, value,privileged, WRITEMODE_WORD);
}


// Write to physical memory (with writemode to handle MSB/LSB/WORD)
void WritePhysicalMemoryWM(int physicalAddress, uint16_t value, bool privileged, WriteMode wm)
{
    if (IsAddressShadowMemory(physicalAddress, privileged))
    {
        //printf("WritePhysicalMemoryWM: Shadow Memory 0x[%4X] = 0x%4X\n", physicalAddress, value);

        if (wm != WRITEMODE_WORD)
        {
            printf("ouch, not implemented yet\n");
            exit(1);
        }
        PT_Write(physicalAddress, value);
        return;
    }

    // Check memory bounds
    if ((physicalAddress > ND_Memsize)||(physicalAddress < 0))
    {
        //printf("Memory out of range error: address %d is outside valid range [0, %d]\n", physicalAddress, ND_Memsize);
        HandleMemoryOutOfRange(physicalAddress);
        return;
    }

    ushort *p_phy_addr;
    p_phy_addr = &VolatileMemory.n_Array[physicalAddress];

	switch (wm)
	{
	case WRITEMODE_MSB: /* Even, which means MSB byte, or bits 15-8 */
		*p_phy_addr = (*p_phy_addr & 0xFF) | (value << 8);
		break;
	case WRITEMODE_LSB: /*Odd, which means LSB byte, or bits 7-0 */
		*p_phy_addr = (*p_phy_addr & 0xFF00) | (value & 0xFF);
		break;
    case WRITEMODE_WORD: // full word
	default:
		*p_phy_addr = value;
		break;
	}

/*

	ushort *p_phy_addr;
	p_phy_addr = &VolatileMemory.n_Array[physicalAddress];
	*p_phy_addr = value;
 */
}

// Handle memory out of range error
void HandleMemoryOutOfRange(uint physicalAddress)
{
    setPEA(physicalAddress & 0xFFFF);
    setPES((physicalAddress >> 16) & 0xFF);

    interrupt(14, 1 << 9); // Memory out of range
}

/// @brief Handle memory protection violation. Will TRAP the instruction
/// @param virtualAddress 
void HandleMPV(uint virtualAddress)
{
    //printf("HandleMPV: %06o\n", virtualAddress);
    interrupt(14, 1 << 2); // MPV - MEMORY_PROTECTION_VIOLATION bit 2            
}

/// @brief Handle page fault. Will TRAP the instruction
/// @param virtualAddress 
void HandlePF(uint virtualAddress)
{
    //printf("HandlePF: %06o\n", virtualAddress);
    interrupt(14, 1 << 3); // PF - PAGE_FAULT bit 3
}

// Check if privileged instruction execution is allowed
