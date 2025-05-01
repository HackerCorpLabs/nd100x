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

//#define DEBUG_TRAP
//#define DEBUG_PK_SWITCH
//  #define DEBUG_IONOFF


#include "cpu_types.h"
#include "cpu_protos.h"


// Initialize the instruction function array
InstrFunc instr_funcs[65536];

/*********** TODO ***********/

extern ushort io_op(ushort ioadd, ushort regA);
extern int IO_Ident(uint16_t level);

/************************************ HELPER FUNCTIONS *************************************/

/// <summary>
/// Check if we are allowed to run a privileged instruction
///
/// Privileged intructions are only available to programs running in system mode (rings 2 and 3) or when memory protection is disabled;
/// </summary>
/// <returns>TRUE if allowed to execute</returns>
bool CheckPriv()
{
	if (!STS_PONI)
		return true; // memory protection disabled

	// Check ring
	ushort pcr = gReg->reg_PCR[CurrLEVEL];
	ushort ring = pcr & 0x03;

	if ((ring == 2) || (ring == 3))
		return true;

	// Failed, not allowed to execute
	// Generate a privileged instruction interrupt
	interrupt(14, 1 << 6); // Privileged instruction
	return false;
}


short signExtend(ushort x)
{
	short res = (ushort)x;

	// If negative (bit 7==1), extend high 8 bits with 1's
	if ((x & 1 << 7) != 0)
		res |= 0xFF00;

	return res;
}


ushort do_add(ushort a, ushort b, ushort k)
{
	int tmp;
	bool is_diff;
	tmp = ((int)a) + ((int)b) + ((int)k);
	/* C (carry) */
	if (tmp & 0xffff0000)
		setbit(_STS, _C, 1);
	else
		setbit(_STS, _C, 0);
	/* O(static overflow), Q (dynamic overflow) */
	is_diff = (((1 << 15) & a) ^ ((1 << 15) & b)); /* is bit 15 of the two operands different? */
	if (!(is_diff) && (((1 << 15) & a) ^ ((1 << 15) & tmp)))
	{						 /* if equal and result is different... */
		setbit(_STS, _O, 1); // Static overflow
		setbit(_STS, _Q, 1); // Dynamic overflow (Instruction test shows Q must be set)
	}
	else
	{
		setbit(_STS, _Q, 0);
		//setbit(_STS, _O, 0); NO!
	}
	return (ushort)tmp;
}


// Calculate effective address for LDnTX
unsigned int calcEL(uint8_t displacement)
{

	unsigned int EL = (gX + displacement) & 0xFFFF;
	EL = (gT & 0xFF) << 16 | EL;
	EL = EL & 0xFFFFFF; // Cap at 24 bits

	// printf("calcEL: EL=%6X, gX=%4X, gT=%4X, displacement=%d\n", EL,gX, gT, displacement	);
	return EL;
}

// read el value from memory
unsigned int ReadEL(unsigned el)
{
	return ReadPhysicalMemory(el, true);
}

// write el to memory
void WriteEL(uint el, ushort value)
{
	WritePhysicalMemory(el, value, true);
}



/***************** HELPER INSTRUCTIONS *****************/

void illegal_instr(ushort operand)
{
	// printf("Illegal instruction %6o  at %6o\r\n", operand, gPC);

	interrupt(14, 1 << 4); /* Illegal Instruction <= WILL TRAP! */
}

void unimplemented_instr(ushort operand)
{
	printf("\r\n");
	printf("--------------------------------\r\n");
	printf("CPU: Unimplemented instruction: %06o at PC: %06o\r\n", operand, gPC);
	printf("--------------------------------\r\n");
	printf("\r\n");

	//set_cpu_run_mode(CPU_STOPPED); /* OK unimplemented function, lets stop CPU and end program that way */
}


/************************************ INSTRUCTIONS *************************************/



/* AAA
 */
void ndfunc_aaa(ushort operand)
{
	short temp;

	temp = signExtend(operand & 0xFF);
	gA = do_add(gA, temp, 0);
}

/* AAB
 */
void ndfunc_aab(ushort operand)
{
	ushort temp;

	temp = signExtend(operand & 0xFF);
	gB = do_add(gB, temp, 0);
}

/* AAT
 */
void ndfunc_aat(ushort operand)
{
	ushort temp;

	temp = signExtend(operand & 0xFF);
	gT = do_add(gT, temp, 0);
}

/* AAX
 */
void ndfunc_aax(ushort operand)
{
	ushort temp;

	temp = signExtend(operand & 0xFF);

	gX = do_add(gX, temp, 0);
}

/* MON
 */
void ndfunc_mon(ushort operand)
{
	uint16_t monitor_number = (operand & 0x1ff);

	// TODO:MAYBE, add emulation layer here
	if (false)
	{
		// identfy montitor call and check if it should be intercepted!
		//mon(monitor_number);
	}
	else
	{
		if (CurrLEVEL < 14)
		{
			if ((monitor_number & (1 << 8)) != 0)
			{
				monitor_number |= 0xFE00; // Sign extend
			}

			gReg->reg[14][_T] = monitor_number;
			interrupt(14, 1 << 1); /* Monitor Call */
			gCHKIT = true;	
		}
	}
}

/* SAA
 */
void ndfunc_saa(ushort operand)
{
	setreg(_A, signExtend(operand & 0xFF));
}

/* SAB
 */
void ndfunc_sab(ushort operand)
{
	setreg(_B, signExtend(operand & 0xFF));
}

/* SAT
 */
void ndfunc_sat(ushort operand)
{
	setreg(_T, signExtend(operand & 0xFF));
}

/* SAX
 */
void ndfunc_sax(ushort operand)
{
	setreg(_X, signExtend(operand & 0xFF));
}

/* SHT, SHD, SHA, SAD
 */
void ndfunc_shifts(ushort operand)
{
	ulong double_reg;

	switch ((operand >> 7) & 0x03)
	{
	case 0: /* SHT */
		gT = ShiftReg(gT, operand);
		break;
	case 1: /* SHD */
		gD = ShiftReg(gD, operand);
		break;
	case 2: /* SHA */
		gA = ShiftReg(gA, operand);
		break;
	case 3: /* SAD */
		double_reg = ShiftDoubleReg(((ulong)gA << 16) | gD, operand);
		gA = double_reg >> 16;
		gD = double_reg & 0xFFFF;
		break;
	default: /* can never reach here but... */
		break;
	}
}

/* NLZ
 */
void ndfunc_nlz(ushort operand)
{
	DoNLZ(operand & 0xFF);
}

/* DNZ
 */
void ndfunc_dnz(ushort operand)
{
	DoDNZ(operand & 0xFF);
}

/* SRB (Privileged)
 */
void ndfunc_srb(ushort operand)
{
	if (!CheckPriv())
		return;

	/* SRB */ /* NOTE: These two seems to have bit req on 0-2 as well */
	DoSRB(operand);
}

/* LRB (Privileged)
 */
void ndfunc_lrb(ushort operand)
{
	if (!CheckPriv())
		return;

	/* SRB */ /* NOTE: These two seems to have bit req on 0-2 as well */
	DoLRB(operand);
}

/// <summary>
/// CJP - Conditional jump
/// Instruction bits 8-10 are used to specify one of 8 jump conditions.
///
/// If the specified condition becomes true, the displacement is added to the program counter and a jump relative to current location takes place.
/// The range is 128 locations backwards and 127 locations forwards. If the specified condition is false, no jump takes place.
///
/// Execution time depends on conditions, but is the same for all instructions.
///
/// A conditional jump instruction must be specified by means of the 8 mnemonics listed below.
/// It is illegal to specify CJP or any combinations of, B, | and , X.
/// </summary>
void CJP(bool jmp_flag, ushort operand)
{
	if (jmp_flag)
	{
		ushort old_gPC = gPC - 1;

		ushort temp = signExtend(operand & 0xff);
		gPC = do_add(gPC - 1, temp, 0);

		if (DISASM)
			disasm_userel(old_gPC, gPC);
	}
}

/// <summary>
/// JAP - Jump if A register is positive or zero, A bit 15 = 0.
/// Code: 130 000
/// Format: JAP <disp.>
///
/// Affected: (P)
/// </summary>
void ndfunc_jap(ushort operand)
{
	bool flag = ((1 << 15) & gA) == 0;
	CJP(flag, operand);
}

/// <summary>
/// JAN - Jump if A register is negative, A bit 15 = 1.
/// Code: 130 400
/// Format: JAN <disp.>
///
/// Affected: (P)
/// </summary>
void ndfunc_jan(ushort operand)
{
	bool flag = ((1 << 15) & gA) != 0;
	CJP(flag, operand);
}

/// <summary>
/// JAZ - Jump if A register is zero.
/// Code: 131 000
/// Format: JAZ<disp>
///
/// Affected: (P)
/// </summary>
void ndfunc_jaz(ushort operand)
{
	bool carry = (gA == 0);
	setbit(_STS, _C, carry);

	CJP(gA == 0, operand);
}

/// <summary>
/// JAF - Jump if A register is filled (not zero)
/// Code: 131 400
/// Format: JAF<disp. >
///
/// Affected: (P)
/// </summary>
void ndfunc_jaf(ushort operand)
{
	CJP(gA != 0, operand);
}

/// <summary>
/// JPC - Count and jump if X register is positive or zero.
/// Code: 132000
/// Format: JPC<disp. >
///
/// X is incremented by one, and if the X bit 15 equals zero after the incrementation, the jump takes place.
/// Affected: (P) and (X)
/// </summary>
void ndfunc_jpc(ushort operand)
{
	gX++;

	CJP(((1 << 15) & gX) == 0, operand);
}

/// <summary>
/// JNC - Count and jump if X register is negative.
/// Code: 132 400
/// Format: JNC<disp.>
/// X is incremented by one; if then the X bit 15 equals one, the jump takes place.
///
/// Affected: (P) and(X)
/// </summary>
void ndfunc_jnc(ushort operand)
{
	gX++;
	CJP((gX & (1 << 15)) != 0, operand);
}

/// <summary>
/// JXN - Jump if X register is negative. X bit 15 = 1.
/// Code: 133 400
/// Format: JXN <disp. >
///
/// Affected: (P)
/// </summary>
void ndfunc_jxn(ushort operand)
{
	CJP((gX & (1 << 15)) != 0, operand);
}

/// <summary>
/// JXZ - Jump if X register is zero.
/// Code: 133 000
/// Format: JXZ <disp. >_
///
/// Affected: (P)
/// </summary>
void ndfunc_jxz(ushort operand)
{
	CJP(gX == 0, operand);
}

/* JPL
 */
void ndfunc_jpl(ushort operand)
{
	ushort old_gPC = gPC - 1;

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	gL = gPC;
	gPC = gEA;

	
	if (DISASM)
		disasm_userel(old_gPC, gPC);
}

/* SKP
 * Skip instructions, this one interleaves with other instructions so might need some extra checkings.
 */
void ndfunc_skp(ushort operand)
{
	if (IsSkip(operand))
		gPC++;
}

/// <summary>
/// BFILL - Byte Fill
/// Code: 140 130
/// Format: BFILL
///
/// This instruction has only one operand. The destination operand is specified in the X, and T registers.
/// The right-most byte in the A-reg. (bits 0-7) is filled into the destination field.
///
/// After execution, the X-register and T-register bit 15 point to the end of the field(after the last byte).
/// The T-register bits(0-11) equal zero.
///
/// The instruction will always have a skip return (no error condition)
/// </summary>
void ndfunc_bfill_new(ushort operand)
{
	bool useAPT = false;
	WriteMode wm;

	// Check if we should use alternative page table, bit 14 in T register
	if ((gT & (1 << 14)) != 0)
		useAPT = true;

	while ((gT & 0xfff) != 0)
	{
		// Bit 15:  0=>MSB, 1=> LSB
		wm = (gT & (1 << 15)) ? WRITEMODE_LSB : WRITEMODE_MSB;
		WriteVirtualMemory(gX, gA & 0xFF, useAPT, wm);

		gT--;

		gT ^= (1 << 15); // Flip T bit 15
		if ((gT & (1 << 15)) == 0)
			gX++;
	}

	gPC++; // Skip return
}

void ndfunc_bfill(ushort operand)
{
	ushort d1, d2, len, addr, i;
	ushort right = (gT & ((ushort)1 << 15)) ? 1 : 0;	   /* Start with right byte? (LSB) */
	bool is_apt = (gT & ((ushort)1 << 14)) ? true : false; /* Use APT or not? */
	ushort thebyte = gA & 0xff;
	len = gT & 0x0fff; /* Number of bytes to do */
	addr = gX;		   /* just in case we do 0 bytes */
	d1 = gX;
	d2 = gT;
	
	for (i = 0; i < len; i++)
	{
		addr = d1 + ((i + right) >> 1); /* Word adress of byte to write */
		MemoryWrite(thebyte, addr, is_apt, ((i + right) & 1));
	}
	gT &= 0x7000;				   /* Null number of bytes, as per manual, also null bit 15 */
	gT |= ((i + right) & 1) << 15; /* set bit 15 to point to next free byte */
	gX = d1 + ((i + right) >> 1);
	

	gPC++; /* This function has a SKIP return on no error, which is always? */
}

/* STZ
 */
void ndfunc_stz(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	MemoryWrite(0, gEA, gUseAPT, 2);
}

/* STA
 */
void ndfunc_sta(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	MemoryWrite(gA, gEA, gUseAPT, 2);
}

/* STT
 */
void ndfunc_stt(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	MemoryWrite(gT, gEA, gUseAPT, 2);
}

/* STX
 */
void ndfunc_stx(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	MemoryWrite(gX, gEA, gUseAPT, 2);
}

/* STD
 */
void ndfunc_std(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	MemoryWrite(gA, gEA + 0, gUseAPT, 2);
	MemoryWrite(gD, gEA + 1, gUseAPT, 2);
}

