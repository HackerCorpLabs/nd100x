/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2008 Zdravko
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

/*
 * 48-bit floating point arithmetic for the ND-100.
 *
 * Reimplemented using pure integer arithmetic (ported from the SIMH
 * ND100 simulator) instead of the previous long double approach.
 * This gives bit-exact results matching the real hardware.
 *
 * ND-100 48-bit float format (3 x 16-bit words):
 *   Word 0 (T register): bit 15 = sign, bits 14-0 = exponent (biased 16384)
 *   Word 1 (A register): upper 16 bits of mantissa
 *   Word 2 (D register): lower 16 bits of mantissa
 *   Mantissa is normalized: 0.5 <= |mantissa| < 1.0
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "cpu_types.h"
#include "cpu_protos.h"

int NDFloat_Div(unsigned short int *p_a, unsigned short int *p_b, unsigned short int *p_r);
int NDFloat_Mul(unsigned short int *p_a, unsigned short int *p_b, unsigned short int *p_r);
int NDFloat_Add(unsigned short int *p_a, unsigned short int *p_b, unsigned short int *p_r);
int NDFloat_Sub(unsigned short int *p_a, unsigned short int *p_b, unsigned short int *p_r);

extern struct CpuRegs *gReg;
void DoNLZ(char scaling);
void DoDNZ(char scaling);
extern void setbit(ushort regnum, ushort stsbit, char val);

/*
 * Internal floating point representation.
 * Used to unpack/repack the 48-bit ND format for arithmetic.
 */
struct fp {
	int s;          /* sign: 0 = positive, 1 = negative */
	int e;          /* exponent (unbiased) */
	uint64_t m;     /* mantissa in upper 32 bits of a 64-bit value (for mul/div) */
};

/*
 * Unpack three 16-bit words into the internal fp struct.
 * The mantissa is placed in the lower 32 bits (for add/sub)
 * or can be shifted as needed (mul/div do their own shifting).
 */
static void
mkfp48(struct fp *fp, ushort w1, ushort w2, ushort w3)
{
	fp->s = (w1 >> 15) & 1;
	fp->e = (w1 & 0x7FFF) - 16384;
	fp->m = ((uint64_t)w2 << 16) + (uint64_t)w3;
}

/*
 * add48 - Add two 48-bit floating point numbers with same sign.
 *
 * Result is written to the output array r[3].
 */
static void
add48(struct fp *f1, struct fp *f2, ushort *r)
{
	struct fp *ft;
	uint64_t m3;
	int scale, gbit;

	/* Ensure f1 has the larger exponent */
	if (f2->e > f1->e) {
		ft = f1; f1 = f2; f2 = ft;
	}

	if ((scale = f1->e - f2->e) > 31) {
		m3 = f1->m;
		goto done;
	}

	/* get shifted out guard bit */
	gbit = scale ? (((1LL << scale) - 1) & f2->m) != 0 : 0;
	f2->m >>= scale;
	m3 = (f1->m + f2->m) | gbit;
	if (m3 > 0xffffffffLL) {
		m3 >>= 1;
		f1->e++;
	}

done:
	r[0] = (f1->e + 16384) | (f1->s << 15);
	r[1] = (ushort)(m3 >> 16);
	r[2] = (ushort)m3;
}

/*
 * sub48 - Subtract two 48-bit floating point numbers with different signs.
 *
 * Result is written to the output array r[3].
 */
static void
sub48(struct fp *f1, struct fp *f2, ushort *r)
{
	struct fp *ft;
	uint64_t m3;
	int scale, gbit;

	/* Ensure f1 has the larger exponent */
	if (f2->e > f1->e) {
		ft = f1; f1 = f2; f2 = ft;
	}

	if ((scale = f1->e - f2->e) > 31) {
		m3 = f1->m;
		goto done;
	}

	/* get shifted out sticky bit */
	gbit = scale ? (((1LL << scale) - 1) & f2->m) != 0 : 0;
	f2->m >>= scale;
	f2->e = f1->e;

	/* check for swap of mantissa */
	if (f2->m > f1->m) {
		ft = f1; f1 = f2; f2 = ft;
	}
	m3 = (f1->m - f2->m) | gbit;

	if (m3 == 0) {
		r[0] = r[1] = r[2] = 0;
		return;
	}

	/* normalize */
	while ((m3 & 0x80000000LL) == 0) {
		m3 <<= 1;
		f1->e--;
	}

done:
	r[0] = (f1->e + 16384) | (f1->s << 15);
	r[1] = (ushort)(m3 >> 16);
	r[2] = (ushort)m3;
}

/*
 * NDFloat_Add - Add two 48-bit floating point numbers.
 *
 * The contents of the effective location and the two following locations
 * are added to the floating accumulator with the result in the floating
 * accumulator.
 */
int NDFloat_Add(ushort *p_a, ushort *p_b, ushort *p_r)
{
	struct fp f1, f2;

	mkfp48(&f1, p_a[0], p_a[1], p_a[2]);
	mkfp48(&f2, p_b[0], p_b[1], p_b[2]);

	if (f1.s ^ f2.s)
		sub48(&f1, &f2, p_r);
	else
		add48(&f1, &f2, p_r);
	return 0;
}

/*
 * NDFloat_Sub - Subtract two 48-bit floating point numbers.
 *
 * The contents of the effective location and the two following locations
 * are subtracted from the floating accumulator with the result
 * in the floating accumulator.
 */
