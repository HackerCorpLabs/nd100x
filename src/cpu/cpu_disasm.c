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
//#define DEBUG_IONOFF

#include <string.h>

#include "cpu_types.h"
#include "cpu_protos.h"

#define BUFSTRSIZE 24
#define BUFSTRSIZE_SMALL 16


char *regn[] = {"S","D","P","B","L","A","T","X","U0","U1"};
char *regn_w[] = {"DS","DD","DP","DB","DL","DA","DT","DX"};

char *intregn_r[] = {"PANS","STS","OPR","PGS","PVL","IIC","PID","PIE","CSR","ACTL", "ALD" ,"PES","PGC","PEA","16","17"};
char *intregn_w[] = {"PANC","STS","LMP","PCR", "4", "IIE","PID","PIE","CCL","LCIL","UCILR","13", "14", "15" ,"16","17"};

char *relmode_str[] ={"",",B ","I ","I ,B ",",X ",",X ,B ","I ,X ","I ,B ,X "};
char *shtype_str[] ={"","ROT ","ZIN ","LIN "};

char *skiptype_str[] = {"EQL","GEQ","GRE","MGRE","UEQ","LSS","LST","MLST"};
char *skipregn_dst[] = {"0","DD","DP","DB","DL","DA","DT","DX"};
char *skipregn_src[] = {"0","SD","SP","SB","SL","SA","ST","SX"};

char *bopstsbit_str[] = {"SSPTM","SSTG","SSK","SSZ","SSQ","SSO","SSC","SSM","","","","","","","",""};

char *bop_str[] = {"BSET ZRO","BSET ONE","BSET BCM","BSET BAC","BSKP ZRO","BSKP ONE",
           "BSKP BCM","BSKP BAC","BSTC","BSTA","BLDC","BLDA","BANC","BAND","BORC","BORA"};

/* OpToStr
 * IN: pointer to string ,raw operand
 * OUT: Sets the string with the dissassembled operand and values
 */