/* STF
 */
void ndfunc_stf(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	MemoryWrite(gT, gEA + 0, gUseAPT, 2);
	MemoryWrite(gA, gEA + 1, gUseAPT, 2);
	MemoryWrite(gD, gEA + 2, gUseAPT, 2);
}

/* LDA
 */
void ndfunc_lda(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gA = MemoryRead(gEA, gUseAPT);
}

/* LDT
 */
void ndfunc_ldt(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gT = MemoryRead(gEA, gUseAPT);
}

/* LDX
 */
void ndfunc_ldx(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gX = MemoryRead(gEA, gUseAPT);
}

/* LDD
 */
void ndfunc_ldd(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	gA = MemoryRead(gEA + 0, gUseAPT);
	gD = MemoryRead(gEA + 1, gUseAPT);
}

/* LDF
 */
void ndfunc_ldf(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	gT = MemoryRead(gEA + 0, gUseAPT);
	gA = MemoryRead(gEA + 1, gUseAPT);
	gD = MemoryRead(gEA + 2, gUseAPT);
}




/* STZTX
 */
void ndfunc_stztx(ushort operand)
{
	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	uint EL = calcEL(displacement);
	WriteEL(EL, 0);

}

/* STATX
 */
void ndfunc_statx(ushort operand)
{
	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	uint EL = calcEL(displacement);
	WriteEL(EL, gA);

}

/* STDTX
 */
void ndfunc_stdtx(ushort operand)
{
	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	uint EL = calcEL(displacement);
	WriteEL(EL, gA);
	WriteEL(EL + 1, gD);

}

/// <summary>
/// Load A register
///
/// Code: 143 3n0
/// Format: LDATX
///
/// Load the contents of the physical memory location pointed to
/// by the effective address into the A register.
/// A := (EL)
///
/// Affected: (A)
/// </summary>
void ndfunc_ldatx(ushort operand)
{
	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;

	unsigned int EL = calcEL(displacement);
	gA = ReadEL(EL);
}

/// <summary>
/// Load X register
/// Code: 143 3n1
/// Format: LDXTX
///
/// Load the contents of the physical memory location pointed to
/// by the effective address into the X  register.
/// X := (EL)
///
/// Affected: (X)
/// </summary>
void ndfunc_ldxtx(ushort operand)
{
	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	unsigned int EL = calcEL(displacement);

	gX = ReadEL(EL);
}

/// <summary>
/// Load Double Word
/// Code: 143 3n2
/// Format: LDDTX
///
/// Load the contents of the physical memory location pointed to by the effective address
/// into the A register and the contents of the effective address plus one  into the D register
/// A := (EL), D := (EL + 1) .
///
/// Affected: (A,D)
/// </summary>
void ndfunc_lddtx(ushort operand)
{

	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	unsigned int EL = calcEL(displacement);

	gA = ReadEL(EL);
	EL++;
	gD = ReadEL(EL);
}

/// <summary>
/// Load B register
///
/// Code: 143 3n3
/// Format: LDBTX
///
/// Load the contents of the physical memory location pointed to by the twice the
/// effective address contents into the B register, then OR the value with 177 000
/// B := 177000 V ((EL) + (EL)) (V = inclusive OR)
///
/// Affected: (B)
/// </summary>
void ndfunc_ldbtx(ushort operand)
{
	ushort temp;
	unsigned int result;

	if (!CheckPriv())
		return;

	uint8_t displacement = (operand >> 3) & 0x07;
	unsigned int EL = calcEL(displacement);

	temp = ReadEL(EL);
	result = (temp + temp) & 0xFFFF;
	gB = result | 0xFE00; // 0177000
}

/// <summary>
/// MIN - Increment memory and skip if zero
/// Code: 040 000
///
/// Format: MIN <address mode> <disp.>
///
/// Effective word is read and incremented by one and then stored in the effective location.If the result becomes zero, the next instruction is skipped.
///
/// Affected: (EL), (P)
/// </summary>
void ndfunc_min(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	ushort temp = MemoryRead(gEA, gUseAPT);
	temp++;
	MemoryWrite(temp, gEA, gUseAPT, 2);

	if (temp == 0)
		gPC++; // Next instruction is skipped
}

/* ADD
 */
void ndfunc_add(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	ushort eff_word = MemoryRead(gEA, gUseAPT);
	gA = do_add(gA, eff_word, 0);
}

/* SUB
 */
void ndfunc_sub(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	ushort eff_word = MemoryRead(gEA, gUseAPT);
	gA = do_add(gA, ~eff_word, 1);
}

/* AND
 */
void ndfunc_and(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gA = gA & MemoryRead(gEA, gUseAPT);
}

/* ORA
 */
void ndfunc_ora(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gA = gA | MemoryRead(gEA, gUseAPT);
}

/* FAD
 */
void ndfunc_fad(ushort operand)
{
	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	ushort a[3], b[3], r[3];
	int res;

	a[0] = gT;
	a[1] = gA;
	a[2] = gD;
	b[0] = MemoryRead(gEA + 0, gUseAPT);
	b[1] = MemoryRead(gEA + 1, gUseAPT);
	b[2] = MemoryRead(gEA + 2, gUseAPT);
	res = NDFloat_Add(a, b, r);
	gT = r[0];
	gA = r[1];
	gD = r[2];
	if (res == -1)
		setbit(_STS, _TG, 1);
}

/* FSB
 */
void ndfunc_fsb(ushort operand)
{
	ushort a[3], b[3], r[3];

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	b[0] = gT;
	a[0] = gT;
	a[1] = gA;
	a[2] = gD;
	b[0] = MemoryRead(gEA + 0, gUseAPT);
	b[1] = MemoryRead(gEA + 1, gUseAPT);
	b[2] = MemoryRead(gEA + 2, gUseAPT);
	NDFloat_Sub(a, b, r);
	gT = r[0];
	gA = r[1];
	gD = r[2];
}

/* FMU
 */
void ndfunc_fmu(ushort operand)
{
	ushort a[3], b[3], r[3];

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	a[0] = gT;
	a[1] = gA;
	a[2] = gD;
	b[0] = MemoryRead(gEA + 0, gUseAPT);
	b[1] = MemoryRead(gEA + 1, gUseAPT);
	b[2] = MemoryRead(gEA + 2, gUseAPT);
	NDFloat_Mul(a, b, r);
	gT = r[0];
	gA = r[1];
	gD = r[2];
}

/* FDV
 */
void ndfunc_fdv(ushort operand)
{
	ushort a[3], b[3], r[3];

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);

	a[0] = gT;
	a[1] = gA;
	a[2] = gD;
	b[0] = MemoryRead(gEA + 0, gUseAPT);
	b[1] = MemoryRead(gEA + 1, gUseAPT);
	b[2] = MemoryRead(gEA + 2, gUseAPT);
	NDFloat_Div(a, b, r);

	// printf("FDV: %06o %06o %06o %06o %06o %06o ==> %06o %06o %06o\n", a[0], a[1], a[2], b[0], b[1], b[2], r[0], r[1], r[2]);
	// This fails: FDV: 042000 100000 000000 040001 140000 000000 ==> 042000 100007 000000

	gT = r[0];
	gA = r[1];
	gD = r[2];
}

/* JMP
 */
void ndfunc_jmp(ushort operand)
{
	ushort old_gPC = gPC - 1;

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	gPC = gEA;

	if (DISASM)
		disasm_userel(old_gPC, gPC);
}

/* GECO
 */
void ndfunc_geco(ushort operand)
{
	/*
		* Microcode listing lists this instruction from micro address 004000. Page 99 in the PDF document "MICROPROGRAMLISTNING FOR ND-110_32 BIT VERSION K-Gandalf-OCR"
		* Page 134 listes the GECO offset address as 7427, assuming it is means opcode 1_427_nnn

		* https://www.ndwiki.org/wiki/GECO

		GECO is a customer-specifed instruction which appears to be included as part of the standard instruction set from ND-100/CE and later.
		The name comes from the customer, GECO (Geophysical Company of Norway).

		SINTRAN III version L, and probably version K and possibly earlier, tests for GECO as part of the startup.
		From this it looks like the registers B, D, A, and X are all used as input parameters. When all are set to 0 the instruction seems to do nothing.
	*/
}

/* VERSN - ND110+
 * IN: uses A reg bit 11-8 as a bitfield to addess the byte of the version number read, the total is 16 bytes
 * so instruction has to be called 16 times, with incremented A each time.
 * OUT: Sets A, T, D
 */
/* VERSN instruction constants */
static unsigned char installation_number[] = {0x01, 0x04, 0x00, 0x01, 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}; // 040171 = CPU?
static int microcode_version = 0x0708;																									 // During SINTRAN boot will load new microcode, but it must be minimum 013. Read from "control store" address 0100 for new microcode to load
static int print_version = 0x80C;

void ndfunc_versn(ushort operand)
{
	int offset = (gA >> 8) & 0x0F;

	// Set D register to the installation number byte at the specified offset
	gD = installation_number[offset];

	// Set A register with print version in upper 12 bits and preserve ALD in lower 4 bits
	gA = (print_version << 4) | (gALD & 0x0F);

	// Set T register with microcode version
	gT = microcode_version;
}


/************ IO INSTRUCTIONS *************/



/// <summary>
/// Check if the IO address points to special "in memory" registers
///
/// Addresses from 100000 . - 100777, are used to specify system control registers which have to be accessed via the ND-100 bus.
/// An example is the Error Correction Control Register (ECCR), physically located on the memory modules.
/// </summary>
/// <returns>true if the IO address was handled, false otherwise</returns>
bool UpdateMemoryIO()
{
	if ((gT < 0x8000) || (gT > 0x81FF))
		return false;

	switch (gT)
	{
	case 0x804D: // 100115
		// By disabling this register ECCR test will say that there is no ECCR memory. Which is a benefit, then it can't fail :)
		// Test #5 in "MEMORY - Version: D00 - 1986-10-30" fails, because it expects and interrupt - but at the moment I dont know why..
		if (gECCR != gA)
		{
			gECCR = gA;			
		}
		return true;
	default:
		break;
	}
	return false;
}

/* IOT
 * This is really an ND1 instruction
 * NOTE:: Privileged instructions
 * Format: IOT number
 * Code: 160 nnn. Opcode 5 bits, 11 bits for IO address
 * (0160000 - 0163777)
 * 
 */
void ndfunc_iot(ushort operand)
{
	// ND110 Microcode:
	// IOT - INSTRUCTION IS PRIVILEGED WHEN RING = 0 OR 1
	//                  AND ILLEGAL    WHEN RING = 2 OR 3
	if (!CheckPriv())
		return;

	/* for now handle it as illegal instruction */
	illegal_instr(operand);
}

/* IOX (Privileged)
 */
void ndfunc_iox(ushort operand)
{
	if (!CheckPriv())
		return;

	if (!UpdateMemoryIO())
		gA = io_op(operand & 0x07ff, gA);

}

/* IOXT (Privileged)
 */
void ndfunc_ioxt(ushort operand)
{
	if (!CheckPriv())
		return;


	if (!UpdateMemoryIO())
		gA = io_op(gT, gA);

}



/* IDENT
 *
 * NOTE: Privileged instruction
 */
void ndfunc_ident(ushort operand)
{
	if (!CheckPriv())
		return;

	switch ((operand & 0x003f))
	{
	case 004:
		DoIDENT(10);
		break;
	case 011:
		DoIDENT(11);
		break;
	case 022:
		DoIDENT(12);
		break;
	case 043:
		DoIDENT(13);
		break;
	default:
		illegal_instr(operand); /* Assume this is how we should hanle it.. TODO: Check!!! */
	}
}

/********************SYSTEM FUNCTIONS  /*******************/


/* OPCOM (Privileged)
 */
void ndfunc_opcom(ushort operand)
{
	if (!CheckPriv())
		return;

	set_cpu_run_mode(CPU_STOPPED);
}

/// <summary>
/// IRW - Inter-Register Write
///
/// Note: This instruction results in a no-operation if the A register of the current program level is used
/// </summary>
void ndfunc_irw(ushort operand)
{
	if (!CheckPriv())
		return;

	ushort level = (operand >> 3) & 0x0F;
	ushort dr = (operand & 0x07);

	if ((level == CurrLEVEL) && (dr == _A))
		return; // A on same level, do nothing (Write from A to A on same level== NOP)

	if ((level == CurrLEVEL) && (dr == _P))
		return; // P on same level, do nothing (Because this is what the microcode does)

	if (dr == _STS)
	{
		// Update STS lower bits (which is unique for each runlevel)
		gReg->reg[level][_STS] = (gA & 0x00FF);
	}
	else
	{
		gReg->reg[level][dr] = gA;
	}
}

/// <summary>
/// IRR - Inter-Register Read
/// Code 0153600
///
/// This instruction is used to read into the A register on current program level one of the general registers inside/outside the current program level.
/// If bits 0-2 are zero, the status registers on the specified program level will be read into the A register bits 0-7, with bits 8-15 cleared.
/// The IRR instruction is privileged.
/// </summary>
void ndfunc_irr(ushort operand)
{
	if (!CheckPriv())
		return;

	ushort level = (operand >> 3) & 0x0F;
	ushort sr = (operand & 0x07);

	if (sr == 0) // STS
	{
		gA = gReg->reg[level][_STS] & 0xFF; // read only lower 8 bits
	}
	else
	{
		gA = gReg->reg[level][sr];
	}
}

/* EXAM (Privileged)
 */
void ndfunc_exam(ushort operand)
{
	if (!CheckPriv())
		return;

	// int fulladdress = (((unsigned int)gA) << 16) | (ushort)gD;
	unsigned int fulladdress = ((gA & 0xFF) << 16) | gD;
	gT = ReadPhysicalMemory(fulladdress, true);
}

/* DEPO (Privileged)
 */