int NDFloat_Sub(ushort *p_a, ushort *p_b, ushort *p_r)
{
	struct fp f1, f2;

	mkfp48(&f1, p_a[0], p_a[1], p_a[2]);
	mkfp48(&f2, p_b[0], p_b[1], p_b[2]);

	/* swap sign of the second operand (b) for subtraction */
	f2.s ^= 1;

	if (f1.s ^ f2.s)
		sub48(&f1, &f2, p_r);
	else
		add48(&f1, &f2, p_r);
	return 0;
}

/*
 * NDFloat_Mul - Multiply two 48-bit floating point numbers.
 *
 * The contents of the floating accumulator are multiplied with the
 * number at the effective floating word locations with the result in
 * the floating accumulator.
 */
int NDFloat_Mul(ushort *p_a, ushort *p_b, ushort *p_r)
{
	struct fp f1, f2;
	int s3, e3;
	uint64_t m3;

	mkfp48(&f1, p_a[0], p_a[1], p_a[2]);
	mkfp48(&f2, p_b[0], p_b[1], p_b[2]);

	/* calc */
	m3 = f1.m * f2.m;
	e3 = f1.e + f2.e;
	s3 = f1.s ^ f2.s;

	/* normalize (if needed) */
	if ((m3 & (1ULL << 63)) == 0) {
		m3 <<= 1;
		e3--;
	}

	/* store result */
	p_r[1] = (ushort)(m3 >> 48);
	p_r[2] = (ushort)(m3 >> 32);
	p_r[0] = (e3 + 16384) | (s3 << 15);
	if (m3 == 0 || e3 < -16383)
		p_r[0] = p_r[1] = p_r[2] = 0;
	return 0;
}

/*
 * NDFloat_Div - Divide two 48-bit floating point numbers.
 *
 * The contents of the floating accumulator (p_a) are divided by the number
 * at the effective floating word locations (p_b). Result in p_r.
 * If division by zero is attempted, the error indicator Z is set.
 */
int NDFloat_Div(ushort *p_a, ushort *p_b, ushort *p_r)
{
	struct fp f1, f2;
	int s3, e3;
	uint64_t m3;

	/* f1 = divisor (from memory, p_b) */
	mkfp48(&f1, p_b[0], p_b[1], p_b[2]);

	/* f2 = dividend (from registers, p_a) */
	mkfp48(&f2, p_a[0], p_a[1], p_a[2]);
	f2.m <<= 32;

	/* Division by zero check */
	if (f1.m == 0) {
		p_r[0] = p_a[0] | 0x7FFF;
		p_r[1] = 0xFFFF;
		p_r[2] = 0xFFFF;
		return 1; /* division by zero */
	}

	/* calc */
	s3 = f1.s ^ f2.s;
	e3 = f2.e - f1.e;
	m3 = f2.m / f1.m;
	if (f2.m % f1.m) /* "guard" bit */
		m3++;

	/* normalize (if needed) */
	if (m3 >= (1ULL << 32)) {
		m3 >>= 1;
		e3++;
	}

	/* store result */
	p_r[1] = (ushort)(m3 >> 16);
	p_r[2] = (ushort)m3;
	p_r[0] = (e3 + 16384) | (s3 << 15);
	if (f2.m == 0 || e3 < -16383)
		p_r[0] = p_r[1] = p_r[2] = 0;
	return 0;
}

/*
 * DoNLZ - Normalize (integer to floating point).
 *
 * Converts the number in the A register to a standard form floating
 * number in the floating accumulator {T,A,D}, using the scaling factor.
 * For integers, the scaling factor should be +16.
 * Because of the single precision fixed point number, the D register
 * will be cleared.
 */
void DoNLZ(char scaling)
{
	int sh, s;
	int val;

	s = 0;
	gD = 0;
	if (gA == 0) { /* zero, special case */
		gT = 0;
		return;
	}

	val = (int)(sshort)gA;
	sh = 16384 + (int)(signed char)scaling;
	if (val < 0) {
		val = -val;
		s = 0x8000;
	}
	if (val > 32767) {
		val >>= 1;
		sh++;
	}
	while ((val & 0x8000) == 0) {
		val <<= 1;
		sh--;
	}
	gT = sh + s;
	gA = (ushort)val;
}

/*
 * DoDNZ - Denormalize (floating point to integer).
 *
 * Converts the floating number in the floating accumulator to a
 * single precision fixed point number in the A register, using the
 * scaling factor.
 *
 * When converting to integers, the scaling factor should be -16 (0xF0).
 * If the conversion causes underflow, T, A, D are all set to zero.
 * If overflow occurs, the error indicator Z is set.
 * Negative numbers are converted to positive before conversion,
 * then the result is negated.
 */
void DoDNZ(char scaling)
{
	int32_t val = 0;
	int sh;

	sh = (gT & 0x7FFF) - 16384 + (int)(signed char)scaling;
	if (sh < 0) {
		val = gA;
		val >>= -sh;
	} else if (sh > 0) {
		val = gA;
		val <<= sh;
		if (val > 32767)
			setbit(_STS, _Z, 1);
	}

	if (gT & 0x8000)
		val = -val;
	gT = 0;
	gD = 0;
	gA = (ushort)val;
}
