/*
 * nd100x - ND100 Virtual Machine
 *
 * Copyright (c) 2025 Ronny Hansen
 *
 * BCD (Binary Coded Decimal) arithmetic for the ND-100 Commercial Extended
 * (CE) instruction set: ADDD, SUBD, COMD, SHDE, PACK, UPACK.
 *
 * Approach: Convert BCD from memory to double, do arithmetic, convert back.
 * This matches the proven C# RetroCore implementation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#include "cpu_types.h"
#include "cpu_protos.h"

extern struct CpuRegs *gReg;
extern void setbit(ushort regnum, ushort stsbit, char val);

/* Max BCD field length in nibbles */
#define BCD_MAX_NIBBLES 32
#define BCD_MAX_WORDS   8

/* BCD sign nibble values */
#define BCD_SIGN_POS 0x0C
#define BCD_SIGN_NEG 0x0D
#define BCD_SIGN_UNS 0x0F

/* Internal BCD operand */
typedef struct {
    ushort addr;            /* D1: word address */
    bool right_byte;        /* D2 bit 15 */
    int sign_format;        /* D2 bits 13-11 */
    bool rounding;          /* D2 bit 10 */
    int decimal_point;      /* D2 bits 9-5 */
    int field_length;       /* D2 bits 4-0 */
    ushort words[BCD_MAX_WORDS]; /* raw memory words */
    int num_words;
    double value;           /* numeric value */
    bool is_negative;
    bool error;
    /* BCD digit string: digits + sign char, null-terminated */
    char bcd_str[BCD_MAX_NIBBLES + 2];
} BcdOperand;

/* ================================================================ */
/* Parse descriptor from register values                            */
/* ================================================================ */
static void bcd_parse(BcdOperand *op, ushort d1, ushort d2)
{
    memset(op, 0, sizeof(*op));
    op->addr = d1;
    op->right_byte = (d2 >> 15) & 1;
    op->sign_format = (d2 >> 11) & 0x07;
    op->rounding = (d2 >> 10) & 1;
    op->decimal_point = (d2 >> 5) & 0x1F;
    op->field_length = d2 & 0x1F;
    if (op->field_length > BCD_MAX_NIBBLES)
        op->field_length = BCD_MAX_NIBBLES;
}

/* ================================================================ */
/* Load BCD operand from memory, convert to double                  */
/* Matches C# LoadOperand + ConcatenateBCD + ConvertBCDToDouble     */
/* ================================================================ */
static void bcd_load(BcdOperand *op, bool is_ascii)
{
    if (op->field_length == 0) {
        op->value = 0.0;
        return;
    }

    /* Validate decimal_point vs field_length */
    if (op->decimal_point >= op->field_length && op->decimal_point != 0) {
        op->error = true;
        return;
    }

    /* Calculate words to read */
    int mem_len;
    if (!is_ascii) {
        /* BCD: 4 nibbles per word */
        mem_len = op->field_length >> 2;
        if ((mem_len << 2) != op->field_length)
            mem_len++;
    } else {
        /* ASCII: 2 bytes per word */
        mem_len = op->field_length >> 1;
    }
    if (mem_len == 0) mem_len = 1;
    if (mem_len > BCD_MAX_WORDS) mem_len = BCD_MAX_WORDS;
    op->num_words = mem_len;

    for (int i = 0; i < mem_len; i++) {
        op->words[i] = MemoryRead((op->addr + i) & 0xFFFF, true);
    }

    /* Extract nibbles from words into digit string.
     * Matches C# PackedBCDToString: reads nibbles from each word LSB-first,
     * inserting at front (which gives MSB-first order).
     * Sign nibbles (C=+, D=-, F=unsigned) are detected inline. */
    char digits[BCD_MAX_NIBBLES + 2];
    int dpos = 0;
    int nibble_count = 0;
    bool found_sign = false;

    for (int w = 0; w < mem_len && nibble_count < op->field_length; w++) {
        ushort word = op->words[w];

        /* C# PackedBCDToString: reads nibbles i=0..3 (LSB first), Insert(0) each.
         * We replicate by building a per-word string, then appending. */
        char word_str[8];
        int wpos = 0;
        int nib_cnt = 4;
        if (w == 0 && op->right_byte) nib_cnt = 2;

        for (int i = 0; i < nib_cnt; i++) {
            int nibble = (word >> (i * 4)) & 0x0F;

            switch (nibble) {
                case BCD_SIGN_POS:
                case BCD_SIGN_UNS:
                    wpos = 0;  /* Clear (C# result.Clear + Insert) */
                    word_str[wpos++] = '+';
                    break;
                case BCD_SIGN_NEG:
                    wpos = 0;
                    word_str[wpos++] = '-';
                    break;
                default:
                    /* Insert digit at front */
                    memmove(word_str + 1, word_str, wpos);
                    word_str[0] = '0' + (nibble & 0x0F);
                    wpos++;
                    break;
            }
        }
        word_str[wpos] = '\0';

        /* Append word_str to digits, up to field_length, stop on sign */
        for (int j = 0; j < wpos && nibble_count < op->field_length && !found_sign; j++) {
            digits[dpos++] = word_str[j];
            if (word_str[j] == '+' || word_str[j] == '-')
                found_sign = true;
            nibble_count++;
        }
    }
    digits[dpos] = '\0';

    /* Pad with leading zeros if needed (C# ConcatenateBCD) */
    while (dpos < op->field_length) {
        memmove(digits + 1, digits, dpos + 1);
        digits[0] = '0';
        dpos++;
    }

    /* If no sign found, treat as unsigned/positive */
    if (!found_sign) {
        /* Operand has no valid sign nibble - treated as error for SHDE
         * validation, but we still produce a value for other ops */
        op->is_negative = false;
    } else {
        op->is_negative = (dpos > 0 && digits[dpos - 1] == '-');
    }

    /* Save BCD string for SHDE/UPACK */
    strncpy(op->bcd_str, digits, sizeof(op->bcd_str) - 1);
    op->bcd_str[sizeof(op->bcd_str) - 1] = '\0';

    /* Build numeric string: remove sign char */
    char num_str[BCD_MAX_NIBBLES + 4];
    int npos = 0;
    int len = (int)strlen(digits);
    for (int i = 0; i < len; i++) {
        if (digits[i] != '+' && digits[i] != '-')
            num_str[npos++] = digits[i];
    }
    num_str[npos] = '\0';

    /* Pad with leading zeros */
    while (npos < op->field_length - 1) {
        memmove(num_str + 1, num_str, npos + 1);
        num_str[0] = '0';
        npos++;
    }

    /* Insert decimal point */
    if (op->decimal_point > 0 && op->decimal_point < npos) {
        int insert_pos = npos - op->decimal_point;
        memmove(num_str + insert_pos + 1, num_str + insert_pos, npos - insert_pos + 1);
        num_str[insert_pos] = '.';
    }

    /* Parse to double */
    op->value = strtod(num_str, NULL);
    if (op->is_negative)
        op->value = -op->value;
}