void ndfunc_depo(ushort operand)
{
	if (!CheckPriv())
		return;

	unsigned int fulladdress = ((gA & 0xFF) << 16) | gD;
	WritePhysicalMemory(fulladdress, gT, true);
}

/* POF (Privileged)
 */
void ndfunc_pof(ushort operand)
{
	
	if (!CheckPriv())
		return;
	setbit_STS_MSB(_PONI, 0);
}

/* PIOF (Privileged)
 */
void ndfunc_piof(ushort operand)
{
	
	if (!CheckPriv())
		return;

	setbit_STS_MSB(_IONI, 0);	
	setbit_STS_MSB(_PONI, 0);
}

/* PON
 */
void ndfunc_pon(ushort operand)
{
	setbit_STS_MSB(_PONI, 1);
}

/* PION
 */
void ndfunc_pion(ushort operand)
{
	setbit_STS_MSB(_IONI, 1);
	setbit_STS_MSB(_PONI, 1);
	gCHKIT = true; // recalc PK
}

/// <summary>
/// IOF - Turn off interrupt system
/// </summary>
void ndfunc_iof(ushort operand)
{
	if (!CheckPriv())
		return;

	setbit_STS_MSB(_IONI, 0);
}

/// <summary>
/// ION
///
/// Turn on interrupt system
/// </summary>
void ndfunc_ion(ushort operand)
{	
	setbit_STS_MSB(_IONI, 1);
	gCHKIT = true; // recalc PK
}


/* REX (Privileged)
 */
void ndfunc_rex(ushort operand)
{
	if (!CheckPriv())
		return;

	setbit_STS_MSB(_SEXI, 0);
}

/* SEX (Privileged)
 */
void ndfunc_sex(ushort operand)
{
	if (!CheckPriv())
		return;

	setbit_STS_MSB(_SEXI, 1);
}

/******************** CX FUNCTIONS  /*******************/



/* SETPT - ND110+
 *
 * NOTE: Privileged instruction
 */
void ndfunc_setpt(ushort operand)
{
	if (!CheckPriv())
		return;

	/* ND110 Microcode:
	9217  004054  %        OPCODE 140300 : SETPT 4
	9218  004054  %
	9219  004054  % SETPT: JXZ * 10               % FINISHED
	9220  004054  %        LDDTX 20
	9221  004054  %        BSET ZRO 130 DA        % PGU-BIT
	9222  004054  %        LDBTX 10
	9223  004054  %        177777                 % OLD BUG IN LDBTX
	9224  004054  %        STD ,B                 % ALWAYS INSIDE PAGE TABLE
	9225  004054  %        LDXTX 00
	9226  004054  %        JMP *—7
	*/

	int cnt = 0;

	// JXZ * 10 % FINISHED
	while (gX != 0)
	{
		uint EL = 0;
		uint EffectiveAddress = 0;

		//  LDDTX 20 <=  A: = (EL), D: = (EL + 1)
		EL = calcEL(2); // Calculates using X, T and mriDisplacement // oct 020 >>3
		gA = (ushort)ReadEL(EL);
		gD = (ushort)ReadEL(EL + 1);

		// BSET ZRO 130 DA % PGU - BIT *

		gA = gA & ~(1 << 0x0b); // 0x0b = 13 octalt. Clear bit 013 in register A

		// LDBTX 10
		EL = calcEL(1); // oct 10 >> 3
		uint elval = ReadEL(EL);
		gB = (ushort)(((elval + elval) & 0xFFFF) | 0xFE00); // 177000

		// 177777					% OLD BUG IN LDBTX

		// STD ,B
		EffectiveAddress = (uint)(gB & 0xFFFF); // (+displacement, which is 0 here)
		WriteVirtualMemory(EffectiveAddress, gA, true, WRITEMODE_WORD);
		WriteVirtualMemory(EffectiveAddress + 1, gD, true, WRITEMODE_WORD);

		//  LDXTX 00 <=  X:= (EL)
		gX = (ushort)ReadEL(calcEL(0)); // Calculates using X, T and mriDisplacement

		// Increase counter
		cnt++;
	}

	gX = (ushort)cnt; // Report number of loops in X (undocumented, but testing using "INSTRUCTION - Version: C00 - 1986-10-30" sub-program "SEGMENTS" identified it.
}

// **************************************************************************************
// ****  ND100 and ND110CX only - segment instructions
// **************************************************************************************

// PDF Page 101 (page number 99) in "MICROPROGRAMLISTNING FOR ND-110_32 BIT VERSION K-Gandalf-OCR.pdf"
/// SINTRAN III CONTROL INSTRUCTIONS
/// ALL ARE PRIVILEGED

/// <summary>
/// Clear Page Tables
/// Code: 140 301
/// Format: CLEPT
///
/// Affected: (?)
/// </summary>
void ndfunc_clept(ushort operand)
{
	if (!CheckPriv())
		return;

	/* ND110 Microcode:
	9229  004054  %        OPCODE 140301 I CLEPT
	9230  004054  %9231  004054  % CLEPT: JXZ * 11               % FINISHED
	9232  004054  %        LDBTX 10
	9233  004054  %        177777                 % OLD BUG IN LDBTX
	9234  004054  %        LDA ,B
	9235  004054  %        JAZ * 3
	9236  004054  %        STATX 20
	9237  004054  %        STZ ,B                 % ALWAYS INSIDE PAGE TABLE
	9238  004054  %        LDXTX 00
	9239  004054  %        JMP *-10
	9240  004054  %*
	*/

	/*

	 *  Affected: Pagetables, A, T, X, B registers ????
	 *  T,X used as an adress reg  with 24 bits in the xxxTX instructions
	 *
	 * This instruction apparently is a replacement for this sequence:
	 * CLEPT:	JXZ * 10	(if X=0 goto END)
	 *		LDBTX 10	(B:=177000|(2*(EL)), EL=T,X+1)
	 *		LDA ,B		(A:=(B))
	 *		JAZ * 3		(if A=0 goto LOOP)
	 *		STATX 20	((EL):=A, EL=T,X+2)
	 *		STZ ,B		( (B):=0 )
	 *		LDXTX 00	(X:=(EL), EL=T,X)
	 * LOOP:	JMP *-7		(goto CLEPT)
	 * END:		...
	 */
	ushort cnt;

	while (gX)
	{
		uint EL = 0;

		/* LDBTX 10 */
		EL = calcEL(1);
		uint elval = ReadEL(EL);
		gB = (ushort)(((elval + elval) & 0xFFFF) | 0xFE00); // 177000

		// LDA, B
		gA = (ushort)ReadVirtualMemory(gB, true);

		// JAZ *3 (jump 3 instruction if A is zero)
		if (gA != 0)
		{
			// STATX 20  =>  (EL) = A
			EL = calcEL(2); // Calculates using X, T and mriDisplacement	//020 OCT  >>3
			WriteEL(EL, (ushort)gA);

			// STZ, B
			WriteVirtualMemory(gB, 0, true, WRITEMODE_WORD);
		}

		//  LDXTX 00 <=  X:= (EL)
		EL = calcEL(0); // Calculates using X, T and mriDisplacement
		gX = (ushort)ReadEL(EL);

		// Increase counter
		cnt++;
	}

	gX = cnt;
}

/// <summary>
/// Clear non re-entrant pages
/// Code: 140 302
/// Format: CLNREENT
///
/// Segment function
///
///
/// The contents of the memory address at A+2 are read to find the page table to be cleared along with the SINTRAN RT bitmap (addressed by the X and T registers).
/// The page table entries corresponding to those bits set in the RT bitmap are then cleared.
///
/// Affected: (?)
/// </summary>
void ndfunc_clnreent(ushort operand)
{
	if (!CheckPriv())
		return;
	// TODO: Implement

	/*
	OPCODE 140302 : CLNREENT

	READ ADDRESS A+2 TO FIND PAGE TABLE TO BE AFFECTED
	READ RT - DESCRIPTION BITMAP WORDS, FOUND FROM ADDRESS X + 25.
	CLEAR PAGE-TABLE ENTRIES CORRESPONDING TO 1 - BITS IN BITMAP.
	THE LAST BITMAP—ADDRESS IS IN ADDRESS X + T.
	*/
}

/// <summary>
/// Change Page Tables
/// Code 140 303
/// Format: CHREENTPAGES
///
/// Segment function
///
/// The X  register is used to address the current (R1) and previous(Rp)  scratch registers.
/// If the R1 is zero, the re-entrant page has nothing to change so the loop is left, otherwise the contents of the memory location pointed to by the R1+2 are loaded into T.
///
/// T then contains the protect table entry, if the page has not been written to (WIP bit 12 is zero )
/// T and R1 are loaded with Rp.
/// R1 (now containing Rp) is tested again for zero.
/// If the page has been written to, the T register is loaded with the contents of the second scratch register(R2) pointed to by R1,
/// and R2 becomes the address of Rp. X is loaded with R1 as the new pointer to the re­entrant pages and Rp is loaded into the D register pointed to by A.
///
/// Affected: (?)
/// </summary>
void ndfunc_chreent_pages(ushort operand)
{
	if (!CheckPriv())
		return;
	// TODO: Implement

	/*
		OPCODE 140303 : CHREENTPAGES

		1. READ ADDRESS D.X -> R1 ; D,X -> PREVIOUS (SCRATCH REG)
		2. IF R1 = 0; SKIP RETURN (FINISHED)
		3. READ ADDRESS T,R1+2
		4. IF NOT WIP; T.R1 —> PREVIOUS; READ ADDR T.R1 -> R1; GOTO 2
		5. READ ADDRESS T,R1  —> R2
		6. WRITE R2 -> ADDRESS PREVIOUS
		7. R1 -> X ; PREVIOUS -> D.A ; RETURN
	*/
}

/// <summary>
/// Clear page tables and collect PGU information.
/// Code: 140 304
/// Format: CLEPU
///
/// Segment function
///
/// Affected: (?)
/// </summary>
void ndfunc_clepu(ushort operand)
{
	if (!CheckPriv())
		return;

	// TODO: Implement

	/*
		OPCODE 140304 : CLEPU

		AS 'CLEPT" BUT INCLUDING WORKING SET INFORMATION
		FOR ALL PAGE-TABLE ENTRIES HANDLED
		IF PGU OF ENTRY IS 1
			D /ø 300
			B /ø 776 SHR 1 - D
			B-REG BITS 0—3 IS NOW BIT NUMBER
			B-REG BITS 4-6 IS NOW WORD NUMBER
			SET BIT IN 8—WORD TABLE IN PAGE-MAP BANK
			POINTED TO BY L-REGISTER

		LAYOUT 0F 8-WORD TABLE

							BIT 15									BIT O
							________________________________________________
		L-REG -> WORD	0	# PAGE 17								PAGE 0 #
		WORD			1	# PAGE 37									20 #
		WORD			2	# PAGE 57									40 #
		WORD			3	# PAGE 177								   160 #

	*/
}



/********************* STACK INSTRUCTIONS *********************/



/* INIT
 * INIT instruction:
 * IN: nothing. uses PC.
 * ADDR  : INIT
 * ADDR+1: Stack demand
 * ADDR+2: Address of stack start
 * ADDR+3: Maximum stack size
 * ADDR+4: Flag
 * ADDR+5: Not used
 * ADDR+6: Error return
 * ADDR+7: Normal return
 *
 */
void ndfunc_init(ushort operand)
{
	ushort demand, start, maxsize, flag;

	demand = MemoryRead(gPC + 0, 0);
	start = MemoryRead(gPC + 1, 0);
	maxsize = MemoryRead(gPC + 2, 0);
	flag = MemoryRead(gPC + 3, 0);
	if ((start + 128 + demand - 122) > (start + maxsize))
	{ /* stack overflow */
		gPC += 5;
		return;
	}
	if ((flag & 0x01) != (gReg->reg[gPIL][_STS] & 0x01))
	{
		gPC += 5;
		return;
	}
	MemoryWrite(gL + 1, start, 0, 2); /* L+1 ==> LINK */
	MemoryWrite(gB, start + 1, 0, 2); /* B   ==> PREVB */
	MemoryWrite(start + maxsize, start + 3, 0, 2); /* SMAX */
	gB = start + 128; /* + 200 oct. */
	/*:TODO:  Flag */
	MemoryWrite(gB + demand - 122, start + 2, 0, 2); /* STP */
	gPC += 6;
	return;
}

/* ENTR
 * IN: nothing. uses PC.
 * ADDR  : ENTR
 * ADDR+1: Stack demand
 * ADDR+2: Error return
 * ADDR+3: Normal return
 *
 */
void ndfunc_entr(ushort operand)
{
	ushort oldB, demand, smax, stp;
	demand = MemoryRead(gPC + 0, 0);
	smax = MemoryRead(gB - 125, 0); /* SMAX */
	if ((gB + demand - 122) > (smax))
	{ /* stack overflow */
		gPC += 1;
		return;
	}
	stp = MemoryRead(gB - 126, 0); /* STP */
	oldB = gB;
	gB = stp + 128;									/* Advance stack frame */
	MemoryWrite(gL + 1, gB - 128, 0, 2);			/* L+1 ==> LINK */
	MemoryWrite(oldB, gB - 127, 0, 2);				/* B   ==> PREVB */
	MemoryWrite(smax, gB - 125, 0, 2);				/* SMAX */
	MemoryWrite(gB + demand - 122, gB - 126, 0, 2); /* STP */
	gPC += 2;
}

/* LEAVE
 */
void ndfunc_leave(ushort operand)
{
	gPC = MemoryRead(gB - 128, 0);
	gB = MemoryRead(gB - 127, 0);
}

/* ELEAV
 */
void ndfunc_eleav(ushort operand)
{
	ushort tmp;
	tmp = MemoryRead(gB - 128, 0) - 1;
	MemoryWrite(tmp, gB - 128, 0, 2); /* LINK */
	MemoryWrite(gA, gB - 123, 0, 2);  /* A ==> ERRCODE */
	gPC = MemoryRead(gB - 128, 0);
	gB = MemoryRead(gB - 127, 0);
}


