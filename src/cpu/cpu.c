/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2006 Per-Olof Astrom
 * Copyright (c) 2006-2008 Roger Abrahamsson
 * Copyright (c) 2008 Zdravko
 * Copyright (c) 2025 Ronny Hansen
 *
 * This file is originated from the nd100em project.
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

// #define DEBUG_TRAP
//#define DEBUG_PK_SWITCH
//  #define DEBUG_IONOFF

#include <string.h>
#include <setjmp.h>

#include "cpu_types.h"
#include "cpu_protos.h"

#include "../machine/machine_types.h"
#include "../machine/machine_protos.h"


//#define DEBUG_TRAP

// Global CPU variable definitions
_NDRAM_ VolatileMemory;
_RUNMODE_ CurrentCPURunMode;
_CPUTYPE_ CurrentCPUType;

struct CpuRegs *gReg = NULL;

uint64_t  instr_counter = 0;
ushort STARTADDR = 0;
int DISASM = 0;

/* Instruction handling array */



//void (*instr_funcs[65536])(ushort);



// Used for TRAP handling to exit an instruction that fails
jmp_buf cpu_jmp_buf;



void do_op(ushort operand, bool isEXR)
{

	if (!isEXR)
		gPC++; // Move P before starting instruction. (but not if executed from register)

	if (instr_funcs[operand] == NULL)
	{		
		unimplemented_instr(operand); // TODO: check if this is correct
		return;
	}
	instr_funcs[operand](operand); /* call using a function pointer from the array
				   this way we are as flexible as possible as we
				   implement io calls. */


}





/* Calculates the effective address to use.
 * Uses MemoryRead to do this so we get the Page Table handling
 * done correctly. Also sets the bool use_apt points to, to tell caller what PT
 * to use for the actual use of the address supplied
 * See Manual ND.06.014, Page 34
 */
ushort New_GetEffectiveAddr(ushort instr, bool *use_apt)
{
	int disp = signExtend(instr & 0xFF);
	ushort eff_addr;

	ushort P = (gPC - 1) & 0xFFFF;

	switch ((instr >> 8) & 0x07)
	{
	case 0: /* (P) + disp */
		eff_addr = P + disp;
		*use_apt = false;
		break;
	case 1: /* (B) + disp */
		eff_addr = gB + disp;
		*use_apt = true;
		break;
	case 2: /* ((P) + disp) */
		eff_addr = P + disp;
		eff_addr = ReadIndirectVirtualMemory(eff_addr, false);
		*use_apt = true;
		break;
	case 3: /* ((B) + disp) */
		eff_addr = gB + disp;
		eff_addr = ReadIndirectVirtualMemory(eff_addr, true);
		*use_apt = true;
		break;
	case 4: /* (X) + disp */
		eff_addr = gX + disp;
		*use_apt = true;
		break;
	case 5: /* (B) + disp + (X) */
		eff_addr = gB + gX + disp;
		*use_apt = true;
		break;
	case 6: /* ((P) + disp) + (X) */
		eff_addr = P + disp;
		eff_addr = gX + ReadIndirectVirtualMemory(eff_addr, false);
		*use_apt = true;
		break;
	case 7: /* ((B) + disp) + (X) */
		eff_addr = (gB + disp) & 0xFFFF;
		eff_addr = gX + ReadIndirectVirtualMemory(eff_addr, true);
		*use_apt = true;
		break;
	}
	return eff_addr;
}


// Calculate internal Interrupt
/// <summary>
/// IIC - Internal Interrupt Code
///
/// This register will hold a code between 0 - 12 (oct), which will identify the internal source for the interrupt.
/// Priority encoded IID | IIE
/// </summary>
///
///                   (oct)
///      | IED bit  | IIC code |
///  ----+----------+----------+------------------------------------------------------------------------
///  n/a |   0      |    0     | Not assigned
///  MC  |   1      |    1     | Monitor Call
///  PV  |   2      |    2     | Protect Violation. Page number is found in the Paging Status Register.
///  PF  |   3      |    3     | Page fault. Page not in memory.
///  II  |   4      |    4     | lllegal instruction. Not implemented instruction.
///  Z   |   5      |    5     | Error indicator. The Z indicator is set.
///  PI  |   6      |    6     | Privileged instruction.
///  IOX |   7      |    7     | IOX error. No answer from external device.
///  PTY |   8      |    10    | Memory parity error
///  MOR |   9      |    11    | Memory out of range Addressing non-existent memory.
///  POW |   10     |    12    | Power fail interrupt
///  ----+----------+----------+------------------------------------------------------------------------
ushort calcIIC()
{
	ushort priorityCode = gIID & gIIE;
	if (priorityCode == 0)
		return 0;

	// printf("IID=0x%x, IIE=0x%x, priorityCode=0x%x\r\n", gIID, gIIE, priorityCode);

	for (int i = 10; i >= 0; i--)
	{
		if ((priorityCode & (1 << i)) != 0)
		{
			return (ushort)i;
		}
	}
	return 0;
}