/* ================================================================ */
/* Convert double back to BCD and store to memory                   */
/* Matches C# ConvertDoubleToBCD + UpdateDecimalOperandsFromBCD     */
/* ================================================================ */
static void bcd_store_value(BcdOperand *op, double value)
{
    if (op->field_length == 0) return;

    bool is_neg = value < 0;
    if (is_neg) value = -value;

    /* Adjust for decimal point position */
    value = round(value * pow(10.0, op->decimal_point)) ;

    /* Convert integer part to digit string */
    long long int_val = (long long)value;
    int digit_slots = op->field_length - 1; /* reserve 1 for sign */
    if (digit_slots < 0) digit_slots = 0;

    char num_str[BCD_MAX_NIBBLES + 2];
    snprintf(num_str, sizeof(num_str), "%0*lld", digit_slots, int_val);

    /* Truncate if too long (take rightmost digits) */
    int slen = (int)strlen(num_str);
    char *start = num_str;
    if (slen > digit_slots) {
        start = num_str + (slen - digit_slots);
    }

    /* Build BCD string: digits + sign */
    char bcd[BCD_MAX_NIBBLES + 2];
    int bpos = 0;
    for (int i = 0; start[i] && bpos < digit_slots; i++) {
        bcd[bpos++] = start[i];
    }
    /* Pad if needed */
    while (bpos < digit_slots) {
        memmove(bcd + 1, bcd, bpos + 1);
        bcd[0] = '0';
        bpos++;
    }
    bcd[bpos++] = is_neg ? '-' : '+';
    bcd[bpos] = '\0';

    /* Save for SHDE */
    strncpy(op->bcd_str, bcd, sizeof(op->bcd_str) - 1);

    /* Pack BCD string into words */
    for (int i = 0; i < BCD_MAX_WORDS; i++)
        op->words[i] = 0;

    int word_idx = 0;
    int nibble_pos = 3; /* start at high nibble of first word */
    int dcount = 0;

    for (int i = 0; i < bpos && dcount < op->field_length; i++) {
        uint8_t nib;
        if (bcd[i] == '+') nib = BCD_SIGN_POS;
        else if (bcd[i] == '-') nib = BCD_SIGN_NEG;
        else nib = bcd[i] - '0';

        op->words[word_idx] |= (ushort)(nib << (nibble_pos * 4));
        nibble_pos--;
        dcount++;

        if (nibble_pos < 0) {
            word_idx++;
            nibble_pos = 3;
        }
    }

    /* Write words to memory */
    int mem_len = op->field_length >> 2;
    if ((mem_len << 2) != op->field_length)
        mem_len++;
    if (mem_len == 0) mem_len = 1;
    if (mem_len > BCD_MAX_WORDS) mem_len = BCD_MAX_WORDS;

    for (int i = 0; i < mem_len; i++) {
        MemoryWrite(op->words[i], (op->addr + i) & 0xFFFF, true, 2);
    }
}