/************************ BYTE ************************/



/// <summary>
/// LBYT Load byte
/// Code: 142200
/// Format: LBYT
///
/// The 8 bit byte specified by the contents of the T and X registers is loaded into the A register bits 0-7, with the A register bits 8-15 cleared.
///
/// Affected: (A)
/// </summary>
void ndfunc_lbyt(ushort operand)
{

	ushort offset = gX >> 1;
	ushort memval = MemoryRead(gT + offset, true);

	if ((gX & 1) != 0)
	{ /* ODD BYTE = LOW */
		gA = memval & 0xFF;
	}
	else
	{
		/* EVEN BYTE = HIGH*/
		gA = (memval >> 8) & 0xFF;
	}
}

/// <summary>
/// SBYT - Store byte
/// Code: 142 600
/// Format: SBYT
///
/// The byte contained in the A register bits 0-7 is stored in one half of the effective location pointed by the T and X registers,
/// the second half of this effective location being unchanged. The contents of the A register are unchanged.
///
/// Affected: (EL)
/// </summary>
void ndfunc_sbyt(ushort operand)
{

	ushort offset = gX >> 1; /* same as divide by 2 */

	if ((gX & 1) != 0)

	{
		// Odd byte, write LSB value
		WriteVirtualMemory((uint)(gT + offset), gA, true, WRITEMODE_LSB);
	}
	else
	{
		// Even byte, write MSB value
		WriteVirtualMemory((uint)(gT + offset), gA, true, WRITEMODE_MSB);
	}
}

/// <summary>
/// MIX3 - Multiply index by 3
///
/// X <- ((A) — 1) *3
///
/// Format: MIX3
///
/// Code: 143 200
///
/// Multiply index by 3
/// The X register is set equal to the contents of the A register minus one multiplied by three, i.e., (X) <- [(A) - 1] *3
///
/// Affected: (X)
/// </summary>
void ndfunc_mix3(ushort operand)
{
	gX = (ushort)((gA - 1) * 3);
}

/*********************** REGISTER OPERANDS ***********************/

// Math register operations
void regop(ushort operand)
{ /* SWAP RAND REXO RORA RADD RCLR EXIT RDCR RING RSUB */
	int RAD, CLD, CM1, tmp;
	ushort sr, dr, source, destination;
	ushort old_gPC = gPC-1;

	RAD = ((operand & 0x0400) >> 10);
	CM1 = ((operand & 0x0080) >> 7);
	CLD = ((operand & 0x0040) >> 6);

	sr = ((operand & 0x0038) >> 3);
	dr = (operand & 0x0007);

	source = (sr == 0) ? 0 : gReg->reg[CurrLEVEL][sr] & 0xFFFF;	 /* handles special case when sr=STS reg */
	destination = (CLD) ? 0 : gReg->reg[CurrLEVEL][dr] & 0xFFFF; // Get destination value

	switch (RAD)
	{
	case 0: /* Logical operation - SWAP RAND REXO RORA */
		if (dr != 0)
		{
			switch ((operand & 0x0300) >> 8)
			{
			case 0:								/* SWAP */
				tmp = gReg->reg[CurrLEVEL][dr]; /* temp if we need to do the swap */
				gReg->reg[CurrLEVEL][dr] = (CM1) ? ~source : source;
				gReg->reg[CurrLEVEL][sr] = (CLD) ? 0 : (ushort)(tmp & 0xFFFF);
				break;
			case 1: /* RAND */
				gReg->reg[CurrLEVEL][dr] &= (CM1) ? ~source : source;
				gReg->reg[CurrLEVEL][dr] = (CLD) ? 0 : gReg->reg[CurrLEVEL][dr];
				break;
			case 2: /* REXO */
				gReg->reg[CurrLEVEL][dr] = (CLD) ? ((CM1) ? ~source : source) : ((CM1) ? gReg->reg[CurrLEVEL][dr] ^ ~source : gReg->reg[CurrLEVEL][dr] ^ source);
				break;
			case 3: /* RORA */
				gReg->reg[CurrLEVEL][dr] = (CLD) ? ((CM1) ? ~source : source) : ((CM1) ? gReg->reg[CurrLEVEL][dr] | ~source : gReg->reg[CurrLEVEL][dr] | source);
				break;
			}
		}
		break;
	case 1: /* Arithmetic operation - RADD RCLR EXIT RDCR RINC RSUB */
		if (dr != 0)
		{
			tmp = gReg->reg[CurrLEVEL][dr]; /* use this insted of (dr) as we need to check for carry and things */
			switch ((operand & 0x0380) >> 7)
			{
			case 0: /* RADD */
				tmp = do_add(destination, source, 0);
				break;
			case 1: /* RADD CM1 */
				tmp = do_add(destination, ~source, 0);
				break;
			case 2: /* RADD AD1 */
				tmp = do_add(destination, source, 1);
				break;
			case 3: /* RADD AD1 CM1 */
				tmp = do_add(destination, ~source, 1);
				break;
			case 4: /* RADD ADC */
				tmp = do_add(destination, source, getbit(_STS, _C));
				break;
			case 5: /* RADD ADC CM1 */
				tmp = do_add(destination, ~source, getbit(_STS, _C));
				break;
			case 6: /* NOOP */
				break;
			case 7: /* NOOP */
				break;
			}
			gReg->reg[CurrLEVEL][dr] = (ushort)(tmp & 0xFFFF);
		}
		else
		{
			setbit(_STS, _C, 0);
		}
		break;
	}

	if ((DISASM) && (dr == _P))
	{
		disasm_userel(old_gPC, gPC);
	}
}


/********************* some */


/*
 * DoMCL - Masked Clear
 *  Affected: Internal register specified
 *  (Only STS, PID & PIE possible)
 *  <IR> = <IR> & (~A)
 *
 * NOTE:: STS need to be checked.
 * NOTE:: Privileged instructions
 */
void DoMCL(ushort instr)
{
	if (!CheckPriv())
		return;

	switch (instr & 0x0F)
	{
	case 01: // STS
		gReg->reg[CurrLEVEL][_STS] &= ~(gA & 0x00FF);
		break;
	case 06: // PID
		/* This affects interrupt, so do locking and checking. */

		gPID &= ~gA;

		gCHKIT = true; // we need to check PK after this
		break;
	case 07: // PIE
		/* This affects interrupt, so do locking and checking. */
		gPIE &= ~gA;
		gCHKIT = true; // we need to check PK after this
		break;
	default:
		/* :TODO: Check if we need to do illegal instruction handling */
		break;
	}
}

/*
 * DoMST - Masked SET
 *  Affected: Internal register specified
 *  (Only STS, PID & PIE possible)
 *  <IR> = <IR> | (A)
 *
 * NOTE:: STS need to be checked.
 * NOTE:: Privileged instructions
 */
void DoMST(ushort instr)
{
	if (!CheckPriv())
		return;

	switch (instr & 0x0F)
	{
	case 01: // STS
		gReg->reg[CurrLEVEL][0] |= (gA & 0x00ff);
		break;
	case 06: // PID
		/* This affects interrupt, so do locking and checking. */

		gPID |= gA;
		gCHKIT = true; // we need to check PK after this
		
		break;
	case 07: // PIE
		/* This affects interrupt, so do locking and checking. */
		gPIE |= gA;
		gCHKIT = true; // we need to check PK after this
		break;
	default:
		/* :TODO: Check if we need to do illegal instruction handling */
		break;
	}
}
  

/*
 * DoTRA - Transfer to register
 *  Affected: Accumulator
 *  A = <IR>;
 *
 * NOTE: Privileged instructions
 */
void DoTRA(ushort instr)
{
	if (!CheckPriv())
		return;

	ushort temp, level;
	ushort i;
	switch (instr & 0x0F)
	{
	case 00: /* TRA PANS */
		gA = gPANS;
		break;
	case 01:								 /* TRA STS */
		gA = gReg->reg[gPIL][_STS] & 0x00FF; /* Only lower 8 bits */
		gA |= gReg->reg_STS & 0xFF00;		 /* Upper 8 bits - SYSTEM bits*/

		break;
	case 02: /* TRA OPR */
		gA = gOPR;
		break;
	case 03: /* TRA PGS */
		/* TODO:: Check that this also is supposed to clear the PGS as it "unlocks" it */
		gA = gPGS;
		gPGS_Lock = false;
		gPGS = 0;
		break;
	case 04: /* TRA PVL */
		/* This one has a strange format. Described in ND-100 Functional Description section 2.9.2.5.4 */
		gA = 0;							  /* Clean it */
		gA = (gPVL & 0x0F) << 3 | 0xd782; /* = IRR (PVL) DP */
		break;
	case 05: /* TRA IIC */
		/* Manuals says(2.2.4.3) that this should be a number equal to the highest bit set in (IID & IIE) - Roger */
		/* Only bit 1-10 is used, so we only return a value between 1 and 10  or else  zero */

		gIIC = calcIIC();

		gA = gIIC;

		gIIC = 0;
		gIID = 0;

		gCHKIT = true; // recalc PK

		break;
	case 06: /* TRA PID */
		gA = gPID;
		break;
	case 07:
		gA = gPIE;
		break;
	case 010:					  // CSR
		gA = (1 << 2) | (1 << 3); // Always report bit 2 and 3 as 1. Bit 2="MAN DIS" (Cache disabled manually as Emulator doesnt need caching. Bit 3=Cache Clear Finished
		// gA = gCSR;
		break;
	case 011: /* TRA ACTL */
		gA = 1 << CurrLEVEL;
		break;
	case 012: /* TRA ALD */
		gA = gALD;
		break;
	case 013: /* TRA PES */
		gA = gPES;
		break;
	case 014: /* PGC/PCR - Paging Control Register */
		temp = gA;
		level = (temp >> 3) & 0x0f;
		gA = gReg->reg_PCR[level];
		if (mmsType == MMS1)
		{
			gA &= ~(1 << 2); // Clear bit 2 for MMS1 mode
		}

		// Always clear bit 15, as thats the way of the ND110 microcode
		gA = gA & ~(1 << 15);

		break;
	case 015: /* TRA PEA */
		gA = gPEA;

		// Unlock PEA and PES
		gPEA_Lock = false;
		gPES_Lock = false;
		break;
	default: /* These registers dont exist, so just return 0 for now FIXME: Check correct behaviour.*/
			 // gA = 0;
		//  do nothing is the correct
		break;
	}
}

/*
 * DoEXR - Run instruction in source register
 */
void DoEXR(ushort instr)
{
	ushort sr, exr_instr;
	char disasm_str[256];
	sr = (instr >> 3) & 0x07;
	if (sr)
		exr_instr = gReg->reg[CurrLEVEL][sr];
	else
		exr_instr = 0;

	if (0140600 == extract_opcode(exr_instr))
	{						 /* ILLEGAL:: EXR of EXR */
		setbit(_STS, _Z, 1); //: TODO: activate CPU trap on level 14!!!
		return;
	}
	if (DISASM)
		disasm_exr(gPC, exr_instr);

	// Execute opcode but do not touch Program Counter
	do_op(exr_instr, true);
}

/*
 * DoWAIT - Give up prio instruction
 * NOTE:: Only basic parts fixed yet, this is a fairly complex one
 *
 * NOTE:: Privileged instructions
 */
void DoWAIT(ushort instr)
{
	if (!CheckPriv())
		return;

	ushort temp;
	if (!STS_IONI)
	{
		// If the interrupt system is OFF
		// The ND-110 stops with the program counter (P register) pointing at the instruction after the WAIT and the front panel RUN indicator is turned off.
		// To restart the system, type ! on the console terminal
		printf("\r\nWAIT when IONI is off PIL[%d] PC[%6o] PID[0x%4X] PIE[0x%4X] IONI[%d] PONI[%d] STS_HI[%4X] STS_LO[%4X]\r\n", gPIL, gPC, gPID, gPIE, STS_IONI, STS_PONI, gReg->reg_STS, gReg->reg[gPIL][_STS]);
		set_cpu_run_mode(CPU_STOPPED);		
		return;
	}

	if (CurrLEVEL == 0)
	{
		// Cant go lower
		return;
	}
	
	
	temp = ~(1 << CurrLEVEL); /* Now we have a 0 in the position we want */
	gPID &= temp;			  /* Give up this level */

	gCHKIT = true; // recalc PK (and do a level switch if needed)
}

/* LWCS (Privileged)
 */
void ndfunc_lwcs(ushort instr)
{
	// LWCS is a no-operation on the ND-110
	// The ND-110 is software compatible but nor microcode compatible and writing to the writable control store has no meaning in the ND-110.
	// A no-operation is executed so that programs written for the ND-100 and NORD-10 can continue

	if (!CheckPriv())
		return;

	// noop
}


//TODO: Make these into callbacks
extern void ProcessTerminalPanc(void);
extern void ProcessTerminalLamp(void);

/*
 * DoTRR - Transfer to register
 *  Affected: Internal register specified
 *  <IR> = A;
 *
 * NOTE: STS and PCR NOT fixed yet!!!
 *
 * NOTE: Privileged instructions
 */