/*
 * Recalculate internal interrupt bits
 * Updates gIID, gPID and gPK
 */
void recalcInternalInterruptBits()
{
	// Check for Z (error) flag	
	if (getbit(_STS, _Z))
	{
		gIID |= 1 << 5;
	}

	if ((gIID & gIIE) != 0)
	{
		// Set PID bit 14 to trigger LVL change to 14
		gPID |= (1 << 14);
		gIIC = calcIIC();
		gCHKIT = true; // removing this makes sintran crash during boot  // System malfunction. Sintran halt in ERRFATAL. L-reg: 042713
	}
}

// Calculate PK based on PID and PIE
void calcPK()
{
	// Recalculate PK based on PID and PIE
	int s;
	ushort lvl;
	ushort i;
	gPK = 0;
	i = gPIE & gPID;
		
	if (i)
	{
		// Check for detected and enabled bits. Highest bits has highest priority
		for (lvl = 15; lvl >= 0; lvl--)
		{
			if (i & 1 << lvl)
			{
				gPK = lvl;
				return;
			}
		}
	}
}

/*
 * Internal interrupt setting routine.
 * IN: interrupt level and possible subbitfield
 * for those levels that has that. (LVL 14).
 */
void interrupt(ushort lvl, ushort sub)
{
	int s;
	if (lvl == 14)
	{
		gIID |= sub;
		if (gIID & gIIE)
			gPID |= (1 << 14);
	}
	else
	{
		gPID |= (1 << lvl);
	}

	// printf("Interrupt at %d, sub=0x%x. GID_BIT_= %d\r\n", lvl, sub, (gIID>>8)&1);
	recalcInternalInterruptBits();

	// Check for MPV (bit 2), PF (bit 3), or illegal instruction (bit 4)
	if (lvl == 14 && (sub & ((1 << 2) | (1 << 3) | (1 << 4))))
	{
#ifdef DEBUG_TRAP
		printf("TRAP at P:[%6o], sub=%d \r\n", gPC, sub);
#endif
		longjmp(cpu_jmp_buf, 1); // Jump back to cpurun() in cpu_thread
	}
}

void device_interrupt(ushort interruptBits)
{
	// Only process bits 10-13 and 15 for device interrupts
	ushort validBits = interruptBits & 0xBC00; // Mask for bits 10-13,15 (0b1111010000000000)

	ushort tmp = gPID;

	// clear gIID bits 10-13,15
	gPID &= ~validBits;

	// set gIID bits from device(s)
	gPID |= validBits;

	if (tmp != gPID)
	{
		gCHKIT = true; // Check if we need to update PK based on new interrupts		
	}
}


/*
 * Routine that handles phys mem writes and shadow memory.
 */
void PhysMemWrite(ushort value, ulong addr)
{
	WritePhysicalMemory(addr, value, false); // in cpu_mms.c
	return;


}

/*
 * Routine that handles phys mem reads and shadow memory.
 */
ushort PhysMemRead(ulong addr)
{
	return ReadPhysicalMemory(addr, false); // in cpu_mms.c

}

/*
 * Write a word to memory.
 * Here we implement all Memory Management System functions.
 */
void MemoryWrite(ushort value, ushort addr, bool UseAPT, unsigned char byte_select)
{
	WriteVirtualMemory(addr, value, UseAPT, byte_select); // in cpu_mms.c
}

/*
 * Read a word from memory.
 * Here we implement all Memory Management System functions.
 */
ushort MemoryRead(ushort addr, bool UseAPT)
{
	return ReadVirtualMemory(addr, UseAPT); // in cpu_mms.c
}

ushort MemoryFetch(ushort addr, bool UseAPT)
{
	return FetchVirtualMemory(addr, UseAPT); // in cpu_mms.c
}

/// @brief Check if we need to switch runlevel
/// @return Returns true if a switch was made, false otherwise
bool checkAndSwitch()
{
	if (gCHKIT)
	{
		gCHKIT = false; // reset flag

		// recalc internal interrupt bits
		recalcInternalInterruptBits();

		if (!STS_IONI)
			return false;

		calcPK();

		if (gPK != gPIL)
		{
			//printf("Switching from %d P[%6o] to %d P[%6o]\r\n", gPIL, gPC, gPK, gReg->reg[gPK][_P]);
			setPIL(gPK); /* Change to new runlevel */

#ifdef DEBUG_PK_SWITCH
			bool isRTC = ((gPVL == 13) || (gPIL == 13));
			if (!isRTC)
			{
				printf("Switched from %d P[%6o] to %d P[%6o]\r\n", gPVL, gReg->reg[gPVL][_P], gPIL, gReg->reg[gPIL][_P]);
				printf("New pc after switch %6o\r\n", gPC);
			}
#endif
			return true;
		}
	}
	return false;
}