void  OpToStr(char *return_string, uint16_t max_len, uint16_t operand)
{
	ushort instr;
	char numstr[BUFSTRSIZE_SMALL];
	char deltastr[BUFSTRSIZE_SMALL];
	unsigned char nibble;
	char offset, delta;
	unsigned char relmode;
	bool isneg;
	char opstr[BUFSTRSIZE];

	offset = operand & 0x00ff;
	nibble = operand & 0x000f;
	relmode = (operand & 0x0700) >> 8;
	delta = (operand & 070);

	/* put offset into a string variable in octal with +/- sign for easy reading */
	((int)offset < 0) ? (void)snprintf(numstr, BUFSTRSIZE, "-%o", -(int)offset) : (void)snprintf(numstr, BUFSTRSIZE, "%o", offset);

	/* ND110 delta offset for some instructions */
	(void)snprintf(deltastr, sizeof(deltastr), "%o", delta);

	instr = extract_opcode(operand);
	switch (instr)
	{
	case 0000000: /* STZ */
		//(void)snprintf(opstr, BUFSTRSIZE, "STZ %s%s", relmode_str[relmode], numstr);		
		(void)snprintf(opstr, sizeof(opstr), "STZ %s%s", relmode_str[relmode], numstr);		
		break; 
	case 0004000: /* STA */
		(void)snprintf(opstr, sizeof(opstr), "STA %s%s", relmode_str[relmode], numstr);
		break;
	case 0010000: /* STT */
		(void)snprintf(opstr, sizeof(opstr), "STT %s%s", relmode_str[relmode], numstr);
		break;
	case 0014000: /* STX */
		(void)snprintf(opstr, sizeof(opstr), "STX %s%s", relmode_str[relmode], numstr);
		break;
	case 0020000: /* STD */
		(void)snprintf(opstr, sizeof(opstr), "STD %s%s", relmode_str[relmode], numstr);
		break;
	case 0024000: /* LDD */
		(void)snprintf(opstr, sizeof(opstr), "LDD %s%s", relmode_str[relmode], numstr);
		break;
	case 0030000: /* STF */
		(void)snprintf(opstr, sizeof(opstr), "STF %s%s", relmode_str[relmode], numstr);
		break;
	case 0034000: /* LDF */
		(void)snprintf(opstr, sizeof(opstr), "LDF %s%s", relmode_str[relmode], numstr);
		break;
	case 0040000: /* MIN */
		(void)snprintf(opstr, sizeof(opstr), "MIN %s%s", relmode_str[relmode], numstr);
		break;
	case 0044000: /* LDA */
		(void)snprintf(opstr, sizeof(opstr), "LDA %s%s", relmode_str[relmode], numstr);
		break;
	case 0050000: /* LDT */
		(void)snprintf(opstr, sizeof(opstr), "LDT %s%s", relmode_str[relmode], numstr);
		break;
	case 0054000: /* LDX */
		(void)snprintf(opstr, sizeof(opstr), "LDX %s%s", relmode_str[relmode], numstr);
		break;
	case 0060000: /* ADD */
		(void)snprintf(opstr, sizeof(opstr), "ADD %s%s", relmode_str[relmode], numstr);
		break;
	case 0064000: /* SUB */
		(void)snprintf(opstr, sizeof(opstr), "SUB %s%s", relmode_str[relmode], numstr);
		break;
	case 0070000: /* AND */
		(void)snprintf(opstr, sizeof(opstr), "AND %s%s", relmode_str[relmode], numstr);
		break;
	case 0074000: /* ORA */
		(void)snprintf(opstr, sizeof(opstr), "ORA %s%s", relmode_str[relmode], numstr);
		break;
	case 0100000: /* FAD */
		(void)snprintf(opstr, sizeof(opstr), "FAD %s%s", relmode_str[relmode], numstr);
		break;
	case 0104000: /* FSB */
		(void)snprintf(opstr, sizeof(opstr), "FSB %s%s", relmode_str[relmode], numstr);
		break;
	case 0110000: /* FMU */
		(void)snprintf(opstr, sizeof(opstr), "FMU %s%s", relmode_str[relmode], numstr);
		break;
	case 0114000: /* FDV */
		(void)snprintf(opstr, sizeof(opstr), "FDV %s%s", relmode_str[relmode], numstr);
		break;
	case 0120000: /* MPY */
		(void)snprintf(opstr, sizeof(opstr), "MPY %s%s", relmode_str[relmode], numstr);
		break;
	case 0124000: /* JMP */
		(void)snprintf(opstr, sizeof(opstr), "JMP %s%s", relmode_str[relmode], numstr);
		break;
	case 0130000: /* JAP */
		(void)snprintf(opstr, sizeof(opstr), "JAP %s", numstr);
		break;
	case 0130400: /* JAN */
		(void)snprintf(opstr, sizeof(opstr), "JAN %s", numstr);
		break;
	case 0131000: /* JAZ */
		(void)snprintf(opstr, sizeof(opstr), "JAZ %s", numstr);
		break;
	case 0131400: /* JAF */
		(void)snprintf(opstr, sizeof(opstr), "JAF %s", numstr);
		break;
	case 0132000: /* JPC */
		(void)snprintf(opstr, sizeof(opstr), "JPC %s", numstr);
		break;
	case 0132400: /* JNC */
		(void)snprintf(opstr, sizeof(opstr), "JNC %s", numstr);
		break;
	case 0133000: /* JXZ */
		(void)snprintf(opstr, sizeof(opstr), "JXZ %s", numstr);
		break;
	case 0133400: /* JXN */
		(void)snprintf(opstr, sizeof(opstr), "JXN %s", numstr);
		break;
	case 0134000: /* JPL */
		(void)snprintf(opstr, sizeof(opstr), "JPL %s%s", relmode_str[relmode], numstr);
		break;
	case 0140000: /* SKP */
		(void)snprintf(opstr, sizeof(opstr), "SKP IF %s %s %s", skipregn_dst[(operand & 0x0007)], skiptype_str[((operand & 0x0700) >> 8)], skipregn_src[((operand & 0x0038) >> 3)]);
		break;
	case 0140120: /* ADDD */
		(void)snprintf(opstr, sizeof(opstr), "ADDD");
		break;
	case 0140121: /* SUBD */
		(void)snprintf(opstr, sizeof(opstr), "SUBD");
		break;
	case 0140122: /* COMD */
		(void)snprintf(opstr, sizeof(opstr), "COMD");
		break;
	case 0140123: /* TSET */
		(void)snprintf(opstr, sizeof(opstr), "TSET");
		break;
	case 0140124: /* PACK */
		(void)snprintf(opstr, sizeof(opstr), "PACK");
		break;
	case 0140125: /* UPACK */
		(void)snprintf(opstr, sizeof(opstr), "UPACK");
		break;
	case 0140126: /* SHDE */
		(void)snprintf(opstr, sizeof(opstr), "SHDE");
		break;
	case 0140127: /* RDUS */
		(void)snprintf(opstr, sizeof(opstr), "RDUS");
		break;
	case 0140130: /* BFILL */
		(void)snprintf(opstr, sizeof(opstr), "BFILL");
		break;
	case 0140131: /* MOVB */
		(void)snprintf(opstr, sizeof(opstr), "MOVB");
		break;
	case 0140132: /* MOVBF */
		(void)snprintf(opstr, sizeof(opstr), "MOVBF");
		break;
	case 0140133:																				  /* VERSN - ND110 specific */
		if ((CurrentCPUType = ND100) || (CurrentCPUType = ND100CE) || (CurrentCPUType = ND100CX)) /* We are ND100 */
			break;
		else /* We are a ND110, print instruction */
			(void)snprintf(opstr, sizeof(opstr), "VERSN");
	case 0140134: /* INIT */
		(void)snprintf(opstr, sizeof(opstr), "INIT");
		break;
	case 0140135: /* ENTR */
		(void)snprintf(opstr, sizeof(opstr), "ENTR");
		break;
	case 0140136: /* LEAVE */
		(void)snprintf(opstr, sizeof(opstr), "LEAVE");
		break;
	case 0140137: /* ELEAV */
		(void)snprintf(opstr, sizeof(opstr), "ELEAV");
		break;
	case 0140300: /* SETPT */
		(void)snprintf(opstr, sizeof(opstr), "SETPT");
		break;
	case 0140301: /* CLEPT */
		(void)snprintf(opstr, sizeof(opstr), "CLEPT");
		break;
	case 0140302: /* CLNREENT */
		(void)snprintf(opstr, sizeof(opstr), "CLNREENT");
		break;
	case 0140303: /* CHREENT-PAGES */
		(void)snprintf(opstr, sizeof(opstr), "CHREENT-PAGES");
		break;
	case 0140304: /* CLEPU */
		(void)snprintf(opstr, sizeof(opstr), "CLEPU");
		break;
	case 0140200: /* USER0 */
		(void)snprintf(opstr, sizeof(opstr), "USER0");
		break;
	case 0140500:																				  /* USER1 or ND110 instruction WGLOB */
		if ((CurrentCPUType = ND100) || (CurrentCPUType = ND100CE) || (CurrentCPUType = ND100CX)) /* We are ND100 */
			(void)snprintf(opstr, sizeof(opstr), "USER1");
		else
			(void)snprintf(opstr, sizeof(opstr), "WGLOB"); /* We are ND110 */
		break;
	case 0140501: /* RGLOB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "RGLOB");
		break;
	case 0140502: /* INSPL - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "INSPL");
		break;
	case 0140503: /* REMPL - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "REMPL");
		break;
	case 0140504: /* CNREK - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "CNREK");
		break;
	case 0140505: /* CLPT  - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "CLPT");
		break;
	case 0140506: /* ENPT  - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "ENPT");
		break;
	case 0140507: /* REPT  - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "REPT");
		break;
	case 0140510: /* LBIT  - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "LBIT");
		break;
	case 0140513: /* SBITP - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SBITP");
		break;
	case 0140514: /* LBYTP - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "LBYTP");
		break;
	case 0140515: /* SBYTP - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SBYTP");
		break;
	case 0140516: /* TSETP - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "TSETP");
		break;
	case 0140517: /* RDUSP - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "RDUSP");
		break;
	case 0140600: /* EXR */
		(void)snprintf(opstr, sizeof(opstr), "EXR %s", skipregn_src[((operand & 0x0038) >> 3)]);
		break;
	case 0140700:																				  /* USER2 */
		if ((CurrentCPUType = ND100) || (CurrentCPUType = ND100CE) || (CurrentCPUType = ND100CX)) /* We are ND100 */
			(void)snprintf(opstr, sizeof(opstr), "USER2");
		else
			(void)snprintf(opstr, sizeof(opstr), "LASB %s", deltastr); /* We are ND110 */
		break;
	case 0140701: /* SASB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SASB %s", deltastr);
		break;
	case 0140702: /* LACB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "LACB %s", deltastr);
		break;
	case 0140703: /* SASB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SASB %s", deltastr);
		break;
	case 0140704: /* LXSB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "LXSB %s", deltastr);
		break;
	case 0140705: /* LXCB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "LXCB %s", deltastr);
		break;
	case 0140706: /* SZSB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SZSB %s", deltastr);
		break;
	case 0140707: /* SZCB - ND110 Specific */
		(void)snprintf(opstr, sizeof(opstr), "SZCB %s", deltastr);
		break;
	case 0141100: /* USER3 */
		(void)snprintf(opstr, sizeof(opstr), "USER3");
		break;
	case 0141200: /* RMPY */
		(void)snprintf(opstr, sizeof(opstr), "RMPY %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0141300: /* USER4 */
		(void)snprintf(opstr, sizeof(opstr), "USER4");
		break;
	case 0141500: /* USER5 */
		(void)snprintf(opstr, sizeof(opstr), "USER5");
		break;
	case 0141600: /* RDIV */
		(void)snprintf(opstr, sizeof(opstr), "RDIV %s", skipregn_src[((operand & 0x0038) >> 3)]);
		break;
	case 0141700: /* USER6 */
		(void)snprintf(opstr, sizeof(opstr), "USER6");
		break;
	case 0142100: /* USER7 */
		(void)snprintf(opstr, sizeof(opstr), "USER7");
		break;
	case 0142200: /* LBYT */
		/* NOTE : moved from old parsing, SKP part, might have introduced P++ probs here */
		(void)snprintf(opstr, sizeof(opstr), "LBYT");
		break;
	case 0142300: /* USER8 */
		(void)snprintf(opstr, sizeof(opstr), "USER8");
		break;
	case 0142500: /* USER9 */
		(void)snprintf(opstr, sizeof(opstr), "USER9");
		break;
	case 0142600: /* SBYT */
		/* NOTE : moved from old parsing, SKP part, might have introduced P++ probs here */
		(void)snprintf(opstr, sizeof(opstr), "SBYT");
		break;
	case 0142700: /* GECO - Undocumented instruction */
		(void)snprintf(opstr, sizeof(opstr), "GECO");
		break;
	case 0143100: /* MOVEW */
		(void)snprintf(opstr, sizeof(opstr), "MOVEW");
		break;
	case 0143200: /* MIX3 */
		(void)snprintf(opstr, sizeof(opstr), "MIX3");
		break;
	case 0143300: /* LDATX */
		(void)snprintf(opstr, sizeof(opstr), "LDATX");
		break;
	case 0143301: /* LDXTX */
		(void)snprintf(opstr, sizeof(opstr), "LDXTX");
		break;
	case 0143302: /* LDDTX */
		(void)snprintf(opstr, sizeof(opstr), "LDDTX");
		break;
	case 0143303: /* LDBTX */
		(void)snprintf(opstr, sizeof(opstr), "LDBTX");
		break;
	case 0143304: /* STATX */
		(void)snprintf(opstr, sizeof(opstr), "STATX");
		break;
	case 0143305: /* STZTX */
		(void)snprintf(opstr, sizeof(opstr), "STZTX");
		break;
	case 0143306: /* STDTX */
		(void)snprintf(opstr, sizeof(opstr), "STDTX");
		break;
	case 0143500: /* LWCS */
		(void)snprintf(opstr, sizeof(opstr), "LWCS");
		break;
	case 0143604: /* IDENT PL10 */
		(void)snprintf(opstr, sizeof(opstr), "IDENT PL10");
		break;
	case 0143611: /* IDENT PL11 */
		(void)snprintf(opstr, sizeof(opstr), "IDENT PL11");
		break;
	case 0143622: /* IDENT PL12 */
		(void)snprintf(opstr, sizeof(opstr), "IDENT PL12");
		break;
	case 0143643: /* IDENT PL13 */
		(void)snprintf(opstr, sizeof(opstr), "IDENT PL13");
		break;
	case 0144000: /* SWAP */
		(void)snprintf(opstr, sizeof(opstr), "SWAP %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144100: /* SWAP CLD */
		(void)snprintf(opstr, sizeof(opstr), "SWAP CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144200: /* SWAP CM1 */
		(void)snprintf(opstr, sizeof(opstr), "SWAP CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144300: /* SWAP CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "SWAP CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144400: /* RAND */
		(void)snprintf(opstr, sizeof(opstr), "RAND %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144500: /* RAND CLD */
		(void)snprintf(opstr, sizeof(opstr), "RAND CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144600: /* RAND CM1 */
		(void)snprintf(opstr, sizeof(opstr), "RAND CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0144700: /* RAND CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RAND CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145000: /* REXO */
		(void)snprintf(opstr, sizeof(opstr), "REXO %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145100: /* REXO CLD */
		(void)snprintf(opstr, sizeof(opstr), "REXO CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145200: /* REXO CM1 */
		(void)snprintf(opstr, sizeof(opstr), "REXO CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145300: /* REXO CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "REXO CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145400: /* RORA */
		(void)snprintf(opstr, sizeof(opstr), "RORA %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145500: /* RORA CLD */
		(void)snprintf(opstr, sizeof(opstr), "RORA CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145600: /* RORA CM1 */
		(void)snprintf(opstr, sizeof(opstr), "RORA CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0145700: /* RORA CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RORA CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146000: /* RADD */
		(void)snprintf(opstr, sizeof(opstr), "RADD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146100: /* RADD CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146200: /* RADD CM1 */
		(void)snprintf(opstr, sizeof(opstr), "RADD CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146300: /* RADD CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146400: /* RADD AD1 */
		(void)snprintf(opstr, sizeof(opstr), "RADD AD1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146500: /* RADD AD1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD AD1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146600: /* RADD AD1 CM1 */
		(void)snprintf(opstr, sizeof(opstr), "RADD AD1 CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0146700: /* RADD AD1 CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD AD1 CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0147000: /* RADD ADC */
		(void)snprintf(opstr, sizeof(opstr), "RADD ADC %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0147100: /* RADD ADC CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD ADC CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0147200: /* RADD ADC CM1 */
		(void)snprintf(opstr, sizeof(opstr), "RADD ADC CM1 %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0147300: /* RADD ADC CM1 CLD */
		(void)snprintf(opstr, sizeof(opstr), "RADD ADC CM1 CLD %s %s", skipregn_src[((operand & 0x0038) >> 3)], skipregn_dst[(operand & 0x0007)]);
		break;
	case 0147400: /* NOOP */
	case 0147500: /* NOOP */
	case 0147600: /* NOOP */
	case 0147700: /* NOOP */
		(void)snprintf(opstr, sizeof(opstr), "ROP NOOP");
		break;
	case 0150000: /* TRA */
		(void)snprintf(opstr, sizeof(opstr), "TRA %s", intregn_r[nibble]);
		break;
	case 0150100: /* TRR */
		(void)snprintf(opstr, sizeof(opstr), "TRR %s", intregn_w[nibble]);
		break;
	case 0150200: /* MCL */
		(void)snprintf(opstr, sizeof(opstr), "MCL %s", intregn_w[nibble]);
		break;
	case 0150300: /* MST */
		(void)snprintf(opstr, sizeof(opstr), "MST %s", intregn_w[nibble]);
		break;
	case 0150400: /* OPCOM */
		(void)snprintf(opstr, sizeof(opstr), "OPCOM");
		break;
	case 0150401: /* IOF */
		(void)snprintf(opstr, sizeof(opstr), "IOF");
		break;
	case 0150402: /* ION */
		(void)snprintf(opstr, sizeof(opstr), "ION");
		break;
	case 0150404: /* POF */
		(void)snprintf(opstr, sizeof(opstr), "POF");
		break;
	case 0150405: /* PIOF */
		(void)snprintf(opstr, sizeof(opstr), "PIOF");
		break;
	case 0150406: /* SEX */
		(void)snprintf(opstr, sizeof(opstr), "SEX");
		break;
	case 0150407: /* REX */
		(void)snprintf(opstr, sizeof(opstr), "REX");
		break;
	case 0150410: /* PON */
		(void)snprintf(opstr, sizeof(opstr), "PON");
		break;
	case 0150412: /* PION */
		(void)snprintf(opstr, sizeof(opstr), "PION");
		break;
	case 0150415: /* IOXT */
		(void)snprintf(opstr, sizeof(opstr), "IOXT");
		break;
	case 0150416: /* EXAM */
		(void)snprintf(opstr, sizeof(opstr), "EXAM");
		break;
	case 0150417: /* DEPO */
		(void)snprintf(opstr, sizeof(opstr), "DEPO");
		break;
	case 0151000:								   /* WAIT */
		(void)snprintf(opstr, sizeof(opstr), "WAIT"); /* TODO:: number??*/
		break;
	case 0151400: /* NLZ*/
		(void)snprintf(opstr, sizeof(opstr), "NLZ %s", numstr);
		break;
	case 0152000: /* DNZ*/
		(void)snprintf(opstr, sizeof(opstr), "DNZ %s", numstr);
		break;
	case 0152400: /* SRB */ /* NOTE: These two seems to have bit req on 0-2 as well */
		(void)snprintf(opstr, sizeof(opstr), "SRB %o", (operand & 0x0078));
		break;
	case 0152600: /* LRB */ /* NOTE: These two seems to have bit req on 0-2 as well */
		(void)snprintf(opstr, sizeof(opstr), "LRB %o", (operand & 0x0078) >> 3);
		break;
	case 0153000: /* MON */
		(void)snprintf(opstr, sizeof(opstr), "MON %o", (operand & 0x00ff));
		break;
	case 0153400: /* IRW */
		(void)snprintf(opstr, sizeof(opstr), "IRW %o %s", (operand & 0x0078), regn_w[(operand & 0x0007)]);
		break;
	case 0153600: /* IRR */
		(void)snprintf(opstr, sizeof(opstr), "IRR %o %s", (operand & 0x0078), regn_w[(operand & 0x0007)]);
		break;
	case 0154000: /* SHT */
		/* negative value -> shift right  else shift left*/
		isneg = ((operand & 0x0020) >> 5) ? 1 : 0;
		//		offset = ((operand & 0x0020)>>5) ? (char)((operand & 0x003F) | 0x00C0) : (operand & 0x003F);
		offset = (isneg) ? (~((operand & 0x003F) | 0xFFC0) + 1) : (operand & 0x003F);
		(isneg) ? (void)snprintf(numstr, sizeof(numstr), "SHR %o", offset) : (void)snprintf(numstr, sizeof(numstr), "%o", offset);
		(void)snprintf(opstr, sizeof(opstr), "SHT %s%s", shtype_str[((operand & 0x0600) >> 9)], numstr);
		break;
	case 0154200: /* SHD */
		/* negative value -> shift right  else shift left*/
		isneg = ((operand & 0x0020) >> 5) ? 1 : 0;
		offset = (isneg) ? (~((operand & 0x003F) | 0xFFC0) + 1) : (operand & 0x003F);
		(isneg) ? (void)snprintf(numstr, sizeof(numstr), "SHR %o", offset) : (void)snprintf(numstr, sizeof(numstr), "%o", offset);
		//		offset = ((operand & 0x0020)>>5) ? (char)((operand & 0x003F) | 0x00C0) : (operand & 0x003F);
		//		((int)offset <0) ? (void)snprintf(numstr,BUFSTRSIZE,"SHR %o",-(int)offset) : (void)snprintf(numstr,BUFSTRSIZE,"%o",offset);
		(void)snprintf(opstr, sizeof(opstr), "SHD %s%s", shtype_str[((operand & 0x0600) >> 9)], numstr);
		break;
	case 0154400: /* SHA */
		/* negative value -> shift right  else shift left*/
		isneg = ((operand & 0x0020) >> 5) ? 1 : 0;
		offset = (isneg) ? (~((operand & 0x003F) | 0xFFC0) + 1) : (operand & 0x003F);
		(isneg) ? (void)snprintf(numstr, BUFSTRSIZE, "SHR %o", offset) : (void)snprintf(numstr, BUFSTRSIZE, "%o", offset);
		//		offset = ((operand & 0x0020)>>5) ? (char)((operand & 0x003F) | 0x00C0) : (operand & 0x003F);
		//		((int)offset <0) ? (void)snprintf(numstr,BUFSTRSIZE,"SHR %o",-(int)offset) : (void)snprintf(numstr,BUFSTRSIZE,"%o",offset);
		(void)snprintf(opstr, sizeof(opstr), "SHA %s%s", shtype_str[((operand & 0x0600) >> 9)], numstr);
		break;
	case 0154600: /* SAD */
		/* negative value -> shift right  else shift left*/
		isneg = ((operand & 0x0020) >> 5) ? 1 : 0;
		offset = (isneg) ? (~((operand & 0x003F) | 0xFFC0) + 1) : (operand & 0x003F);
		(isneg) ? (void)snprintf(numstr, BUFSTRSIZE, "SHR %o", offset) : (void)snprintf(numstr, BUFSTRSIZE, "%o", offset);
		//		offset = ((operand & 0x0020)>>5) ? (char)((operand & 0x003F) | 0x00C0) : (operand & 0x003F);
		//		((int)offset <0) ? (void)snprintf(numstr,BUFSTRSIZE,"SHR %o",-(int)offset) : (void)snprintf(numstr,BUFSTRSIZE,"%o",offset);
		(void)snprintf(opstr, sizeof(opstr), "SAD %s%s", shtype_str[((operand & 0x0600) >> 9)], numstr);
		break;
	case 0160000: /* IOT */
		(void)snprintf(opstr, sizeof(opstr), "IOT %o", (operand & 0x07ff));
		break;
	case 0164000: /* IOX */
		(void)snprintf(opstr, sizeof(opstr), "IOX %o", (operand & 0x07ff));
		break;
	case 0170000: /* SAB */
		(void)snprintf(opstr, sizeof(opstr), "SAB %s", numstr);
		break;
	case 0170400: /* SAA */
		(void)snprintf(opstr, sizeof(opstr), "SAA %s", numstr);
		break;
	case 0171000: /* SAT */
		(void)snprintf(opstr, sizeof(opstr), "SAT %s", numstr);
		break;
	case 0171400: /* SAX */
		(void)snprintf(opstr, sizeof(opstr), "SAX %s", numstr);
		break;
	case 0172000: /* AAB */
		(void)snprintf(opstr, sizeof(opstr), "AAB %s", numstr);
		break;
	case 0172400: /* AAA */
		(void)snprintf(opstr, sizeof(opstr), "AAA %s", numstr);
		break;
	case 0173000: /* AAT */
		(void)snprintf(opstr, sizeof(opstr), "AAT %s", numstr);
		break;
	case 0173400: /* AAX */
		(void)snprintf(opstr, sizeof(opstr), "AAX %s", numstr);
		break;
	case 0174000:				 /* BSET ZRO */
	case 0174200:				 /* BSET ONE */
	case 0174400:				 /* BSET BCM */
	case 0174600:				 /* BSET BAC */
	case 0175000:				 /* BSKP ZRO */
	case 0175200:				 /* BSKP ONE */
	case 0175400:				 /* BSKP BCM */
	case 0175600:				 /* BSKP BAC */
	case 0176000:				 /* BSTC */
	case 0176200:				 /* BSTA */
	case 0176400:				 /* BLDC */ 
	case 0176600:				 /* BLDA */
	case 0177000:				 /* BANC */
	case 0177200:				 /* BAND */
	case 0177400:				 /* BORC */
	case 0177600:				 /* BORA */
		if (!(operand & 0x0007)) /* STS reg bits handling FIXME:: what if it is bit >7 & STS??*/
			(void)snprintf(opstr, sizeof(opstr), "%s %s", bop_str[((operand & 0x0780) >> 7)], bopstsbit_str[((operand & 0x0078) >> 3)]);
		else
			(void)snprintf(opstr, sizeof(opstr), "%s %o D%s", bop_str[((operand & 0x0780) >> 7)], (int)(operand & 0x0078), regn[(operand & 0x0007)]);
		break;
	default: /* UNDEF */ /* Some ND instruction codes is undefined unfortunately. */
		(void)snprintf(opstr, sizeof(opstr), "UNDEF");
		break;
	}

	// Safe string copy with guaranteed null-termination
	snprintf(return_string, max_len, "%s", opstr);	
}


// Declare and define the disasm array and pointer to it
DisasmArray disasm_arr = { NULL };
DisasmArray* p_DIS = &disasm_arr;

int disasm_ctr = 0;
 
void disasm_allocate(ushort addr) {
	if ((*p_DIS)[addr]) return; /* already exists */	

	// Allocate memory for the disasm entry at the given address
	(*p_DIS)[addr] = calloc(1,sizeof(struct disasm_entry));	
}

void disasm_instr(ushort addr, ushort instr){

	// Add the instruction to the disassembly array if it doesn't exist
	if ((*p_DIS)[addr] == NULL)
	{
		disasm_addword(addr, instr);
	}

	// If the instruction exists, set it to code and copy the disassembly string
	if ((*p_DIS)[addr] != NULL) {
		char disasm_str[BUFSTRSIZE];
		OpToStr(disasm_str, sizeof(disasm_str), instr);	


		(*p_DIS)[addr]->iscode = true;
		snprintf((*p_DIS)[addr]->asm_str,32,"%s",disasm_str);
	}
}

void disasm_exr(ushort addr, ushort instr){
	char disasm_str[BUFSTRSIZE];
	OpToStr(disasm_str, sizeof(disasm_str), instr);	

	if ((*p_DIS)[addr] != NULL) {
		(*p_DIS)[addr]->isexr = true;
		snprintf((*p_DIS)[addr]->exr,32,"%s",disasm_str);
	}
}

void disasm_addword(ushort addr, ushort myword){
	if ((*p_DIS)[addr]) return; /* already exists */

	(*p_DIS)[addr] = calloc(1,sizeof(struct disasm_entry));
	if ((*p_DIS)[addr] != NULL) {
		(*p_DIS)[addr]->theword = myword;
	}
}

void disasm_init(){
	int i;
	for(i=0;i<65536;i++){
		(*p_DIS)[i]= NULL;
	}
	disasm_ctr=0;
}

void disasm_setlbl(ushort addr){
	disasm_ctr++;
	if ((*p_DIS)[addr] != NULL) {
		(*p_DIS)[addr]->labelno = disasm_ctr;
	}
}

void disasm_set_isdata(ushort addr){
	if ((*p_DIS)[addr] != NULL) {
		(*p_DIS)[addr]->isdata = true;
	}
}

void disasm_userel(ushort addr, ushort where){
	if ((*p_DIS)[addr] != NULL) {
		if ((*p_DIS)[addr]->use_rel) { /* we have already used relative from here */
		} else {
			(*p_DIS)[addr]->use_rel = true;
			disasm_allocate(where); // make sure we dont have a null ptr.
			if ((*p_DIS)[where]->labelno) { /* Where already has a label  */
				(*p_DIS)[addr]->rel_acc_lbl = (*p_DIS)[where]->labelno;
			} else {
				disasm_setlbl(where);
				(*p_DIS)[addr]->rel_acc_lbl = (*p_DIS)[where]->labelno;
			}
		}
	}
}


void disasm_dump(){
	int i;
	int tmp;
	char u,l;
	ushort w;
	char* disasm_str;

	//char* disasm_fname = "disasm.txt";
	char* disasm_fname = "/dev/stdout";
	char* disasm_ftype = "w";

	FILE* disasm_file = fopen(disasm_fname,disasm_ftype);

	for(i=0;i<65536;i++){
		if ((*p_DIS)[i] != NULL) {
			w = (*p_DIS)[i]->theword;
			u = (w >> 8) & 0xff;
			l = w & 0xff;

			fprintf(disasm_file,"%06o    %06o   ",i,(*p_DIS)[i]->theword);
			if ((*p_DIS)[i]->labelno)
				fprintf(disasm_file," L%05d ",(*p_DIS)[i]->labelno);
			else
				fprintf(disasm_file,"       ");
			if ((*p_DIS)[i]->iscode) {
				fprintf(disasm_file,"%s",(*p_DIS)[i]->asm_str);
				tmp=strlen((const char*)(*p_DIS)[i]->asm_str);
				fprintf(disasm_file,"%.*s", (32-tmp), "                                 "); /* align */
				if ((*p_DIS)[i]->use_rel)
					fprintf(disasm_file,"%% L%05d ",(*p_DIS)[i]->rel_acc_lbl);
				if ((*p_DIS)[i]->isexr)
					fprintf(disasm_file,"%% %s",(*p_DIS)[i]->exr);
			} else if ((*p_DIS)[i]->isdata) {
				fprintf(disasm_file,"DATA: ");
				if (u>=32 & u<=127)
					fprintf(disasm_file,"\'%c\'",u);
				if (l>=32 & l<=127)
					fprintf(disasm_file,"\'%c\'",l);
			} else {
				fprintf(disasm_file,"UNKN: ");
				if (u>=32 & u<=127)
					fprintf(disasm_file,"\'%c\'",u);
				if (l>=32 & l<=127)
					fprintf(disasm_file,"\'%c\'",l);

				fprintf(disasm_file,"          ");								
				OpToStr(disasm_str, sizeof(disasm_str),(*p_DIS)[i]->theword);			
				fprintf(disasm_file,"%% %s",disasm_str);
			}
			fprintf(disasm_file,"\n");
		}
	}

	fclose(disasm_file);
}


/* 
 * Mask out instruction opcode from parameters
 * NOTE: Manual does not specify details about how ND100
 * maps all bits and some opcodes will miss this decode.
 * This means we will not completely emulate ND100 behaviour for
 * illegal opcodes yet.
 */
ushort extract_opcode(ushort instr) {
	switch(instr & (0xFFFF<<11)) {
	case 0130000:			/* JAP, JAN, JAZ, JAF, JPC, JNC, JXZ, JXN */
		return(instr & (0xFFFF<<8));
	case 0140000:	
		if(0 == (instr & (0x03<<6)))	/* SKIP Instruction */
			return 0140000;
		else 				/* Decode further */
			return(decode_140k(instr));
	case 0150000:				/* Decode further */
			return(decode_150k(instr));
	case 0144000:				/* ROP Register operations */
		return instr & (0xFFFF<<6);
	case 0154000:				/* Shift Instruction */
		return instr & ((0xFFFF<<11) | (0x03<<7));
	case	0170000:		/* Argument Instructions */
		return instr & (0xFFFF<<8);
	case	0174000:		/* Bit Operation Instructions */
		return instr & (0xFFFF<<7); /* Bit operations, 16 of them, 4 BSET,4 BSKP and 8 others */
	default:				/* Memory Reference Instructions & IOX */
		return instr & (0xFFFF<<11);
	}
	return instr;	//:NOTE: This should not be reached. Added only to satisfy complaining compiler.
}

ushort decode_140k(ushort instr) {
	switch (instr & (0xFFFF)) {
	case 0140120: /* ADDD */
	case 0140121: /* SUBD */
	case 0140122: /* COMD */
	case 0140123: /* TSET */
	case 0140124: /* PACK */
	case 0140125: /* UPACK */
	case 0140126: /* SHDE */
	case 0140127: /* RDUS */
	case 0140130: /* BFILL */
	case 0140131: /* MOVB */
	case 0140132: /* MOVBF */
		return(instr);
	case 0140133: /* VERSN - ND110 specific */
		if ((CurrentCPUType == ND110) || (CurrentCPUType == ND110CE) || (CurrentCPUType == ND110CX))
			return(instr);
		else
			break;
	case 0140134: /* INIT */
	case 0140135: /* ENTR */
	case 0140136: /* LEAVE */
	case 0140137: /* ELEAV */
	case 0140300: /* SETPT */
	case 0140301: /* CLEPT */
	case 0140302: /* CLNREENT */
	case 0140303: /* CHREENT-PAGES */
	case 0140304: /* CLEPU */
	case 0143500: /* LWCS */
	case 0143604: /* IDENT PL10 */
	case 0143611: /* IDENT PL11 */
	case 0143622: /* IDENT PL12 */
	case 0143643: /* IDENT PL13 */
		return(instr);
	default:
		break;
        }
	switch (instr & (0xFFFF<<6)) {
	case 0140200: /* USER1 (microcode defined by user or illegal instruction otherwise) */
		return instr & (0xFFFF<<6);	
	case 0140500: /* USER2 (microcode defined by user or illegal instruction otherwise) */
		if ((CurrentCPUType == ND100) || (CurrentCPUType == ND100CE) || (CurrentCPUType == ND100CX)) /* We are ND100 */
			return instr & (0xFFFF<<6);	
		else switch (instr & (0xFFFF)) {							/* We are not */
			case 0140500: /* WGLOB - ND110 Specific */
			case 0140501: /* RGLOB - ND110 Specific */
			case 0140502: /* INSPL - ND110 Specific */
			case 0140503: /* REMPL - ND110 Specific */
			case 0140504: /* CNREK - ND110 Specific */
			case 0140505: /* CLPT  - ND110 Specific */
			case 0140506: /* ENPT  - ND110 Specific */
			case 0140507: /* REPT  - ND110 Specific */
			case 0140510: /* LBIT  - ND110 Specific */
			case 0140513: /* SBITP - ND110 Specific */
			case 0140514: /* LBYTP - ND110 Specific */
			case 0140515: /* SBYTP - ND110 Specific */
			case 0140516: /* TSETP - ND110 Specific */
			case 0140517: /* RDUSP - ND110 Specific */
				return(instr);
			default:
				break;
		}
	case 0140600: /* EXR */
		return instr & (0xFFFF<<6);	
	case 0140700: /* USER3 (microcode defined by user or illegal instruction otherwise) */
		if ((CurrentCPUType == ND100) || (CurrentCPUType == ND100CE) || (CurrentCPUType == ND100CX)) /* We are ND100 */
			return instr & (0xFFFF<<6);	
		else switch (instr & (0xFFC7)) {							/* We are not */
			case 0140700: /* LASB - ND110 Specific */
			case 0140701: /* SASB - ND110 Specific */
			case 0140702: /* LACB - ND110 Specific */
			case 0140703: /* SASB - ND110 Specific */
			case 0140704: /* LXSB - ND110 Specific */
			case 0140705: /* LXCB - ND110 Specific */
			case 0140706: /* SZSB - ND110 Specific */
			case 0140707: /* SZCB - ND110 Specific */
				return(instr & (0xFFC7));
			default:
				break;
		}
	case 0141100: /* USER4 (microcode defined by user or illegal instruction otherwise) */
	case 0141200: /* RMPY */
	case 0141300: /* USER5 (microcode defined by user or illegal instruction otherwise) */
	case 0141500: /* USER6 (microcode defined by user or illegal instruction otherwise) */
	case 0141600: /* RDIV */
	case 0141700: /* USER7 (microcode defined by user or illegal instruction otherwise) */
	case 0142100: /* USER8 (microcode defined by user or illegal instruction otherwise) */
	case 0142200: /* LBYT */
	case 0142300: /* USER9 (microcode defined by user or illegal instruction otherwise) */
	case 0142500: /* USER10 (microcode defined by user or illegal instruction otherwise) */
	case 0142600: /* SBYT */
	case 0142700: /* GECO - Undocumented instruction */
	case 0143100: /* MOVEW */
	case 0143200: /* MIX3 */
		return instr & (0xFFFF<<6);	
	default:
		break;
	}
	switch (instr & (0xFFC7)) {
	case 0143300: /* LDATX */
	case 0143301: /* LDXTX */
	case 0143302: /* LDDTX */
	case 0143303: /* LDBTX */
	case 0143304: /* STATX */
	case 0143305: /* STZTX */
	case 0143306: /* STDTX */
		return (instr & (0xFFC7));
	default:
		break;
	}
	return instr;	/* NOTE: This should not be reached, but decoding might not be totally exhaustive.*/
	/* TODO: Check if we should return NOOP, or create our own internal illegal instruction code and trap that later. */
}

ushort decode_150k(ushort instr) {
	switch (instr & (0xFFFF)) {
	case 0150400 : /* OPCOM */
	case 0150401 : /* IOF */
	case 0150402 : /* ION */
	case 0150404 : /* POF */
	case 0150405 : /* PIOF */
	case 0150406 : /* SEX */
	case 0150407 : /* REX */
	case 0150410 : /* PON */
	case 0150412 : /* PION */
	case 0150415 : /* IOXT */
	case 0150416 : /* EXAM */
	case 0150417 : /* DEPO */
		return (instr);	
	default:
		break;
	}
	switch (instr & (0xFFFF<<8)) {
	case 0151000 : /* WAIT */
	case 0151400 : /* NLZ*/
	case 0152000 : /* DNZ*/
	case 0153000 : /* MON */
		return (ushort)(instr & (0xFFFF<<8));	
	default:
		break;
	}
	switch (instr & (0xFFFF<<7)) {
	case 0152400 : /* SRB */ /* NOTE: These two seems to have bit req on 0-2 as well */
	case 0152600 : /* LRB */
	case 0153400 : /* IRW */
	case 0153600 : /* IRR */
		return instr & (0xFFFF<<7);	
	default:
		break;
	}

	switch (instr & (0xFFFF<<6)) {
	case 0150000 : /* TRA */
	case 0150100 : /* TRR */
	case 0150200 : /* MCL */
	case 0150300 : /* MST */
		return instr & (0xFFFF<<6);	
	default:
		break;
	}
	return instr;	/* NOTE: This should not be reached, but decoding might not be totally exhaustive.*/
	/* TODO: Check if we should return NOOP, or create our own internal illegal instruction code and trap that later. */
}