void DoTRR(ushort instr)
{
	if (!CheckPriv())
		return;

	int s;
	ushort temp, level;
	switch (instr & 0x0F)
	{
	case 00: // TRR PANC
		gPANC = gA;
		ProcessTerminalPanc();

		break;
	case 01: // TRR STS
		/* ND-06.029.1 ND-110 Instruction Set, lists only lower 8 bits as changeable... */
		gReg->reg[CurrLEVEL][_STS] = (gReg->reg[CurrLEVEL][_STS] & 0xff00) | (gA & 0x00ff); /* Only change LSB  */
		break;
	case 02: // TRR LMP
		gLMP = gA;        
		ProcessTerminalLamp();

		break;
	case 03: /* PGC/PCR - Paging Control Register */
		temp = gA;
		level = (temp >> 3) & 0x0f;
		if (mmsType == MMS1)
		{
			temp &= ~(1 << 2); // Force Clear bit 2 for MMS1 mode
		}
		gReg->reg_PCR[level] = temp;

		break;
	case 05: // TRR IIE
		gIIE = gA;
		gCHKIT = true; // we need to check PK after this
		break;
	case 06: // TRR PID
		// TODO:? according to manual it can only set bit 15,13-12-11
		gPID = gA;
		gCHKIT = true; // we need to check PK after this
		break;
	case 07: // TRR PIE
		gPIE = gA;
		gCHKIT = true; // we need to check PK after this
		break;
	case 010: // TRR CCL (cache clear)
		gCCL = gA;
		break;
	case 011: // TRR LCIL
		gLCIL = gA;
		break;
	case 012: // TRR UCIL
		gUCIL = gA;
		break;
	case 013: /* TRR CILP (ND110 only??) */
		break;
	case 015: /* TRR ECCR (ND110 only??) */
		gECCR = gA;
		break;
	case 017: /* TRR CS (ND110 only) */
		break;
	}
}


/*
 * DoSRB - Store register block.
 *  Affected:(EL),+ 1 +2 + 3 + 4 + 5 + 6 + 7
 *            P    X  T   A   D   L  STS  B
 *
 *  Uses the alternative pagetable!
 */
void DoSRB(ushort operand)
{

	if (!CheckPriv())
		return;

	ushort lvl, addr;
	ushort sts_temp;

	lvl = ((operand & 0x0078) >> 3);
	addr = gX;

    sts_temp = gReg->reg[lvl][_STS] & 0x00ff;

	// If the current program level is specified, the stored P register points to the instruction following SRB.
	MemoryWrite(gReg->reg[lvl][_P], addr, true, 2);
	MemoryWrite(gReg->reg[lvl][_X], addr + 1, true, 2);
	MemoryWrite(gReg->reg[lvl][_T], addr + 2, true, 2);
	MemoryWrite(gReg->reg[lvl][_A], addr + 3, true, 2);
	MemoryWrite(gReg->reg[lvl][_D], addr + 4, true, 2);
	MemoryWrite(gReg->reg[lvl][_L], addr + 5, true, 2);
	MemoryWrite(sts_temp, addr + 6, true, 2); /* Only write LSB of STS */
	MemoryWrite(gReg->reg[lvl][_B], addr + 7, true, 2);
}

/*
 * DoLRB - Load register block.
 *           (EL),+ 1 +2 + 3 + 4 + 5 + 6 + 7
 *  Affected:  P    X  T   A   D   L  STS  B
 *
 *  Uses the alternative pagetable!
 */

/// <summary>
/// LRB - Load register Block
/// Code: 152 6n2
/// Format: SRB <level* 10>
///
/// The instruction <LRB level * 10B> loads the contents  of the register block on program level specified in the
/// level field of the instruction.
///
/// The specified register block is  loaded by the contents of succeeding memory locations starting at the location
/// specified by the contents of the X register.
///
/// If the current program level is specified, the P register is not affected.
///
/// The LBR instruction is privileged
/// </summary>
void DoLRB(ushort operand)
{

	if (!CheckPriv())
		return;

	ushort lvl, addr;

	lvl = ((operand & 0x0078) >> 3);
	addr = gX;


	if (lvl != CurrLEVEL)
	{ /* Dont change P on current level if this happens to be specified */
		gReg->reg[lvl][_P] = MemoryRead(addr, true);
	}
	gReg->reg[lvl][_X] = MemoryRead(addr + 1, true);
	gReg->reg[lvl][_T] = MemoryRead(addr + 2, true);
	gReg->reg[lvl][_A] = MemoryRead(addr + 3, true);
	gReg->reg[lvl][_D] = MemoryRead(addr + 4, true);
	gReg->reg[lvl][_L] = MemoryRead(addr + 5, true);
	gReg->reg[lvl][_STS] = (gReg->reg[lvl][_STS] & 0xff00) | (MemoryRead(addr + 6, true) & 0x00ff); /* Only load LSB STS */	
	gReg->reg[lvl][_B] = MemoryRead(addr + 7, true);
	
}

bool IsSkip(ushort instr)
{
	ushort sr, dr, source, desti;
	signed short ss, sd, sgr, ovf;
	char z, o, c, s;
	sr = (instr >> 3) & 0x07;
	dr = (instr >> 0) & 0x07;
	source = (0 == sr) ? 0 : gReg->reg[CurrLEVEL][sr]; /* Never use STS reg but zero value instead */
	desti = (0 == dr) ? 0 : gReg->reg[CurrLEVEL][dr];  /* Never use STS reg but zero value instead */
	ss = (signed short)source;
	sd = (signed short)desti;

	/* Ok, lets set flags */
	z = (0 == (desti - source)) ? 1 : 0;
	sgr = sd - ss;
	ovf = (sd & ~ss & ~sgr) | (~sd & ss & sgr);
	o = (ovf < 0) ? 1 : 0;
	c = ((desti - source) < 0) ? 0 : 1;
	s = ((ushort)(sd - ss) >> 15) & 0x01;

	/* And use these to do the skipping, so we try and follow ND behaviour */
	switch ((instr >> 8) & 0x07)
	{
	case 0: /* EQL */
		if (z)
			return true;
		break;
	case 1: /* GEQ */
		if (!s)
			return true;
		break;
	case 2: /* GRE */
		if (!(s ^ o))
			return true;
		break;
	case 3: /* MGRE */
		if (c)
			return true;
		break;
	case 4: /* UEQ */
		if (!z)
			return true;
		break;
	case 5: /* LSS */
		if (s)
			return true;
		break;
	case 6: /* LST */
		if (s ^ o)
			return true;
		break;
	case 7: /* MLST */
		if (!c)
			return true;
		break;
	}
	return false;
}

void do_bops(ushort operand)
{
	ushort bn, dr, desti;
	bn = ((operand & 0x0078) >> 3);
	dr = (operand & 0x0007);

    switch ((operand & 0x0780) >> 7)
	{
	case 0: /* BSET ZRO */
		setbit(dr, bn, 0);
		break;
	case 1: /* BSET ONE */
		setbit(dr, bn, 1);
		break;
	case 2: /* BSET BCM */
		desti = getbit(dr, bn);
		desti ^= 1; /* XOR with one to invert bit */
		setbit(dr, bn, desti);
		break;
	case 3: /* BSET BAC */
		setbit(dr, bn, getbit(_STS, _K));
		break;
	case 4: /* BSKP ZRO */
		if (!getbit(dr, bn))
			gPC++; /* Skip next instruction if zero */
		break;
	case 5: /* BSKP ONE */
		if (getbit(dr, bn))
			gPC++; /* Skip next instruction if one */
		break;
	case 6: /* BSKP BCM */
		if ((getbit(dr, bn) ^ 1) == getbit(_STS, _K))
			gPC++; /* Skip next instruction if bit complement */
		break;
	case 7: /* BSKP BAC */
		if (getbit(dr, bn) == getbit(_STS, _K))
			gPC++; /* Skip next instruction if equal */
		break;
	case 8: /* BSTC */
		setbit(dr, bn, (getbit(_STS, _K) ^ 1));
		setbit(_STS, _K, 1);
		break;
	case 9: /* BSTA */
		setbit(dr, bn, getbit(_STS, _K));
		setbit(_STS, _K, 0);
		break;
	case 10: /* BLDC */
		setbit(_STS, _K, getbit(dr, bn) ^ 1);
		break;
	case 11: /* BLDA */
		setbit(_STS, _K, getbit(dr, bn));
		break;
	case 12: /* BANC */
		setbit(_STS, _K, ((getbit(dr, bn) ^ 1) & getbit(_STS, _K)));
		break;
	case 13: /* BAND */
		setbit(_STS, _K, (getbit(dr, bn) & getbit(_STS, _K)));
		break;
	case 14: /* BORC */
		setbit(_STS, _K, ((getbit(dr, bn) ^ 1) | getbit(_STS, _K)));
		break;
	case 15: /* BORA */
		setbit(_STS, _K, (getbit(dr, bn) | getbit(_STS, _K)));
		break;
	}
}

ushort ShiftReg(ushort reg, ushort instr)
{
	bool isneg = ((instr & 0x0020) >> 5) ? 1 : 0;
	ushort offset = (isneg) ? (~((instr & 0x003F) | 0xFFC0) + 1) : (instr & 0x003F);
	ushort shifttype = ((instr >> 9) & 0x03);
	int i, tmp, msb;
	int m = getbit(_STS, _M);
	tmp = m; /* just in case.. */
	for (i = 1; i <= offset; i++)
	{
		tmp = (isneg) ? (reg & 0x01) : ((reg >> 15) & 0x01); /* tmp = bit shifted out */
		msb = reg >> 15 & 1;								 /* msb before shift */
		reg = (isneg) ? reg >> 1 : reg << 1;
		switch (shifttype)
		{
		case 0:																 /* Plain */
			reg = (isneg) ? ((reg & 0x7fff) | (msb << 15)) : (reg & 0xfffe); /* SHR : SHL */
			break;
		case 1: /* ROT */
			reg = (isneg) ? ((reg & 0x7fff) | (tmp << 15)) : ((reg & 0xfffe) | tmp);
			break;
		case 2: /* ZIN */
			reg = (isneg) ? (reg & 0x7fff) : (reg & 0xfffe);
			break;
		case 3: /* LIN */
			reg = (isneg) ? ((reg & 0x7fff) | (m << 15)) : ((reg & 0xfffe) | m);
			break;
		}
	}
	setbit(_STS, _M, tmp);
	return reg;
}

ulong ShiftDoubleReg(ulong reg, ushort instr)
{
	bool isneg = ((instr & 0x0020) >> 5) ? 1 : 0;
	ushort offset = (isneg) ? (~((instr & 0x003F) | 0xFFC0) + 1) : (instr & 0x003F);
	ushort shifttype = ((instr >> 9) & 0x03);
	int i, tmp, msb;
	int m = getbit(_STS, _M);
	tmp = m; /* just in case.. */
	for (i = 1; i <= offset; i++)
	{
		tmp = (isneg) ? (reg & 0x01) : ((reg >> 31) & 0x01); /* tmp = bit shifted out */
		msb = reg >> 31 & 1;								 /* msb before shift */
		reg = (isneg) ? reg >> 1 : reg << 1;
		switch (shifttype)
		{
		case 0:																		 /* Plain */
			reg = (isneg) ? ((reg & 0x7fffffff) | (msb << 31)) : (reg & 0xfffffffe); /* SHR : SHL */
			break;
		case 1: /* ROT */
			reg = (isneg) ? ((reg & 0x7fffffff) | (tmp << 31)) : ((reg & 0xfffffffe) | tmp);
			break;
		case 2: /* ZIN */
			reg = (isneg) ? (reg & 0x7fffffff) : (reg & 0xfffffffe);
			break;
		case 3: /* LIN */
			reg = (isneg) ? ((reg & 0x7fffffff) | (m << 31)) : ((reg & 0xfffffffe) | m);
			break;
		}
	}
	setbit(_STS, _M, tmp);
	return reg;
}

/*
 * DoIDENT
 * Handles IDENT PLxx instructions
 */
void DoIDENT(ushort priolevel)
{

	int id = IO_Ident(priolevel);
	if (id >= 0)
	{
		gA = id & 0xFFFF;
	}
	else
	{
		gA = 0;

		if (priolevel != 13)	   // ignore RTC
			interrupt(14, 1 << 7); /* IOX Error if no IDENT code found */

	}
	return;
}

/// <summary>
/// RDUS - Read don't use cache Code: 140127
/// Code: 140 127
/// Format: RDUS
///
///  This instruction reads the content of the memory location pointed to by the T—register into the A—register.
///  The address in the T-register is a logical memory address.Translation to a physical memory address is normally done by using the page tables.
///  However, the translation will use the alternative page table when PTM is on (Page Table Modus) (status register bit 0 is 1) and the paging system is on, PON.
/// </summary>

void DoRDUS(ushort instr)
{
	gA = MemoryRead(gT, true);
}

/// <summary>
/// TSET - Test and set
/// Code: 140 123
/// Format: TSET
///
/// This instruction writes -1 into the memory address pointed to by the T—register.
/// Simultaneously, the old content of the same address is read into the A-register.This read/write sequence is performed with the memory system ’locked',
/// so that the two memory accesses cannot be split by other accesses on other memory channels.
/// This may be used to implement processor synchronizing.
/// The address in the T—register is a logical memory address.
/// Translation to a physical memory address is normally done by using the page tables.
/// However, the translation will use the alternative page table when PTM is on (Page Table Modus) (status register bit 0 is 1) and the paging system is on, PON.
///
/// The old content of the memory address is always read from the memory, and never from the cache, Data is written both to memory and cache.
/// </summary>
void DoTSET(ushort instr)
{
	// regs.currentRegisters.A = (ushort)cpu.ReadVirtualMemory(regs.currentRegisters.T, PageTable.AlternativePageTable);
	// cpu.WriteVirtualMemory(regs.currentRegisters.T, 0xFFFF, PageTable.AlternativePageTable); // Write -1

	gA = MemoryRead(gT, true);
	MemoryWrite(0xFFFF, gT, true, 2);
}