/* ================================================================ */
/* ADDD - Add Decimal                                               */
/* ================================================================ */
void ndfunc_addd(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, false);
    if (op1.error) return;

    bcd_load(&op2, false);
    if (op2.error) return;

    if (op2.field_length > op1.field_length) {
        /* Overflow */
        return;
    }

    double result = op1.value + op2.value;
    bcd_store_value(&op1, result);

    /* Success: skip return */
    gPC++;
}

/* ================================================================ */
/* SUBD - Subtract Decimal                                          */
/* ================================================================ */
void ndfunc_subd(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, false);
    if (op1.error) return;

    bcd_load(&op2, false);
    if (op2.error) return;

    if (op2.field_length > op1.field_length) {
        return;
    }

    double result = op1.value - op2.value;
    bcd_store_value(&op1, result);

    gPC++;
}

/* ================================================================ */
/* COMD - Compare Decimal                                           */
/* ================================================================ */
void ndfunc_comd(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, false);
    if (op1.error) return;

    bcd_load(&op2, false);
    if (op2.error) return;

    double diff = op1.value - op2.value;

    if (diff > 0)
        gA = 1;
    else if (diff < 0)
        gA = 0xFFFF;
    else
        gA = 0;

    gPC++;
}

/* ================================================================ */
/* SHDE - Decimal Shift                                             */
/* Shift op1 by (op1.dp - op2.dp) places, store in op2              */
/* Matches C# ShiftBCD logic                                        */
/* ================================================================ */

static char *shift_bcd_string(const char *bcd, int orig_len, int shift_count,
                              bool rounding, bool *out_error)
{
    static char result[BCD_MAX_NIBBLES + 4];
    *out_error = false;

    int len = (int)strlen(bcd);

    /* Separate sign from digits */
    bool is_neg = (len > 0 && bcd[len - 1] == '-');
    char sign_char = is_neg ? '-' : '+';

    char digits[BCD_MAX_NIBBLES + 2];
    int dlen = 0;
    for (int i = 0; i < len; i++) {
        if (bcd[i] != '+' && bcd[i] != '-')
            digits[dlen++] = bcd[i];
    }
    digits[dlen] = '\0';

    if (shift_count == 0) {
        snprintf(result, sizeof(result), "%s%c", digits, sign_char);
        return result;
    }

    if (shift_count > 0) {
        /* Shift right: drop rightmost digit, prepend '0' */
        for (int i = 0; i < shift_count; i++) {
            char last = (dlen > 0) ? digits[dlen - 1] : '0';
            /* Shift all digits right by 1 position */
            for (int j = dlen - 1; j > 0; j--)
                digits[j] = digits[j - 1];
            digits[0] = '0';
            digits[dlen] = '\0';

            if (rounding && last >= '5') {
                long long num = strtoll(digits, NULL, 10);
                num++;
                snprintf(digits, sizeof(digits), "%0*lld", dlen, num);
                digits[dlen] = '\0';
            }
        }
    } else {
        /* Shift left: drop leftmost digit, append '0' */
        int left_shift = -shift_count;
        for (int i = 0; i < left_shift; i++) {
            if (digits[0] != '0') {
                *out_error = true;
            }
            /* Shift all digits left by 1 position */
            for (int j = 0; j < dlen - 1; j++)
                digits[j] = digits[j + 1];
            digits[dlen - 1] = '0';
            digits[dlen] = '\0';
        }
    }

    snprintf(result, sizeof(result), "%s%c", digits, sign_char);

    /* Ensure result is correct length */
    int rlen = (int)strlen(result);
    if (rlen > orig_len) {
        memmove(result, result + (rlen - orig_len), orig_len + 1);
    }

    return result;
}