// To reduce host load, we enable sleeping when the ND CPU is idle
// The ND CPU is idle when running in level 0 in SINTRAN.
// We detect that the CPU is idle by checking the gPIL register is == 0
// But we only activate sleep when the CPU is idle for a while, and after it has been in another PIL level to achieve a quick boot.
ushort lvlcnt=0;
bool activateSleep = false;

// allocate once
ushort operand;

/// @brief CPU tick function - DO NOT CALL THIS DIRECT AS IT NEES setjmp() setup correctly
/// @details This function is called every CPU tick. It fetches the next instruction, executes it, and handles interrupts.
void private_cpu_tick()
{
	// Check for level shift (typically after an interrupt or WAIT instruction)
	checkAndSwitch();

	// Fetch next instruction
	gReg->myreg_PFB = MemoryFetch(gPC, false); //TODO: Remove this  step?
	gReg->myreg_IR = gReg->myreg_PFB;

	operand = gReg->myreg_IR;	


	// Dissasemble ?
	if (DISASM)
		disasm_instr(gPC, operand);


	if (gPIL>0)
		activateSleep = true;


	if (activateSleep)
	{
		// Check if we need to sleep
		lvlcnt = (gPIL == 0) ? (lvlcnt + 1) : 0;

		if (lvlcnt > 10000)
		{			
			sleep_ms(1); // Sleep 1 ms
			lvlcnt = 0;
		}
	}

	// Execute instruction
	instr_counter++;
	do_op(operand, false);
}

/// @brief run the CPU for a number of ticks. Use -1 for infinite.
/// @details This function runs the CPU for a number of ticks. It handles interrupts and checks for level switches.
/// @param ticks 
void cpu_run(int ticks)
{

	// Set up longjmp target once at startup
	if (setjmp(cpu_jmp_buf) != 0)
	{
		// We had an interrupt (MPV, PF, or illegal instruction)
		// PGS bit 15 indicates if fault was during fetch (1) or data cycle (0)

		// PGS:
		//
		// if bit 15 is a one, the page fault or protection violation occurred during the fetch of an instruction.
		// In this case, the P register has not been incremented and the instruction causing the violation(and the restart point)
		//
		// If bit 15 is zero, the page fault or protection violation occurred during the data cycles of an instruction.
		// In this case, the P register points to the instruction after the instruction causing the internal hardware status interrupt.
		// When the cause of the internal hardware status interrupt has been removed, the restart point will be found by subtracting one from the P register.

#ifdef DEBUG_TRAP
		printf("CPU: Interrupt handler returned, PC=%06o, PGS=%04x\n", gPC, gPGS);
#endif
	}


	while ((CurrentCPURunMode != SHUTDOWN) && (ticks !=0 ))
	{
		if (CurrentCPURunMode != STOP)
		{
			private_cpu_tick();

			// Tick IO devices pr cpu tick
			IO_Tick();

			if (ticks > 0)
			{
				ticks--;
			}

		}

        if (CurrentCPURunMode == STOP)
        {
            // OPCOM MODE ?
            printf("CPU: WAS STOPPED, SHUTTING DOWN\r\n");
            CurrentCPURunMode = SHUTDOWN;
        }
	}	
}


void cpu_init()
{
	/* initialize an empty register set */
	gReg = calloc(1, sizeof(struct CpuRegs));

	/* Initialize volatile memory to zero */
	memset(&VolatileMemory, 0, sizeof(VolatileMemory));

	// setbit(_STS, _O, 1);
	setbit_STS_MSB(_N100, 1);
	gCSR = 1 << 2; /* this bit sets the cache as not available */

	/* Set cpu as running for now. Probably should depend on settings */
	CurrentCPURunMode = RUN;
	instr_counter = 0;

	// Allocate ShadowMemory for pagetables
	CreatePagingTables();

	/* OK lets set up the parsing for our current cpu before we start it. */
	Setup_Instructions();

	gALD = 01560; // oct 1560 (ALD position 4, Binary load from 1560) // Floppy

	if (DISASM)
		disasm_setlbl(gPC);
}

void cleanup_cpu()
{
	// Destroy paging tables
	DestroyPagingTables();
}