/// <summary>
/// MOVEW - WORD BLOCK INSTRUCTION
///
/// Code 143 1nn
/// If the memory management system is off, bank 0 of physical memory is addressed. (Bit PTM of the STS register is zero) and the following transfer fields become equivalent:
///   nn = 00 = 01 = 03 = 04
///   nn = 02 = 05
///   nn = 06 = 07
///
/// MOVEW can be interrupted. L, A, D, X, T and P registers are then changed to restart execution.
///
/// A and D - Source address
/// X and T - Destination address
/// L		- The number of words to be moved (max 2048)
///
/// A and/or X are used for physical memory-block moves and are incremented when the D and/or T registers overflow.
///
/// If the L register contains a value grater then 2048 (L=o4000) no words are moved and A,D, T and X are unchanged.
///
/// After transfer the register contains: A,D, T,X - The addresses after the last moved word . L = zero
///
/// Format: MOVEW
/// </summary>
void DoMOVEW(ushort instr)
{
	unsigned int sourceAddress = gD;
	unsigned int destinationAddress = gT;
	ushort cnt = gL;

	ushort displacement = (instr & 0x00F);

	// Check if source and destination are in physical memory
	bool isSourcePhysical = false;
	bool isDestinationPhysical = false;

	switch (displacement)
	{
	case 2:
	case 5:
		destinationAddress |= (gX << 16);
		isDestinationPhysical = true;
		break;

	case 6:
	case 7:
		sourceAddress |= (gA << 16);
		isSourcePhysical = true;
		break;
	case 8:
		destinationAddress |= (gX << 16);
		isDestinationPhysical = true;

		sourceAddress |= (gA << 16);
		isSourcePhysical = true;
		break;
	}

	// Check for priveleged instruction
	if (isSourcePhysical || isDestinationPhysical)
	{
		if (!CheckPriv())
			return;
	}

	// Warning: In the loop of read/write below, PageFault can occur, and the instruction can be restarted.
	ushort temp = 0;

	while (cnt > 0)
	{
		switch (displacement)
		{
		case 0: // move from PT to PT
			temp = MemoryRead(sourceAddress, false);
			MemoryWrite(temp, destinationAddress, false, 2);
			break;
		case 1: // move from PT to APT
			temp = MemoryRead(sourceAddress, false);
			MemoryWrite(temp, destinationAddress, true, 2);
			break;
		case 2: // move from PT to physical memory
			temp = MemoryRead(sourceAddress, false);
			WritePhysicalMemory(destinationAddress, temp, true);
			break;
		case 3: // move from APT to PT
			temp = (ushort)MemoryRead(sourceAddress, true);
			MemoryWrite(temp, destinationAddress, false, 2);
			break;
		case 4: // move from APT to APT
			temp = (ushort)MemoryRead(sourceAddress, true);
			MemoryWrite(temp, destinationAddress, true, 2);
			break;
		case 5: // move from APT to physical memory
			temp = (ushort)MemoryRead(sourceAddress, true);
			WritePhysicalMemory(destinationAddress, temp, true);
			break;
		case 6: // move from physical memory to PT
			temp = ReadPhysicalMemory(sourceAddress, true);
			MemoryWrite(temp, destinationAddress, false, 2);
			break;
		case 7: // move from physical memory to APT
			temp = ReadPhysicalMemory(sourceAddress, true);
			MemoryWrite(temp, destinationAddress, true, 2);
			break;

		case 8: // move from physical memory to physical memory
			temp = ReadPhysicalMemory(sourceAddress, true);
			WritePhysicalMemory(destinationAddress, temp, true);
			break;

		default:
			break;
		}
		sourceAddress++;
		destinationAddress++;
		cnt--;
	}

	// After here, no PageFault can occur - update register values

	// update L
	gL = cnt;

	// Update Source with the new address
	gD = (sourceAddress & 0xFFFF);
	if (isSourcePhysical)
	{
		gA = (sourceAddress >> 16) & 0xFFFF;
	}

	// Update destination
	gT = (destinationAddress & 0xFFFF);
	if (isDestinationPhysical)
	{
		gX = (destinationAddress >> 16) & 0xFFFF;
	}
}

#define _removed_MOVB_AND_MOVBF_ 1
#if _removed_MOVB_AND_MOVBF_ // replaced with doMoveBytes
/*
 * MOVB instruction. TODO:: Fix edge case and document params here...
 */
void DoMOVB(ushort instr)
{
	ushort source, dest, lens, lend, len, s_lr, d_lr, s_apt, d_apt;
	int dir; /* direction, 0=low to high, 1 = high to low */
	int i;
	ushort thebyte;
	ushort addr_d, addr_s;

	addr_d = 0;
	addr_s = 0;
	dir = 0;
	source = gA;
	dest = gX;
	lens = gD & 0x0fff;
	lend = gT & 0x0fff;
	s_lr = ((gD >> 15) & 1);
	d_lr = ((gT >> 15) & 1);
	s_apt = ((gD >> 14) & 1);
	d_apt = ((gT >> 14) & 1);
	len = (((int)lens - lend) < 0) ? lens : lend; /* get smallest length as number to copy */
	/* Check overlap if any and direction to copy */
	if (((int)source - dest) < 0)
	{
		dir = 1;
	}
	else if (((int)source - dest) == 0)
	{ /* :TODO: check bytes to determine direction, or if no need to copy exist */
	}
	else
	{
		dir = 0;
	}

	/* COPY */
	if (dir)
	{ /* high to low */
		for (i = len - 1; i >= 0; i--)
		{
			addr_s = source + ((i + s_lr) >> 1); /* Word adress of byte to read */
			thebyte = MemoryRead(addr_s, s_apt);
			thebyte = ((i + d_lr) & 1) ? thebyte : (thebyte >> 8) & 0xff; /* right, LSB : left, MSB */
			addr_d = dest + ((i + d_lr) >> 1);							  /* Word adress of byte to write */
			MemoryWrite(thebyte, addr_d, d_apt, ((i + d_lr) & 1));
		}
		i = 0;
	}
	else
	{ /* low to high */
		for (i = 0; i < len; i++)
		{
			addr_s = source + ((i + s_lr) >> 1); /* Word adress of byte to read */
			thebyte = MemoryRead(addr_s, s_apt);
			thebyte = ((i + d_lr) & 1) ? thebyte : (thebyte >> 8) & 0xff; /* right, LSB : left, MSB */
			addr_d = dest + ((i + d_lr) >> 1);							  /* Word adress of byte to write */
			MemoryWrite(thebyte, addr_d, d_apt, ((i + d_lr) & 1));
		}
	}

	gD &= 0x7000;				  /* Null number of bytes, as per manual, also null bit 15 */
	gT &= 0x7000;				  /* Null number of bytes, also null bit 15 */
	gD |= ((i + d_lr) & 1) << 15; /* set bit 15 to point to next free byte */
	gT |= ((i + d_lr) & 1) << 15; /* set bit 15 to point to next free byte */
	gT |= len & 0x0fff;			  /* number of bytes done to lowest 12 bits*/

	gA = addr_s + ((len + s_lr) >> 1);
	gX = addr_d + ((len + d_lr) >> 1);	

	gPC++; /* This function has a SKIP return on no error, which is always? */
}

/*
 * MOVBF instruction. TODO:: ALL
 */
void DoMOVBF(ushort instr)
{
	ushort source, dest, lens, lend, len, s_lr, d_lr, s_apt, d_apt;
	int i;
	ushort thebyte;
	ushort addr_d, addr_s;
	source = gA;
	dest = gX;
	bool overlap;

	addr_d = 0;
	addr_s = 0;
	lens = gD & 0x0fff;
	lend = gT & 0x0fff;
	s_lr = ((gD >> 15) & 1);
	d_lr = ((gT >> 15) & 1);
	s_apt = ((gD >> 14) & 1);
	d_apt = ((gT >> 14) & 1);

	len = (((int)lens - lend) < 0) ? lens : lend; /* get smallest length as number to copy */

	if (source > dest)
		overlap = false;
	else if ((ushort)((ushort)(ceil(len / 2)) + source - 1) > dest)
		overlap = true;
	else
		overlap = false;

	for (i = 0; i < len; i++)
	{
		addr_s = source + ((i + s_lr) >> 1); /* Word adress of byte to read */
		thebyte = MemoryRead(addr_s, s_apt);
		thebyte = ((i + d_lr) & 1) ? thebyte : (thebyte >> 8) & 0xff; /* right, LSB : left, MSB */
		addr_d = dest + ((i + d_lr) >> 1);							  /* Word adress of byte to write */
		MemoryWrite(thebyte, addr_d, d_apt, ((i + d_lr) & 1));
		lens--;
		lend--;
	}

	gA = source + ((len + s_lr) >> 1);
	gX = dest + ((len + d_lr) >> 1);

	gD &= 0xefff;				  /* Null bit 12 */
	gT &= 0xcfff;				  /* Null bit 12 & 13 */
	gD |= ((i + d_lr) & 1) << 15; /* set bit 15 to point to next free byte */
	gT |= ((i + d_lr) & 1) << 15; /* set bit 15 to point to next free byte */

	gD &= 0xf000;		 /* clean lowest bits before or */
	gT &= 0xf000;		 /* clean lowest bits before or */
	gD |= lens & 0x0fff; /* decremented byte counter to lowest 12 bits*/
	gT |= lend & 0x0fff; /* decremented byte counter to lowest 12 bits*/

	if (!overlap)
		gPC++; /* This function has a SKIP return on no error */

	return;
}
#endif


void add_A_mem(ushort eff_addr, bool UseAPT)
{
	int temp, data, oldreg;
	oldreg = gA;
	data = MemoryRead(eff_addr, UseAPT);
	temp = gA + data;

	// FIXME - ADD FLAG HANDLING CORRECTLY FOR C,O,Q FLAGS (CHECK AGAIN THINK WE MIGHT HAVE SUBTLE BUGS)

	if ((temp > 0xFFFF) || (temp < 0))
	{
		setbit(_STS, _C, 1);
		if ((oldreg & 0x8000) && (data & 0x8000) && !(temp & 0x8000))
		{
			setbit(_STS, _Q, 1);
		}
		else
		{
			setbit(_STS, _Q, 0);
		}
	}
	else
	{
		setbit(_STS, _C, 0);
		if (!(oldreg & 0x8000) && !(data & 0x8000) && (temp & 0x8000))
		{
			setbit(_STS, _Q, 1);
		}
		else
		{
			setbit(_STS, _Q, 0);
		}
	}

	gA = (temp & 0xFFFF);
}

/*
 * Move bytes in memory
 * Note: This is part of the commercial instruction set
 * It seems SINTRAN doesnt use this function for booting and operating
 * checkOverlapping: MOVBF sets this to true, MOVB sets this to false
 */
void doMoveBytes(bool checkOverlapping)
{
	const int LEN_MASK = 0xFFF;
	int readValue;

	int numBytesSource = gD & LEN_MASK; // Source length
	int numBytesDest = gT & LEN_MASK;	// Destination length
	if (numBytesDest < numBytesSource)
		numBytesSource = numBytesDest; // Cap number of bytes to max length of Destination

	// If Bit 13 is set, then setup has been executed and we are returning from an interrupt
	if (!(gD & (1 << 13)))
	{
		gT = (gT & 0xC000) | numBytesSource;
		gD = (gT & 0xC000);

		// Mark D bit 13 with setup done
		gD |= (1 << 13);
	}

	if (checkOverlapping)
	{
		// Convert byte count to word count for addressing
		int numWordsD = numBytesSource >> 1; // Same as numBytesD / 2

		// Calculate start and end positions for source and destination in terms of words
		int sourceStart = gA;
		int destinationStart = gX;
		int sourceEnd = sourceStart + numWordsD;
		int destinationEnd = destinationStart + numWordsD;

		// Check for forbidden overlap
		// Overlap is forbidden if destination overlaps source before it is read
		if (destinationStart < sourceEnd && destinationEnd > sourceStart)
		{
			// OVERLAP EXISTS - ILLEGAL IF 'MOVBF'!!
			// Forbidden overlap exists, return with error (no skip)
			return;
		}
	}

	bool useAPT = true; // Use alternative page table
	WriteMode readMode;
	WriteMode writeMode;

	if (gX < gA)
	{
		// High to low
		for (int i = (gT & LEN_MASK); i > 0; i--)
		{
			// Bit 15: 0=>MSB, 1=> LSB
			readMode = (gD & (1 << 15)) ? WRITEMODE_LSB : WRITEMODE_MSB;
			readValue = MemoryRead(gA, useAPT);

			if (readMode == WRITEMODE_MSB)
			{
				readValue = (readValue >> 8) & 0xFF;
			}
			else
			{
				readValue = readValue & 0xFF;
			}

			WriteMode writeMode = (gT & (1 << 15)) ? WRITEMODE_LSB : WRITEMODE_MSB;
			MemoryWrite(readValue, gX, useAPT, writeMode);

			gD ^= (1 << 15); // Flip D bit 15
			if (!(gD & (1 << 15)))
				gA--;

			gT ^= (1 << 15); // Flip T bit 15
			if (!(gT & (1 << 15)))
				gX--;
		}
	}
	else
	{
		// Low to High
		for (int i = (gD & LEN_MASK); i < (gT & LEN_MASK); i++)
		{
			// Bit 15: 0=>MSB, 1=> LSB
			WriteMode readMode = (gD & (1 << 15)) ? WRITEMODE_LSB : WRITEMODE_MSB;
			readValue = MemoryRead(gA, useAPT);

			if (readMode == WRITEMODE_MSB)
			{
				readValue = (readValue >> 8) & 0xFF;
			}
			else
			{
				readValue = readValue & 0xFF;
			}

			WriteMode writeMode = (gT & (1 << 15)) ? WRITEMODE_LSB : WRITEMODE_MSB;
			MemoryWrite(readValue, gX, useAPT, writeMode);

			gD ^= (1 << 15); // Flip D bit 15
			if (!(gD & (1 << 15)))
				gA++;

			gT ^= (1 << 15); // Flip T bit 15
			if (!(gT & (1 << 15)))
				gX++;
		}
	}

	// After execution, bit 15 of the D and T registers point to the end of the field that has been moved.
	// Note: DON'T CLEAR bit 15 of D and T, but clear bits 13 and 12.

	// After execution the field length of the D (source) equals Zero
	// Note: Clear setup and count bits
	gD &= 0xC000;

	// Documentation for MOVB and MOVBF says the same but implementation differs
	if (checkOverlapping)
		gT &= 0xC000; // MOVBF
	else
		gT &= 0xCFFF; // MOVB

	gPC++; // SKIP return
}