void ndfunc_shde(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, false);
    if (op1.error) return;

    /* ND-100 manual: "The sign and digits of the first operand are checked
     * before execution and any illegal digit codes reported."
     * No valid sign nibble found = illegal operand = error (no skip). */
    {
        int slen = (int)strlen(op1.bcd_str);
        if (slen == 0 || (op1.bcd_str[slen-1] != '+' && op1.bcd_str[slen-1] != '-'))
            return;
    }

    /* Don't need to load op2 data, just its descriptor */
    if (op2.field_length == 0) {
        return; /* error */
    }

    int shift_direction = op1.decimal_point - op2.decimal_point;

    /* Start with op1's BCD string */
    char work[BCD_MAX_NIBBLES + 4];
    strncpy(work, op1.bcd_str, sizeof(work) - 1);
    work[sizeof(work) - 1] = '\0';

    /* Pad to op2 field_length if needed (C# PadLeft) */
    if (op2.field_length > op1.field_length) {
        int wlen = (int)strlen(work);
        while (wlen < op2.field_length) {
            memmove(work + 1, work, wlen + 1);
            work[0] = '0';
            wlen++;
        }
    }

    bool shift_error = false;
    char *newbcd = work;

    /* Adjust decimals - Left shift case (C# lines 1144-1152) */
    if (shift_direction < 0 && op1.decimal_point > 0 &&
        op1.decimal_point != op2.decimal_point) {
        int numShift = op1.decimal_point - op2.decimal_point;
        newbcd = shift_bcd_string(newbcd, op2.field_length, numShift, false, &shift_error);
        /* Copy result to work buffer for next call */
        strncpy(work, newbcd, sizeof(work) - 1);
        work[sizeof(work) - 1] = '\0';
        newbcd = work;
    }

    /* Adjust decimals - Right shift case (C# lines 1155-1163) */
    if (shift_direction >= 0 && op2.decimal_point > 0 &&
        op1.decimal_point != op2.decimal_point) {
        int numShift = op1.decimal_point - op2.decimal_point;
        newbcd = shift_bcd_string(newbcd, op2.field_length, numShift, false, &shift_error);
        strncpy(work, newbcd, sizeof(work) - 1);
        work[sizeof(work) - 1] = '\0';
        newbcd = work;
    }

    /* Main shift (C# line 1165) */
    char *shifted = shift_bcd_string(newbcd, op2.field_length, shift_direction,
                                     op2.rounding, &shift_error);

    if (shift_error) {
        /* Error: significant digits lost on left shift - no skip */
        return;
    }

    /* Trim to field_length if needed (C# lines 1173-1178) */
    int slen = (int)strlen(shifted);
    if (slen > op2.field_length) {
        int trim = slen - op2.field_length;
        memmove(shifted, shifted + trim, op2.field_length + 1);
    }

    /* Pack the shifted BCD string directly into op2 words and write to memory */
    for (int i = 0; i < BCD_MAX_WORDS; i++) op2.words[i] = 0;

    int word_idx = 0;
    int nibble_pos = 3;
    int dcount = 0;

    for (int i = 0; i < slen && dcount < op2.field_length; i++) {
        uint8_t nib;
        if (shifted[i] == '+') nib = BCD_SIGN_POS;
        else if (shifted[i] == '-') nib = BCD_SIGN_NEG;
        else nib = shifted[i] - '0';

        op2.words[word_idx] |= (ushort)(nib << (nibble_pos * 4));
        nibble_pos--;
        dcount++;

        if (nibble_pos < 0) {
            word_idx++;
            nibble_pos = 3;
        }
    }

    int mem_len = op2.field_length >> 2;
    if ((mem_len << 2) != op2.field_length) mem_len++;
    if (mem_len == 0) mem_len = 1;
    if (mem_len > BCD_MAX_WORDS) mem_len = BCD_MAX_WORDS;

    for (int i = 0; i < mem_len; i++) {
        MemoryWrite(op2.words[i], (op2.addr + i) & 0xFFFF, true, 2);
    }

    gPC++;
}

/* ================================================================ */
/* PACK - ASCII to BCD                                              */
/* ================================================================ */
void ndfunc_pack(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, true); /* ASCII mode */
    if (op1.error) return;

    bcd_load(&op2, false); /* BCD mode */
    if (op2.error) return;

    bcd_store_value(&op2, op1.value);

    gPC++;
}

/* ================================================================ */
/* UPACK - BCD to ASCII                                             */
/* ================================================================ */
void ndfunc_unpack(ushort instr)
{
    (void)instr;
    BcdOperand op1, op2;

    bcd_parse(&op1, gA, gD);
    bcd_parse(&op2, gX, gT);

    bcd_load(&op1, false); /* BCD source */
    if (op1.error) {
        gD = (gD & 0xFFF0) | 2;
        return;
    }

    if (op2.field_length == 0) return;

    /* For now, simple ASCII conversion - store digits as ASCII bytes */
    /* TODO: full ASCII sign format support */
    bcd_store_value(&op2, op1.value);

    gPC++;
}