/*
 * MOVB
 */
void ndfunc_movb(ushort instr)
{
	doMoveBytes(false);
}

/*
 * MOVBF instruction.
 */
void ndfunc_movbf(ushort instr)
{
	doMoveBytes(true);
}

void sub_A_mem(ushort eff_addr, bool UseAPT)
{
	int temp, data, oldreg;
	oldreg = gA;
	data = MemoryRead(eff_addr, UseAPT);
	temp = gA - data;
	/*
	 * FIXME - ADD FLAG HANDLING CORRECTLY FOR C,O,Q FLAGS (CHECK AGAIN THINK WE MIGHT HAVE SUBTLE BUGS)
	 */
	if ((temp > 0xFFFF) || (temp < 0))
	{
		setbit(_STS, _C, 0);
		if ((oldreg & 0x8000) && (data & 0x8000) && !(temp & 0x8000))
		{
			setbit(_STS, _Q, 1);
		}
		else
		{
			setbit(_STS, _Q, 0);
		}
	}
	else
	{
		setbit(_STS, _C, 1);
		if (!(oldreg & 0x8000) && !(data & 0x8000) && (temp & 0x8000))
		{
			setbit(_STS, _Q, 1);
		}
		else
		{
			setbit(_STS, _Q, 0);
		}
	}

	gA = (temp & 0xFFFF);
}

/*
 * RDIV
 */
void rdiv_org(ushort instr)
{
	sshort divider;
	int dividend;
	div_t result3; /* stdlib.h */
	/* :TODO: Apparently Carry can be set too. CHECK that... Might be RAD=1??? */
	/* Overflow and division with zero also need to be fixed!! */
	/* :NOTE: The way it is described in the manual, we assume this is a fraction (numerator/denominator and return a quotient and remainder as per manual */
	divider = ((instr & 0x0038) >> 3) ? (sshort)gReg->reg[gPIL][((instr & 0x0038) >> 3)] : 0;

	if (divider == 0)
	{
		// Division by zero
		setbit(_STS, _Z, 1);
		return;
	}

	dividend = ((int)gA << 16) | gD;
	result3 = div(dividend, divider);
	gA = result3.quot;
	gD = result3.rem;
}

/// <summary>
/// RDIV - Integer inter-register divide
/// AD/<sr> —> A<- (Quotient) and D<- (Remainder)
///
/// Format: RDIV<sr>
///
/// Code: 141 600
///
/// The 32 bit signed integer contained in the double accumulator AD is divided by the contents of the register in the<sr> fieid, with the quotient in the A register
/// and the remainder in the D register, i.e., AD/sr = A< (quotient) and D<(remainder).
/// The sign of the remainder is always equal to the sign of the dividend (AD). The destination field of the instruction is not used.
///
/// If the division causes overflow, the error indicator Z is set to one.
/// The numbers are considered as fixed point integers with the fixed point after the rightmost position.
///
/// Divide double accumulator with source register.Quotient in A, remainder in D (AD= A*(sr)+ D)
/// A:= AD/(sr)

/// Affected: (A), (D), Z,C, O, Q
/// </summary>
void rdiv(ushort instr)
{
	int dividend = ((int)gA << 16) | gD;
	short divisor = ((instr & 0x0038) >> 3) ? (short)gReg->reg[gPIL][((instr & 0x0038) >> 3)] : 0;

	if (divisor == 0)
	{
		// Division by zero - set error
		setbit(_STS, _Z, 1);
		// TODO: check for Z error
		return;
	}

	int quotient = dividend / divisor;

	int reminder = dividend - (quotient * divisor);

	// Check for carry (ie, value is bigger than 16 bits)
	setbit(_STS, _C, ((quotient & 0xFFFF0000) != 0));

	if (abs(quotient) >= 32768)
	{
		setbit(_STS, _Z, 1);
		return;
	}
	gA = quotient;
	gD = reminder;
	;
}

/*
 * RMPY
 */
void rmpy_org(ushort instr)
{
	/* :TODO: Apparently Carry can be set too. CHECK that... Might be RAD=1??? */
	int a, b, result;
	a = ((instr & 0x0038) >> 3) ? (int)gReg->reg[gPIL][((instr & 0x0038) >> 3)] : 0;
	b = (instr & 0x0007) ? (int)gReg->reg[gPIL][(instr & 0x0007)] : 0;
	result = a * b;
	if (abs(result) > INT_MAX)
	{ /* Set O and Q */
		setbit(_STS, _Q, 1);
		setbit(_STS, _O, 1);
	}
	else
	{
		; //: TODO: Carry???;
		setbit(_STS, _Q, 0);
		setbit(_STS, _O, 0);
	}
	gA = (sshort)((result & 0xffff0000) >> 16);
	gD = (sshort)(result & 0x0000ffff);
}

/// <summary>
/// RMPY - Integer inter-register multiply
/// AD <- dr * sr
///
/// Format: RMPY<sr><dr>
///
/// Code: 141 200
///
/// The <sr> and <dr> fields are used to specify the two operands to be mutiplied (represented as two's complement integers), the codes are the same as for ROP.
/// The result is a 32 bit signed integer which will be placed in the A and D registers with the 16 most significant bits in the A register and the 16 least significant bits in the D register.
///
/// Multiply source with destination.Result in double accumulator
/// AD: = (sr)*(dr)
///
/// Affected: (A),(D), C,O,Q
/// </summary>
void rmpy(ushort instr)
{
	int minusCnt = 0;
	short source_value = (short)((instr & 0x0038) >> 3) ? (short)gReg->reg[gPIL][((instr & 0x0038) >> 3)] : 0;
	short dest_value = (short)(instr & 0x0007) ? (short)gReg->reg[gPIL][(instr & 0x0007)] : 0;

	// Below logic multiply-logic is 100% correct and verified against ND-100 microcode
	if ((source_value & (1 << 15)) != 0)
	{
		source_value *= -1;
		minusCnt++;
	}

	if ((dest_value & (1 << 15)) != 0)
	{
		dest_value *= -1;
		minusCnt++;
	}

	int result = source_value * dest_value;


	if (abs(result) > INT_MAX)
	{
		// Set O and Q
		setbit(_STS, _Q, 1);
		setbit(_STS, _O, 1);
	}
	else
	{
		setbit(_STS, _Q, 0);
		//setbit(_STS, _O, 0); NO!
	}

	// Check for carry (ie, value is bigger than 16 bits)
	setbit(_STS, _C, ((result & 0xFFFF0000) != 0));

	if (minusCnt == 1)
	{
		gA = (ushort)(((short)((result >> 16) & 0xFFFF) * -1) & 0x3FF);
		gD = (ushort)((short)(result & 0xFFFF) * -1);
	}
	else
	{
		// set A and D registers
		gA = (ushort)((result >> 16) & 0xFFFF);
		gD = (ushort)(result & 0xFFFF);
	}
}

/*
 * MPY
 */
void mpy(ushort operand)
{
	int a, b, result;
	a = (sshort)gA;

	gEA = New_GetEffectiveAddr(operand, &gUseAPT);
	ushort mem = MemoryRead(gEA, gUseAPT);
	b = (sshort)mem;

	setbit(_STS, _Q, 0);

	result = a * b;

	if (abs(result) > 32767)
	{ /* Set O and Q */
		setbit(_STS, _Q, 1);
		setbit(_STS, _O, 1);
	}
	gA = (sshort)result;
}



/************************ BCD instructions *************************/

/* BCD registers and helper functions */
ushort D1 = 0;
ushort D2 = 0;

void GetBCD(ushort address)
{
	D1 = MemoryRead(address, true);
	D2 = MemoryRead((address + 1) & 0xFFFF, true);
}

void StoreBCD(ushort address)
{
	MemoryWrite(address, D1, true, WRITEMODE_WORD);
	MemoryWrite((address + 1) & 0xFFFF, D2, true, WRITEMODE_WORD);
}

/* ADDD  */
void ndfunc_addd(ushort instr)
{
}

/* SUBD  */
void ndfunc_subd(ushort instr)
{
}

/* COMD  */
void ndfunc_comd(ushort instr)
{
}

/* PACK  */
void ndfunc_pack(ushort instr)
{
}

/* UPACK */
void ndfunc_unpack(ushort instr)
{
}

/* SHDE  */
void ndfunc_shde(ushort instr)
{
}


/*************************** INITIALIZATION ***************************/

void Instruction_Add(int opcode, void *funcpointer)
{
	if (instr_funcs[opcode] != NULL)
	{
		printf("Warning: Overwriting instruction %06o\n", opcode);
	}

	instr_funcs[opcode] = funcpointer;
}

void Instruction_Add_Range(int start, int stop, void *funcpointer)
{
	int i;
	for (i = start; i <= stop; i++)
	{
		if (instr_funcs[i] != NULL)
		{
			printf("Warning: Overwriting instruction %06o\n",i);
		}

		instr_funcs[i] = funcpointer;
	}
	return;
}

// For debugging purposes
void print_mask_binary(unsigned short mask) {
    for (int i = 15; i >= 0; i--) {
        putchar((mask & (1 << i)) ? '1' : '0');
        if (i % 4 == 0 && i != 0) {
            putchar('_');
        }
    }
}

void Instruction_Add_Mask(int opcode, int mask, void *funcpointer)
{
	int i;
	int signature = opcode & mask;

	//printf("Instruction %06o should have mask ", opcode);
    //print_mask_binary(mask);
    //putchar('\n');

	for (i = opcode; i <= 0xFFFF; i++)
	{
		if ((i & mask) == signature)
		{
			if (instr_funcs[i] != NULL)
			{
				printf("Warning: Overwriting instruction %06o with %06o\n", i, opcode);
			}

			instr_funcs[i] = funcpointer;
		}
	}
	return;
}

/*
 * Add IO handler addresses in this function
 * This also thus actually acts as the new instruction parser also.
 */
void Setup_Instructions()
{
	//Instruction_Add_Range(0000000, 0177777, &illegal_instr); /* First make all instructions by default point to illegal_instr  */

	// Instruction_Add_Range(0000000, 0003777, &ndfunc_stz); /* STZ  */
	Instruction_Add_Mask(0000000, 0xF800, &ndfunc_stz);

	// Instruction_Add_Range(0004000, 0007777, &ndfunc_sta); /* STA  */
	Instruction_Add_Mask(0004000, 0xF800, &ndfunc_sta);

	// Instruction_Add_Range(0010000, 0013777, &ndfunc_stt); /* STT  */
	Instruction_Add_Mask(0010000, 0xF800, &ndfunc_stt);

	// Instruction_Add_Range(0014000, 0017777, &ndfunc_stx); /* STX  */
	Instruction_Add_Mask(0014000, 0xF800, &ndfunc_stx);

	// Instruction_Add_Range(0020000, 0023777, &ndfunc_std); /* STD  */
	Instruction_Add_Mask(0020000, 0xF800, &ndfunc_std);

	// Instruction_Add_Range(0024000, 0027777, &ndfunc_ldd); /* LDD  */
	Instruction_Add_Mask(0024000, 0xF800, &ndfunc_ldd);

	// Instruction_Add_Range(0030000, 0033777, &ndfunc_stf); /* STF  */
	Instruction_Add_Mask(0030000, 0xF800, &ndfunc_stf);

	// Instruction_Add_Range(0034000, 0037777, &ndfunc_ldf); /* LDF  */
	Instruction_Add_Mask(0034000, 0xF800, &ndfunc_ldf);

	// Instruction_Add_Range(0040000, 0043777, &ndfunc_min); /* MIN  */
	Instruction_Add_Mask(0040000, 0xF800, &ndfunc_min);

	// Instruction_Add_Range(0044000, 0047777, &ndfunc_lda); /* LDA  */
	Instruction_Add_Mask(0044000, 0xF800, &ndfunc_lda);

	// Instruction_Add_Range(0050000, 0053777, &ndfunc_ldt); /* LDT  */
	Instruction_Add_Mask(0050000, 0xF800, &ndfunc_ldt);

	// Instruction_Add_Range(0054000, 0057777, &ndfunc_ldx); /* LDX  */
	Instruction_Add_Mask(0054000, 0xF800, &ndfunc_ldx);

	// Instruction_Add_Range(0060000, 0063777, &ndfunc_add); /* ADD  */
	Instruction_Add_Mask(0060000, 0xF800, &ndfunc_add);

	// Instruction_Add_Range(0064000, 0067777, &ndfunc_sub); /* SUB  */
	Instruction_Add_Mask(0064000, 0xF800, &ndfunc_sub);

	// Instruction_Add(0070000, 0073777, &ndfunc_and); /* AND  */
	Instruction_Add_Mask(0070000, 0xF800, &ndfunc_and);

	// Instruction_Add_Range(0074000, 0077777, &ndfunc_ora); /* ORA  */
	Instruction_Add_Mask(0074000, 0xF800, &ndfunc_ora);

	// Instruction_Add_Range(0100000, 0103777, &ndfunc_fad); /* FAD  */
	Instruction_Add_Mask(0100000, 0xF800, &ndfunc_fad);

	// Instruction_Add_Range(0104000, 0107777, &ndfunc_fsb); /* FSB  */
	Instruction_Add_Mask(0104000, 0xF800, &ndfunc_fsb);

	// Instruction_Add_Range(0110000, 0113777, &ndfunc_fmu); /* FMU  */
	Instruction_Add_Mask(0110000, 0xF800, &ndfunc_fmu);

	// Instruction_Add_Range(0114000, 0117777, &ndfunc_fdv); /* FDV  */
	Instruction_Add_Mask(0114000, 0xF800, &ndfunc_fdv);

	// Instruction_Add_Range(0120000, 0123777, &mpy);		/* MPY  */
	Instruction_Add_Mask(0120000, 0xF800, &mpy);

	// Instruction_Add_Range(0124000, 0127777, &ndfunc_jmp); /* JMP  */
	Instruction_Add_Mask(0124000, 0xF800, &ndfunc_jmp);

	// Instruction_Add_Range(0134000, 0137777, &ndfunc_jpl); /* JPL  */
	Instruction_Add_Mask(0134000, 0xF800, &ndfunc_jpl);

	// CJPs - Conditional jumps
	// Instruction_Add_Range(0130000, 0130377, &ndfunc_jap); /* JAP */
	Instruction_Add_Mask(0130000, 0xFF00, &ndfunc_jap);

	// Instruction_Add_Range(0130400, 0130777, &ndfunc_jan); /* JAN */
	Instruction_Add_Mask(0130400, 0xFF00, &ndfunc_jan);

	// Instruction_Add_Range(0131000, 0131377, &ndfunc_jaz); /* JAZ */
	Instruction_Add_Mask(0131000, 0xFF00, &ndfunc_jaz);

	// Instruction_Add_Range(0131400, 0131777, &ndfunc_jaf); /* JAF */
	Instruction_Add_Mask(0131400, 0xFF00, &ndfunc_jaf);

	// Instruction_Add_Range(0132000, 0132377, &ndfunc_jpc); /* JPC */
	Instruction_Add_Mask(0132000, 0xFF00, &ndfunc_jpc);

	// Instruction_Add_Range(0132400, 0132777, &ndfunc_jnc); /* JNC */
	Instruction_Add_Mask(0132400, 0xFF00, &ndfunc_jnc);

	// Instruction_Add_Range(0133000, 0133377, &ndfunc_jxz); /* JXZ */
	Instruction_Add_Mask(0133000, 0xFF00, &ndfunc_jxz);

	// Instruction_Add_Range(0133400, 0133777, &ndfunc_jxn); /* JXN */
	Instruction_Add_Mask(0133400, 0xFF00, &ndfunc_jxn);

	// Instruction_Add(0140000, 0143777, &ndfunc_skp);
	Instruction_Add_Mask(0140000, 0xF8C0, &ndfunc_skp);

	// BCD (CX)
	Instruction_Add(0140120, &ndfunc_addd);	  /* ADDD  */
	Instruction_Add(0140121, &ndfunc_subd);	  /* SUBD  */
	Instruction_Add(0140122, &ndfunc_comd);	  /* COMD  */
	Instruction_Add(0140124, &ndfunc_pack);	  /* PACK  */
	Instruction_Add(0140125, &ndfunc_unpack); /* UPACK */
	Instruction_Add(0140126, &ndfunc_shde);	  /* SHDE  */

	Instruction_Add(0140123, &DoTSET); /* TSET  */
	Instruction_Add(0140127, &DoRDUS); /* RDUS  */

	{ // CE; CX

		Instruction_Add(0140130, &ndfunc_bfill); /* BFILL */
		Instruction_Add(0140131, &DoMOVB);		 /* MOVB  */
		Instruction_Add(0140132, &DoMOVBF);		 /* MOVBF */

		// Instruction_Add(0140131, &ndfunc_movb);  /* MOVB  */
		// Instruction_Add(0140132, &ndfunc_movbf); /* MOVBF */
	}

	switch (CurrentCPUType)
	{
	case ND110:
	case ND110CE:
	case ND110CX:
	case ND110PCX:
		Instruction_Add(0140133, &ndfunc_versn); /* VERSN - ND110+ */
		break;
	default:
		break;
	}

	{ // CE; CX

		Instruction_Add(0140134, &ndfunc_init);	 /* INIT  */
		Instruction_Add(0140135, &ndfunc_entr);	 /* ENTR  */
		Instruction_Add(0140136, &ndfunc_leave); /* LEAVE */
		Instruction_Add(0140137, &ndfunc_eleav); /* ELEAV */
	}
	// Instruction_Add(0140200, 0140277, &illegal_instr); /* USER1 (microcode defined by user or illegal instruction otherwise) */

	switch (CurrentCPUType)
	{
	case ND110:
	case ND110CE:
	case ND110CX:
	case ND110PCX:
		// ALL are priveleged!
		Instruction_Add(0140500, &unimplemented_instr); /* WGLOB - ND110 Specific */
		Instruction_Add(0140501, &unimplemented_instr); /* RGLOB - ND110 Specific */
		Instruction_Add(0140502, &unimplemented_instr); /* INSPL - ND110 Specific */
		Instruction_Add(0140503, &unimplemented_instr); /* REMPL - ND110 Specific */
		Instruction_Add(0140504, &unimplemented_instr); /* CNREK - ND110 Specific */
		Instruction_Add(0140505, &unimplemented_instr); /* CLPT  - ND110 Specific */
		Instruction_Add(0140506, &unimplemented_instr); /* ENPT  - ND110 Specific */
		Instruction_Add(0140507, &unimplemented_instr); /* REPT  - ND110 Specific */
		Instruction_Add(0140510, &unimplemented_instr); /* LBIT  - ND110 Specific */

		Instruction_Add(0140513, &unimplemented_instr); /* SBITP - ND110 Specific */
		Instruction_Add(0140514, &unimplemented_instr); /* LBYTP - ND110 Specific */
		Instruction_Add(0140515, &unimplemented_instr); /* SBYTP - ND110 Specific */
		Instruction_Add(0140516, &unimplemented_instr); /* TSETP - ND110 Specific */
		Instruction_Add(0140517, &unimplemented_instr); /* RDUSP - ND110 Specific */

		break;
	default:
		// Instruction_Add_Range(0140500, 0140577, &illegal_instr); /* USER2 (microcode defined by user or illegal instruction otherwise) */
		break;
	}

	Instruction_Add_Mask(0140600, 0xFFC0, &DoEXR); /* EXR */
	switch (CurrentCPUType)
	{
	case ND110:
	case ND110CE:
	case ND110CX:
	case ND110PCX:
		// ALL are priveleged!
		Instruction_Add(0140700, &unimplemented_instr); /* LASB - ND110 Specific */
		Instruction_Add(0140701, &unimplemented_instr); /* SASB - ND110 Specific */
		Instruction_Add(0140702, &unimplemented_instr); /* LACB - ND110 Specific */
		Instruction_Add(0140703, &unimplemented_instr); /* SASB - ND110 Specific */
		Instruction_Add(0140704, &unimplemented_instr); /* LXSB - ND110 Specific */
		Instruction_Add(0140705, &unimplemented_instr); /* LXCB - ND110 Specific */
		Instruction_Add(0140706, &unimplemented_instr); /* SZSB - ND110 Specific */
		Instruction_Add(0140707, &unimplemented_instr); /* SZCB - ND110 Specific */
		break;
	default:
		break;
	}

	if (true)
	{
		// ND100-CX and ND110-CX only

		Instruction_Add(0140300, &ndfunc_setpt);		 /* SETPT */
		Instruction_Add(0140301, &ndfunc_clept);		 /* CLEPT */
		Instruction_Add(0140302, &ndfunc_clnreent);		 /* CLNREENT */
		Instruction_Add(0140303, &ndfunc_chreent_pages); /* CHREENT-PAGES */
		Instruction_Add(0140304, &ndfunc_clepu);		 /* CLEPU */
	}
	Instruction_Add_Mask(0141200, 0xFFC0, &rmpy);		 /* RMPY */
	Instruction_Add_Mask(0141600, 0xFFC0, &rdiv);		 /* RDIV */
	Instruction_Add_Mask(0142200, 0xFFC0, &ndfunc_lbyt); /* LBYT */
	Instruction_Add_Mask(0142600, 0xFFC0, &ndfunc_sbyt); /* SBYT */

	// CX instructions
	Instruction_Add(0142700, &ndfunc_geco);				 /* GECO - Undocumented instruction */
	Instruction_Add_Mask(0143100, 0xFFC0, &DoMOVEW);	 /* MOVEW */
	Instruction_Add_Mask(0143200, 0xFFC0, &ndfunc_mix3); /* MIX3 */

	Instruction_Add_Mask(0143300, 0xFFC7, &ndfunc_ldatx); /* LDATX */
	Instruction_Add_Mask(0143301, 0xFFC7, &ndfunc_ldxtx); /* LDXTX */
	Instruction_Add_Mask(0143302, 0xFFC7, &ndfunc_lddtx); /* LDDTX */
	Instruction_Add_Mask(0143303, 0xFFC7, &ndfunc_ldbtx); /* LDBTX */
	Instruction_Add_Mask(0143304, 0xFFC7, &ndfunc_statx); /* STATX */
	Instruction_Add_Mask(0143305, 0xFFC7, &ndfunc_stztx); /* STZTX */
	Instruction_Add_Mask(0143306, 0xFFC7, &ndfunc_stdtx); /* STDTX */

	Instruction_Add(0143500, &ndfunc_lwcs); /* LWCS */

	Instruction_Add(0143604, &ndfunc_ident); /* IDENT PL10 */
	Instruction_Add(0143611, &ndfunc_ident); /* IDENT PL11 */
	Instruction_Add(0143622, &ndfunc_ident); /* IDENT PL12 */
	Instruction_Add(0143643, &ndfunc_ident); /* IDENT PL13 */

	Instruction_Add_Range(0144000, 0147777, &regop); /* --ROPS-- */
	Instruction_Add_Mask(0150000, 0xFFF0, &DoTRA);	 /* TRA */
	Instruction_Add_Mask(0150100, 0xFFF0, &DoTRR);	 /* TRR */
	Instruction_Add_Mask(0150200, 0xFFF0, &DoMCL);	 /* MCL */
	Instruction_Add_Mask(0150300, 0xFFF0, &DoMST);	 /* MST */
	Instruction_Add(0150400, &ndfunc_opcom);		 /* OPCOM */
	Instruction_Add(0150401, &ndfunc_iof);			 /* IOF */
	Instruction_Add(0150402, &ndfunc_ion);			 /* ION */
	switch (CurrentCPUType)
	{
	case ND110PCX:
		/* ND110 Butterfly only instruction */
		Instruction_Add(0150403, &unimplemented_instr); /* RTNSIM (SECRE) */
		break;
	default:
		break;
	}
	Instruction_Add(0150404, &ndfunc_pof);	/* POF */
	Instruction_Add(0150405, &ndfunc_piof); /* PIOF */
	Instruction_Add(0150406, &ndfunc_sex);	/* SEX */
	Instruction_Add(0150407, &ndfunc_rex);	/* REX */
	Instruction_Add(0150410, &ndfunc_pon);	/* PON */
	Instruction_Add(0150412, &ndfunc_pion); /* PION */

	Instruction_Add(0150415, &ndfunc_ioxt); /* IOXT */
	Instruction_Add(0150416, &ndfunc_exam); /* EXAM */
	Instruction_Add(0150417, &ndfunc_depo); /* DEPO */

	Instruction_Add_Mask(0151000, 0xFF00, &DoWAIT);		/* WAIT - Range 151000 - 151377 */
	Instruction_Add_Mask(0151400, 0xFF00, &ndfunc_nlz); /* NLZ */
	Instruction_Add_Mask(0152000, 0xFF00, &ndfunc_dnz); /* DNZ */
	Instruction_Add_Mask(0152402, 0xFF07, &ndfunc_srb); /* SRB */
	Instruction_Add_Mask(0152600, 0xFF07, &ndfunc_lrb); /* LRB */
	Instruction_Add_Mask(0153000, 0xFF00, &ndfunc_mon); /* MON  - Range 153000-153377  */
	Instruction_Add_Mask(0153400, 0xFF80, &ndfunc_irw); /* IRW */
	Instruction_Add_Mask(0153600, 0xFF80, &ndfunc_irr); /* IRR */

	// Instruction_Add_Range(0154000, 0157777, &ndfunc_shifts); /* SHT, SHD, SHA, SAD */  /* NOTE: this is actually a ND1 instruction, so need to check which NDs implement it later */
	Instruction_Add_Mask(0154000, 0x7980, &ndfunc_shifts); // SHT
	Instruction_Add_Mask(0154200, 0x7980, &ndfunc_shifts); // SHD
	Instruction_Add_Mask(0154400, 0x7980, &ndfunc_shifts); // SHA
	Instruction_Add_Mask(0154600, 0x7980, &ndfunc_shifts); // SAD

	// IOT Range 0160000 - 0163777
	Instruction_Add_Mask(0160000, 0xF800, &ndfunc_iot); /* IOT  - ND1 specific, but exists on all CPU's*/

	Instruction_Add_Mask(0164000, 0xF800, &ndfunc_iox); /* IOX */	

	Instruction_Add_Mask(0170000, 0xFF00, &ndfunc_sab); /* SAB */
	Instruction_Add_Mask(0170400, 0xFF00, &ndfunc_saa); /* SAA */
	Instruction_Add_Mask(0171000, 0xFF00, &ndfunc_sat); /* SAT */
	Instruction_Add_Mask(0171400, 0xFF00, &ndfunc_sax); /* SAX */
	Instruction_Add_Mask(0172000, 0xFF00, &ndfunc_aab); /* AAB */
	Instruction_Add_Mask(0172400, 0xFF00, &ndfunc_aaa); /* AAA */
	Instruction_Add_Mask(0173000, 0xFF00, &ndfunc_aat); /* AAT */
	Instruction_Add_Mask(0173400, 0xFF00, &ndfunc_aax); /* AAX */

	Instruction_Add_Range(0174000, 0177777, &do_bops); /* Bit Operation Instructions */
													   /* Bit operations, 16 of them, 4 BSET,4 BSKP and 8 others */
}
