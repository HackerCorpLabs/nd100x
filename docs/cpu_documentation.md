================================================================================
#                              CPU Documentation                                #
================================================================================

## 📑 Table of Contents

### 🔍 CPU Information
- [CPU Overview](#cpu-overview)
- [Architecture](#architecture)

### ⚙️ Registers
- [General Purpose Registers](#general-purpose-registers)
- [Special Registers](#special-registers)

### 📚 Instruction Classes
- [memory_transfer](#memory_transfer)
- [jump_on_condition](#jump_on_condition)
- [register_block](#register_block)
- [register_src_dst](#register_src_dst)
- [register_dst](#register_dst)
- [register_arithmetic](#register_arithmetic)
- [register_logical](#register_logical)
- [bit_instructions](#bit_instructions)
- [bit_instructions_with_condition](#bit_instructions_with_condition)
- [shift_instructions](#shift_instructions)

### 🏠 Addressing Modes
- [Addressing Modes](#addressing-modes)

### 💻 Instructions by Category
- [Argument Instruction](#argument-instruction)
- [Arithmetic and Logical](#arithmetic-and-logical)
- [Bit instructions](#bit-instructions)
- [Bit instructions_with_destination](#bit-instructions_with_destination)
- [Byte Instructions](#byte-instructions)
- [Control Instructions](#control-instructions)
- [Decimal Instructions](#decimal-instructions)
- [Execute](#execute)
- [Floating Conversion (Standard Format)](#floating-conversion-(standard-format))
- [Input and Output](#input-and-output)
- [Inter-level Instructions](#inter-level-instructions)
- [Interrupt Control Instructions](#interrupt-control-instructions)
- [Memory Examine and Test Instructions](#memory-examine-and-test-instructions)
- [Memory Management Instructions](#memory-management-instructions)
- [Memory Transfer - Double word instructions](#memory-transfer---double-word-instructions)
- [Memory Transfer - Load Instruction](#memory-transfer---load-instruction)
- [Memory Transfer - Store Instruction](#memory-transfer---store-instruction)
- [Memory Transfer Instructions](#memory-transfer-instructions)
- [Monitor Calls](#monitor-calls)
- [Physical Memory Control Instructions](#physical-memory-control-instructions)
- [privileged](#privileged)
- [Register Block Instructions](#register-block-instructions)
- [Register Operations](#register-operations)
- [Register Transfer](#register-transfer)
- [Sequencing Instructions](#sequencing-instructions)
- [Shift Instructions](#shift-instructions)
- [Skip Instruction](#skip-instruction)
- [Stack Operations](#stack-operations)
- [Standard Floating Instructions](#standard-floating-instructions)
- [System/CPU Information](#system/cpu-information)
- [Undocumented Instructions](#undocumented-instructions)
- [Word Block Instructions](#word-block-instructions)

### 📖 Instruction Reference
- [172400] [AAA](#aaa)
- [172000] [AAB](#aab)
- [173000] [AAT](#aat)
- [173400] [AAX](#aax)
- [060000] [ADD](#add)
- [140120] [ADDD](#addd)
- [070000] [AND](#and)
- [177000] [BANC](#banc)
- [177200] [BAND](#band)
- [140130] [BFILL](#bfill)
- [176600] [BLDA](#blda)
- [176400] [BLDC](#bldc)
- [177600] [BORA](#bora)
- [177400] [BORC](#borc)
- [174000] [BSET](#bset)
- [175000] [BSKP](#bskp)
- [176200] [BSTA](#bsta)
- [176000] [BSTC](#bstc)
- [140303] [CHREENTPAGES](#chreentpages)
- [140301] [CLEPT](#clept)
- [140304] [CLEPU](#clepu)
- [140302] [CLNREENT](#clnreent)
- [140505] [CLPT](#clpt)
- [140504] [CNREK](#cnrek)
- [140122] [COMD](#comd)
- [146100] [COPY](#copy)
- [150417] [DEPO](#depo)
- [152000] [DNZ](#dnz)
- [140137] [ELEAV](#eleav)
- [140506] [ENPT](#enpt)
- [140135] [ENTR](#entr)
- [150416] [EXAM](#exam)
- [146142] [EXIT](#exit)
- [140600] [EXR](#exr)
- [100000] [FAD](#fad)
- [114000] [FDV](#fdv)
- [110000] [FMU](#fmu)
- [104000] [FSB](#fsb)
- [142700] [GECO](#geco)
- [143600] [IDENT](#ident)
- [140134] [INIT](#init)
- [140502] [INSPL](#inspl)
- [150401] [IOF](#iof)
- [150402] [ION](#ion)
- [160000] [IOT](#iot)
- [164000] [IOX](#iox)
- [150415] [IOXT](#ioxt)
- [153600] [IRR](#irr)
- [153400] [IRW](#irw)
- [131400] [JAF](#jaf)
- [130400] [JAN](#jan)
- [130000] [JAP](#jap)
- [131000] [JAZ](#jaz)
- [124000] [JMP](#jmp)
- [132400] [JNC](#jnc)
- [132000] [JPC](#jpc)
- [134000] [JPL](#jpl)
- [133400] [JXN](#jxn)
- [133000] [JXZ](#jxz)
- [140702] [LACB](#lacb)
- [140700] [LASB](#lasb)
- [140510] [LBIT](#lbit)
- [140511] [LBITP](#lbitp)
- [142200] [LBYT](#lbyt)
- [140514] [LBYTP](#lbytp)
- [044000] [LDA](#lda)
- [143300] [LDATX](#ldatx)
- [143303] [LDBTX](#ldbtx)
- [024000] [LDD](#ldd)
- [143302] [LDDTX](#lddtx)
- [034000] [LDF](#ldf)
- [050000] [LDT](#ldt)
- [054000] [LDX](#ldx)
- [143301] [LDXTX](#ldxtx)
- [140136] [LEAVE](#leave)
- [152600] [LRB](#lrb)
- [143500] [LWCS](#lwcs)
- [140705] [LXCB](#lxcb)
- [140704] [LXSB](#lxsb)
- [150200] [MCL](#mcl)
- [040000] [MIN](#min)
- [143200] [MIX3](#mix3)
- [153000] [MON](#mon)
- [140131] [MOVB](#movb)
- [140132] [MOVBF](#movbf)
- [143100] [MOVEW](#movew)
- [120000] [MPY](#mpy)
- [150300] [MST](#mst)
- [151400] [NLZ](#nlz)
- [150400] [OPCOM](#opcom)
- [074000] [ORA](#ora)
- [140124] [PACK](#pack)
- [150405] [PIOF](#piof)
- [150412] [PION](#pion)
- [150404] [POF](#pof)
- [150410] [PON](#pon)
- [146000] [RADD](#radd)
- [144400] [RAND](#rand)
- [146100] [RCLR](#rclr)
- [146200] [RDCR](#rdcr)
- [141600] [RDIV](#rdiv)
- [140127] [RDUS](#rdus)
- [140517] [RDUSP](#rdusp)
- [140503] [REMPL](#rempl)
- [140507] [REPT](#rept)
- [150407] [REX](#rex)
- [145000] [REXO](#rexo)
- [140501] [RGLOB](#rglob)
- [146400] [RINC](#rinc)
- [141200] [RMPY](#rmpy)
- [145400] [RORA](#rora)
- [146600] [RSUB](#rsub)
- [170400] [SAA](#saa)
- [170000] [SAB](#sab)
- [140703] [SACB](#sacb)
- [154600] [SAD](#sad)
- [140701] [SASB](#sasb)
- [171000] [SAT](#sat)
- [171400] [SAX](#sax)
- [140512] [SBIT](#sbit)
- [140513] [SBITP](#sbitp)
- [142600] [SBYT](#sbyt)
- [140515] [SBYTP](#sbytp)
- [140300] [SETPT](#setpt)
- [150406] [SEX](#sex)
- [154400] [SHA](#sha)
- [154200] [SHD](#shd)
- [140126] [SHDE](#shde)
- [154000] [SHT](#sht)
- [140000] [SKP](#skp)
- [152402] [SRB](#srb)
- [004000] [STA](#sta)
- [143304] [STATX](#statx)
- [020000] [STD](#std)
- [143306] [STDTX](#stdtx)
- [030000] [STF](#stf)
- [010000] [STT](#stt)
- [014000] [STX](#stx)
- [000000] [STZ](#stz)
- [143305] [STZTX](#stztx)
- [064000] [SUB](#sub)
- [140121] [SUBD](#subd)
- [144000] [SWAP](#swap)
- [140707] [SZCB](#szcb)
- [140706] [SZSB](#szsb)
- [150000] [TRA](#tra)
- [150100] [TRR](#trr)
- [140123] [TSET](#tset)
- [140516] [TSETP](#tsetp)
- [140125] [UPACK](#upack)
- [140133] [VERSN](#versn)
- [151000] [WAIT](#wait)
- [140500] [WGLOB](#wglob)

================================================================================
## 🔍 CPU Information
================================================================================

### ND-100

**Word Size:** 16 bits
**Endianness:** big

================================================================================
## ⚙️ Registers
================================================================================

| Name | Description | Size | Type |
|:-----|:------------|:-----|:-----|
| `STS` | Status register (user) | 16 bits |  |
| `D` | Data register | 16 bits |  |
| `P` | Program counter | 16 bits |  |
| `B` | Base register for addressing | 16 bits |  |
| `L` | Link register | 16 bits |  |
| `A` | Accumulator register | 16 bits |  |
| `T` | Temporary register | 16 bits |  |
| `X` | Index register | 16 bits |  |

================================================================================
## 📚 Instruction Classes
================================================================================

### memory_transfer

Memory transfer instructions specify memory transfers.
These are the only instructions that use the addressing modes in their format.


**Format:** `<opcode> <address mode> <disp>`
**Mask:** `1111_1000_0000_0000`

--------------------------------------------------------------------------------

### jump_on_condition

Jump on condition instructions specify a jump to a specified address if a condition is met.
The condition is specified by the condition code bits in the status register.


**Format:** `<opcode> <displacement>`
**Mask:** `1111_1111_0000_0000`

--------------------------------------------------------------------------------

### register_block

Register block instructions specify a register block to be loaded from memory.


**Format:** `<opcode> <level> <type>`
**Mask:** `1111_1111_1000_0000`

--------------------------------------------------------------------------------

### register_src_dst

Class where we can specify a source and destination register

**Format:** `<opcode> <source> <destination>`
**Mask:** `1111_1111_1100_0000`

--------------------------------------------------------------------------------

### register_dst

Class where we can specify a destination register

**Format:** `<opcode> <destination>`
**Mask:** `1111_1111_1111_1000`

--------------------------------------------------------------------------------

### register_arithmetic

Register arithmetic instructions

**Format:** `<opcode>  <ADC> <AD1> <CM1> <CM2> <source> <destination>`
**Mask:** `1111_1100_0000_0000`

--------------------------------------------------------------------------------

### register_logical

Register logical instructions

**Format:** `<opcode>  <ADC> <AD1> <CM1> <CM2> <source> <destination>`
**Mask:** `1111_1100_0000_0000`

--------------------------------------------------------------------------------

### bit_instructions

Bit instructions

**Format:** `<opcode> <function> <bit_no> <destination>`
**Mask:** ``

--------------------------------------------------------------------------------

### bit_instructions_with_condition

Bit instructions

**Format:** `<opcode> <condition> <bit_no> <destination>`
**Mask:** ``

--------------------------------------------------------------------------------

### shift_instructions

Shift instructions


**Format:** `<opcode> <shift_type> <shift_counter>`
**Mask:** `1111_1000_0000_0000`

--------------------------------------------------------------------------------

================================================================================
## 🏠 Addressing Modes
================================================================================

These three bits give the addressing mode for the instruction

### Bit Structure

The addressing mode is encoded in the instruction word and consists of the following subfields:

| Bit | Name | Description |
|-----|------|-------------|
| 10 | X | Index register flag |
| 9 | I | Indirect addressing flag |
| 8 | B | Base register flag |

#### Effects when Bits are Set

- **X**: Address is indexed by X register
- **I**: Address is indirect
- **B**: Base-relative instead of P-relative

#### Syntax Variations

When multiple bits are set, the assembly syntax changes as follows:

- When X=1: Add `,X` prefix of operand
- When I=1: Add `I` prefix of operand
- When B=1: Add `,B` suffix of operand

### Summary of Addressing Modes

| Mode | Value | Format | Example | Effective Address |
|------|-------|--------|---------|------------------|
| prelative | 0 | `<disp>` | `STA *2` | (P) + disp |
| brelative | 1 | `<disp>, B` | `LDA -4,B` | (B) + disp |
| indirect_prelative | 2 | `I <disp>` | `LDA I *22` | ((P) + disp) |
| indirect_brelative | 3 | `I <disp> ,B` | `JPL I 3,B` | ((B) + disp) |
| xrelative | 4 | `<disp>,X` | `LDA 0,X` | (X) + disp |
| brelative_indexed | 5 | `<disp>,B,X` | `LDA 17,B ,X` | (B) + disp + (X) |
| indirect_prelative_indexed | 6 | `,X I <disp>` | `LDA ,X I *4` | ((P) + disp) + (X) |
| indirect_brelative_indexed | 7 | `,X I ,B <disp>` | `LDA ,X I ,B *4` | ((B) + disp) + (X) |

See [Addressing Modes](addressing_modes.md) for more detailed information.

--------------------------------------------------------------------------------

================================================================================
## 💻 Instructions by Category
================================================================================

### Argument Instruction

| Instruction | Description |
|:------------|:------------|
| [`AAA`](#aaa) | Add argument to A |
| [`AAB`](#aab) | Add argument to B |
| [`AAT`](#aat) | Add argument to T |
| [`AAX`](#aax) | Add argument to X |
| [`SAA`](#saa) | Set argument to A |
| [`SAB`](#sab) | Set argument to B |
| [`SAT`](#sat) | Set argument to T |
| [`SAX`](#sax) | Set argument to X |


#### Description

These instructions operate on registers.

The <number> is sign extended.

8-bit argument numbers are extended to 16-bits using Sign extension.
  
  The 8-bit argument becomes the least significant byte
  The higher byte is extended with ones or zeros.

Positive arguments have the higher byte extended with zeros
Nnegative numbers are extended with ones with the argument in 2's complement form.
--------------------------------------------------------------------------------

### Arithmetic and Logical

| Instruction | Description |
|:------------|:------------|
| [`ADD`](#add) | Add to A register |
| [`AND`](#and) | Logical AND to A register |
| [`MPY`](#mpy) | Multiply integer |
| [`ORA`](#ora) | Logical inclusive OR to A register |
| [`SUB`](#sub) | Subtract from A register |

--------------------------------------------------------------------------------

### Bit instructions

| Instruction | Description |
|:------------|:------------|
| [`BANC`](#banc) | ogical AND with bit compl |
| [`BAND`](#band) | Logical AND to K |
| [`BLDA`](#blda) | Load K |
| [`BLDC`](#bldc) | Load K and complement |
| [`BORA`](#bora) | Logical OR to K |
| [`BSET`](#bset) | Set specified bit in equal to specified condition |
| [`BSKP`](#bskp) | Skip next location if specified condition is true |
| [`BSTA`](#bsta) | Store and clear K |
| [`BSTC`](#bstc) | Store complement and set K to 1 |

#### Description

These instructions manipulate single bits within the working and STS registers.

Instruction structure:

    15         7 6     3   0
    ┌──────────┬───────┬───┐
    │ bit op   │  bn   │dr │
    └──────────┴───────┴───┘

Bit operation:
  Bits 11-15 are always set to one for a bit operation.
  Bits 7-10 determine the type of operation as follows:

    ┌──────────┬────────────┬───────────────────────────────┬─────────────┐
    │type bits │ mnemonic   │ description                   │ code        │
    │10 9 8 7  │            │                               │             │
    ├──────────┼────────────┼───────────────────────────────┼─────────────┤
    │ 0 0 0 0  │ BSET ZRO   │ bit ⟵ 0                      │ 174000₈     │
    │ 0 0 0 1  │ BSET ONE   │ bit ⟵ 1                      │ 174200₈     │
    │ 0 0 1 0  │ BSET BCM   │ bit ⟵ bit                    │ 174400₈     │
    │ 0 0 1 1  │ BSET BAC   │ bit ⟵ K                      │ 174600₈     │
    │ 0 1 0 0  │ BSKP ZRO   │ skip if bit = 0               │ 175000₈     │
    │ 0 1 0 1  │ BSKP ONE   │ skip if bit = 1               │ 175200₈     │
    │ 0 1 1 0  │ BSKP BCM   │ skip if bit = bit             │ 175400₈     │
    │ 0 1 1 1  │ BSKP BAC   │ skip if bit = K               │ 175600₈     │
    │ 1 0 0 0  │ BSTC       │ bit ⟵ K, K ⟵ 1              │ 176000₈     │
    │ 1 0 0 1  │ BSTA       │ bit ⟵ K, K ⟵ 0              │ 176200₈     │
    │ 1 0 1 0  │ BLDC       │ K ⟵ bit                      │ 176400₈     │
    │ 1 0 1 1  │ BLDA       │ K ⟵ bit                      │ 176600₈     │
    │ 1 1 0 0  │ BANC       │ K ⟵ (bit AND K)              │ 177000₈     │
    │ 1 1 0 1  │ BAND       │ K ⟵ (bit AND K)              │ 177200₈     │
    │ 1 1 1 0  │ BORC       │ K ⟵ (bit OR K)               │ 177400₈     │
    │ 1 1 1 1  │ BORA       │ K ⟵ (bit OR K)               │ 177600₈     │
    └──────────┴────────────┴───────────────────────────────┴─────────────┘

**Sub-instructions:**
Only the BSET and BSKP instructions have the following qualifying sub-instructions:
    ZRO
    ONE
    BCM
    BAC

**bn (bits 3-6):**
  The address of the bit to be manipulated is given by these four bits.
  Remember that each bit is given its OCTAL address.

**dr (bits 0-2):**
  The following registers allow bit operations and are specified as follows:

    ┌──────────┬──────────┬────────┐
    │ register │ mnemonic │ code   │
    ├──────────┼──────────┼────────┤
    │ STS      │ †        │ 0₈     │
    │ D        │ DD       │ 1₈     │
    │ P        │ DP       │ 2₈     │
    │ B        │ DB       │ 3₈     │
    │ L        │ DL       │ 4₈     │
    │ A        │ DA       │ 5₈     │
    │ T        │ DT       │ 6₈     │
    │ X        │ DX       │ 7₈     │
    └──────────┴──────────┴────────┘

† For STS no mnemonic is required as it is implied by the following table of compound mnemonics:

**STS register:**
There are only eight bits which can be operated on in the STS register. They have special mnemonics and unique octal code values which combine the bn and dr fields.

    ┌────────────────┬──────────┬─────────────────────────────┬─────────────┐
    │ compound       │ STS bit  │ description                 │ octal code  │
    │ mnemonic       │          │                             │             │
    ├────────────────┼──────────┼─────────────────────────────┼─────────────┤
    │ SSPTM          │ 0        │ page table flag             │ 00₈         │
    │ SSTG           │ 1        │ floating point rounding flag│ 10₈         │
    │ SSK            │ 2        │ 1 bit accumulator (K)       │ 20₈         │
    │ SSZ            │ 3        │ error flag (Z)              │ 30₈         │
    │ SSQ            │ 4        │ dynamic overflow flag (Q)   │ 40₈         │
    │ SSO            │ 5        │ static overflow flag (O)    │ 50₈         │
    │ SSC            │ 6        │ carry flag (C)              │ 60₈         │
    │ SSM            │ 7        │ multi-shift link flag (M)   │ 70₈         │
    └────────────────┴──────────┴─────────────────────────────┴─────────────┘

--------------------------------------------------------------------------------

### Bit instructions_with_destination

| Instruction | Description |
|:------------|:------------|
| [`BORC`](#borc) | Logical OR with bit complement |

--------------------------------------------------------------------------------

### Byte Instructions

| Instruction | Description |
|:------------|:------------|
| [`BFILL`](#bfill) | Byte fill. |
| [`LBYT`](#lbyt) | Load byte from memory to A register |
| [`MOVB`](#movb) | Move byte. |
| [`MOVBF`](#movbf) | Move bytes forward. |
| [`SBYT`](#sbyt) | Store byte from A register to memory |

#### Description

These instructions address single bytes within the memory map.

A special addressing mode is used for these instructions, utilizing the T and X registers. 
The contents of T point to the beginning of a character string, and the contents of X point to a specific byte within that string.


--------------------------------------------------------------------------------

### Control Instructions

| Instruction | Description |
|:------------|:------------|
| [`CHREENTPAGES`](#chreentpages) | Change page tables. |
| [`CLEPT`](#clept) | Clear page tables. |
| [`CLEPU`](#clepu) | Clear page tables and collect PGU information. |
| [`CLNREENT`](#clnreent) | Clear non reentrant pages. |
| [`CLPT`](#clpt) | Clear segment from the page tables. |
| [`CNREK`](#cnrek) | Clear non reentrant pages (SINTRAN K only).     |
| [`ENPT`](#enpt) | Enter segment in page tables.  |
| [`INSPL`](#inspl) | Insert page in page list. (See Appendix B for a software description.) |
| [`LACB`](#lacb) | Load the A register from the core map-table bank (CMBNK). |
| [`LASB`](#lasb) | Load the A register with the contents of the segment-table bank (STBNK). |
| [`LBIT`](#lbit) | Load single bit accumulator (K) with logical memory bit. |
| [`LBITP`](#lbitp) | Load single bit accumulator (K) with physical memory bit. |
| [`LBYTP`](#lbytp) | Load the A register with a byte from physical memory. |
| [`LXCB`](#lxcb) | Load the X register from the core table bank (CMBNK). |
| [`LXSB`](#lxsb) | Load the X register from the segment table bank (STBNK). |
| [`RDUSP`](#rdusp) | Read a physical memory word without using cache. |
| [`REMPL`](#rempl) | Remove page from page list. |
| [`REPT`](#rept) | Enter reentrant segment in page tables. (See Appendix B for a software description.) |
| [`RGLOB`](#rglob) | Examine global pointers. |
| [`SACB`](#sacb) | Store the A register in the core map table bank (CMBNK). |
| [`SASB`](#sasb) | Store the A register contents in the segment table bank (STBNK). |
| [`SBIT`](#sbit) | Store the single bit accumulator (K) in a logical memory bit. |
| [`SBITP`](#sbitp) | Store the single bit accumulator (K) in a physical memory bit. |
| [`SBYTP`](#sbytp) | Store a byte in physical memory. |
| [`SETPT`](#setpt) | Set page tables. |
| [`SZCB`](#szcb) | Store zero in the core map-table bank (CMBNK). |
| [`SZSB`](#szsb) | Store zero in the segment-table bank (STBNK). |
| [`TSETP`](#tsetp) | Test and set physical memory word. |
| [`WGLOB`](#wglob) | Initialize global pointers. |

#### Description

### SINTRAN III Control Instructions

These instructions are **PRIVILEGED** and only available to:

- Programs running in system mode (rings 2-3)
- Programs running without memory protection

These instructions monitor the contents of physical memory

--------------------------------------------------------------------------------

### Decimal Instructions

| Instruction | Description |
|:------------|:------------|
| [`ADDD`](#addd) | Add two decimal operands. |
| [`COMD`](#comd) | Compare two decimal operands. |
| [`PACK`](#pack) | Convert to BCD. |
| [`SHDE`](#shde) | Decimal shift. |
| [`SUBD`](#subd) | Subtract two decimal operands. |
| [`UPACK`](#upack) | Convert to ASCII. |

### Description

These instructions use decimal operands residing in main memory only.


**Instruction structure:**

    D1 (word address)
    ┌───────────────────────────────┐
    │15           ...            0  │
    │ word address                  │
    └───────────────────────────────┘

    D2 (operand features)
    ┌────┬────┬───────┬───────┬──────────────┬──────────────┐
    │15  │14  │13-11  │10     │  9  -    5   │  4  -    0   │
    ├────┼────┼───────┼───────┼──────────────┼──────────────┤
    │lr  │ •  │ASCII  │r      │decimal point | field length │    
    └────┴────┴───────┴───────┴──────────────┴──────────────┘

**Descriptors (D1 and D2):**
- Two 16-bit words (D1 and D2) specify the operands used in decimal instructions.
- D1: The first descriptor gives the word address of the decimal operand in memory.
- D2: Describes the following operand features:
  - **lr (bit 15):**
    - lr = 0: The operand starts in the left byte of a memory word (least significant 8 bits).
    - lr = 1: The operand starts in the right byte of a memory word (most significant 8 bits).
  - **ASCII (bits 11-13):**
    - These three bits give the sign representation used for ASCII format:

        ┌─────┬───────────────────────┐
        │bits │ sign representation   │
        ├─────┼───────────────────────┤
        │000  │ embedded trailing     │
        │001  │ separate trailing     │
        │010  │ embedded leading      │
        │011  │ separate leading      │
        │100  │ unsigned              │
        └─────┴───────────────────────┘

    - Bit 13 also represents an unsigned number in BCD representation.
  - **r (bit 10):**
    - Rounding bit. If rounding is selected, one is added to the shifted operand when the least significant digit is lost during shift and the last digit shifted out of the field is 25.
    - r = 1: rounding on
    - r = 0: rounding off
  - **decimal point (bits 5-9):**
    - These bits give the position of the decimal point (0 to 31, positive or negative). Zero is the decimal place to the right of the least significant digit. The number must be less than the operand field length.
  - **field length (bits 0-4):**
    - These bits give the operand field length in nibbles (4-bit values) or bytes (8-bit values). BCD numbers are represented by 4 bits (1 nibble), so the field length will be in nibbles; an ASCII coded digit is represented by a byte and the field length will be in bytes.
    - Operands start at any byte address in memory.
    - The maximum field length is 32 nibbles/bytes.

**Decimal operands:**
- Decimal operands occupy a maximum of eight 16-bit memory locations. Each operand consists of BCD coded numbers.
- Decimal operands must be right adjusted so that the least significant digit and sign are in the last byte of the operand field.
- Before any instruction is executed, the operands are read into the register file. The result of the instruction is written into memory.
- All decimal instructions use two operands. The descriptors of each operand are held in separate registers:
  - First operand descriptor: A and D registers
  - Second operand descriptor: X and T registers

**Decimal overflow:**
- Decimal overflow is caused by:
  - EITHER a carry from the most significant digit position in the result
  - OR an oversized result, where the second operand was larger than the first, causing the significant digits of the result to be lost.
  - Note: The field size alone does not indicate possible overflow.
- Most decimal instructions are followed by an instruction or jump to a routine which takes care of overflow errors, known as an **error return**. A decimal instruction executed without error generation skips the error return and program execution continues at the second instruction after it.


### BCD- Binary Coded Decimal

Decimal digits are represented in binary-coded decimal (BCD), sometimes known as packed decimal.

Four bits are used to represent a decimal digit:

    Table 3. BCD notation

    ┌───────────────────────┬───────────────────────┐
    │ binary notation       │ decimal equivalent    │
    │ msb         lsb       │                       │
    ├───────────────────────┼───────────────────────┤
    │ 0  0  0  0            │ 1                     │
    │ 0  0  0  1            │ 2                     │
    │ 0  0  1  0            │ 3                     │
    │ 0  0  1  1            │ 4                     │
    │ 0  1  0  0            │ 5                     │
    │ 0  1  0  1            │ 6                     │
    │ 0  1  1  0            │ 7                     │
    │ 0  1  1  1            │ 8                     │
    │ 1  0  0  0            │ 9                     │
    │ 1  0  0  1            │ 10                    │
    │ 1  0  1  0            │ +                     │
    │ 1  0  1  1            │ -                     │
    │ 1  1  0  0            │ + †                   │
    │ 1  1  0  1            │ - †                   │
    │ 1  1  1  0            │ +                     │
    │ 1  1  1  1            │ (+)                   │
    └───────────────────────┴───────────────────────┘

(+) represents unsigned, it is treated as a plus.

† The ND-110 instruction set uses only the codes 1100 for plus and 1101 for minus.

The maximum length of an operand is 31 decimal digits plus a sign nibble (4 bits), this occupies eight consecutive memory locations (eight 16-bit words).

### ASCII coded decimal

ASCII-coded decimal notation uses eight bits to represent a decimal digit.

The format of an ASCII code decimal is:

    ┌───────┬───────┐
    │ zone  │ digit │
    └───────┴───────┘

*Figure 5. ASCII byte structure*

**Table 4. ASCII notation**

    ┌───────────────────────────────┬───────────────────────┐
    │ ASCII Code                    │ Decimal Equivalent    │
    │ msb                lsb       │                       │
    ├───────────────────────────────┼───────────────────────┤
    │ 0 0 1 1 0 0 0 0              │ 0                     │
    │ 0 0 1 1 0 0 0 1              │ 1                     │
    │ 0 0 1 1 0 0 1 0              │ 2                     │
    │ 0 0 1 1 0 0 1 1              │ 3                     │
    │ 0 0 1 1 0 1 0 0              │ 4                     │
    │ 0 0 1 1 0 1 0 1              │ 5                     │
    │ 0 0 1 1 0 1 1 0              │ 6                     │
    │ 0 0 1 1 0 1 1 1              │ 7                     │
    │ 0 0 1 1 1 0 0 0              │ 8                     │
    │ 0 0 1 1 1 0 0 1              │ 9                     │
    └───────────────────────────────┴───────────────────────┘

Bit 7 (msb) is the parity bit and is always zero in ASCII code.

**Sign representation:**

The ASCII notation for sign is as follows:

    +   00101011   53₈
    -   00101101   55₈

There are four ways of representing the sign in a decimal operand:

- **separate trailing**: The byte following the last significant digit contains the sign.
- **separate leading**: The byte preceding the ASCII digit code contains the sign.
- **embedded trailing**: The byte representing the least significant decimal digit also contains the sign.
- **embedded leading**: The byte representing the most significant digit also contains the sign.
- **embedded sign coding**: The embedded codes are represented by ASCII notation as follows:

*Table 5. ASCII embedded notation*

    ASCII value
    
    ┌───────────────┬──────────────────┬───────────────────┐
    │ decimal       │ Positive sign    │ negative sign     │    
    │ operand       │ octal |  binary  |  octal |  binary  │
    ├───────────────┼───────┼──────────┼────────┼──────────┤
    │ 0             │ 173   │ 01111011 │  175   │ 01111001 │
    │ 1             │ 101   │ 01000001 │  112   │ 01001010 │
    │ 2             │ 102   │ 01000010 │  113   │ 01001011 │
    │ 3             │ 103   │ 01000011 │  114   │ 01001100 │
    │ 4             │ 104   │ 01000100 │  115   │ 01001101 │
    │ 5             │ 105   │ 01000101 │  116   │ 01001110 │
    │ 6             │ 106   │ 01000110 │  117   │ 01001111 │
    │ 7             │ 107   │ 01000111 │  120   │ 01010000 │
    │ 8             │ 110   │ 01001000 │  121   │ 01010001 │
    │ 9             │ 111   │ 01001001 │  122   │ 01010010 │
    └───────────────┴───────┴──────────┴────────┴──────────┘


--------------------------------------------------------------------------------

### Execute

| Instruction | Description |
|:------------|:------------|
| [`EXR`](#exr) | Execute instruction found in specified register |

--------------------------------------------------------------------------------

### Floating Conversion (Standard Format)

| Instruction | Description |
|:------------|:------------|
| [`DNZ`](#dnz) | Denormalise      |
| [`NLZ`](#nlz) | Normalize |

--------------------------------------------------------------------------------

### Input and Output

| Instruction | Description |
|:------------|:------------|
| [`IOT`](#iot) | NORD-1 INSTRUCTION (DO NOT USE) |
| [`IOX`](#iox) | Exchange information between I/O system and A register. |
| [`IOXT`](#ioxt) | Exchange information between I/O system and A register. |


--------------------------------------------------------------------------------

### Inter-level Instructions

| Instruction | Description |
|:------------|:------------|
| [`IRR`](#irr) | Inter Register Read |
| [`IRW`](#irw) | Inter Register Write |

--------------------------------------------------------------------------------

### Interrupt Control Instructions

| Instruction | Description |
|:------------|:------------|
| [`IDENT`](#ident) | Transfer IDENT code of interrupting device with highest priority on the specified level to A register.  |
| [`IOF`](#iof) | Interrupt System OFF |
| [`ION`](#ion) | Interrupt System ON |
| [`OPCOM`](#opcom) | Operator Communication |
| [`POF`](#pof) | Memory management OFF |
| [`WAIT`](#wait) | When interrupt system off: halts the program and enters the operator's communication.  |


### INTERRUPT CONTROL INSTRUCTIONS

These instructions are **PRIVILEGED** and only available to:

- Programs running in system mode (rings 2-3)
- Programs running without memory protection

These instructions control the CPU interrupt system.


---

## General Description

The ND-110 has a priority interrupt system with 16 program levels. 

Each program level has its own set of working registers (A, B, D, L, P, STS, T, X). 

The program levels have increasing priority, that is, program level 15 has the highest priority and program level 0 the lowest.

The 16 levels are subdivided as follows:

| level | used for                           | controlled by        |
|-------|------------------------------------|----------------------|
| 15    | very fast user interrupts          | program/ext. device  |
| 14    | internal hardware status interrupts| program/ext. device  |
| 13-10 | vectored interrupts*               | program/ext. device  |
| 9-0   | system and user programs           | program              |

*2048 possible sources

---

### Program level selection and control

Program level selection and control is via two 16-bit registers:

- **PID**: Priority Interrupt Detect
- **PIE**: Priority Interrupt Enable

PID is affected by program and external interrupts; PIE is controlled by program only. They can only be changed or monitored by the privileged instructions: `TRA`, `TRR`, `MST`, and `MCL` (see pages 118-120).

**Note:**
When the power is turned on, the power-up sequence resets PIE and PID so that program level 0 is selected.

---

### Interrupt programming is via three registers:

- **IIC**: Internal Interrupt Code
- **IIE**: Internal Interrupt Enable
- **PVL**: Previous Level (of hardware interrupt source)


--------------------------------------------------------------------------------

### Memory Examine and Test Instructions

| Instruction | Description |
|:------------|:------------|
| [`RDUS`](#rdus) | Read a word without using cache |
| [`TSET`](#tset) | Test and set |

--------------------------------------------------------------------------------

### Memory Management Instructions

| Instruction | Description |
|:------------|:------------|
| [`PIOF`](#piof) | Memory management and interrupt system OFF |
| [`PION`](#pion) | Memory management and interrupt system ON |
| [`PON`](#pon) | Memory management ON |
| [`REX`](#rex) | Reset extended address mode |
| [`SEX`](#sex) | Set extended address mode |

#### Description


These instructions are PRIVILEGED and only available to:

* programs running in system mode (rings 2-3)
* programs running without memory protection 
 
These instructions control the CPU memory management system.
--------------------------------------------------------------------------------

### Memory Transfer - Double word instructions

| Instruction | Description |
|:------------|:------------|
| [`LDD`](#ldd) | Load double word |
| [`STD`](#std) | Store double word |

--------------------------------------------------------------------------------

### Memory Transfer - Load Instruction

| Instruction | Description |
|:------------|:------------|
| [`LDA`](#lda) | Load A register |
| [`LDT`](#ldt) | Load T register |
| [`LDX`](#ldx) | Load X register |

--------------------------------------------------------------------------------

### Memory Transfer - Store Instruction

| Instruction | Description |
|:------------|:------------|
| [`MIN`](#min) | Memory increment and skip next instruction if zero (EA): = (EA) + 1 |
| [`STA`](#sta) | Store A register to memory location  |
| [`STT`](#stt) | Store T register to memory location |
| [`STX`](#stx) | Store X register to memory location  |
| [`STZ`](#stz) | Store zero to memory location |

--------------------------------------------------------------------------------

### Memory Transfer Instructions

| Instruction | Description |
|:------------|:------------|
| [`LDATX`](#ldatx) | Load A register |
| [`LDBTX`](#ldbtx) | Load B register |
| [`LDDTX`](#lddtx) | Load double word |
| [`LDXTX`](#ldxtx) | Load X register |
| [`STATX`](#statx) | Store A register |
| [`STDTX`](#stdtx) | Store double word |
| [`STZTX`](#stztx) | Store zero |

### SINTRAN III Memory Transfer Instructions

These instructions are **PRIVILEGED** and only available to:

- Programs running in system mode (rings 2-3)
- Programs running without memory protection

These instructions read/write from/to physical memory locations independent of whether paging is on or off. If the address is within the page-table range then the page tables are affected.

---

### Instruction format

```
<physical instruction mnemonic> <displacement>
```

### Instruction structure

```
 | 15                      6 | 5          3 | 2-0 |
 |---------------------------|--------------|-----|
 | physical memory operation | displacement |type |
```

- **physical memory operation type** (bits 15-6 and 2-0):
  There are seven physical memory read/write instructions, specified by a base octal code of 143300₈ (bits 15-6) and type field (bits 2-0).
- **disp.**: The contents of the T and X register give the effective address of the physical memory location (see page 28). 
- A 3-bit displacement can be added to the X register within the instruction code. This is denoted by Δ in the following codes:

| instruction mnemonic | octal code |
|----------------------|------------|
| LDATX                | 1433Δ0     |
| LDXTX                | 1433Δ1     |
| LDDTX                | 1433Δ2     |
| LDBTX                | 1433Δ3     |
| STATX                | 1433Δ4     |
| STZTX                | 1433Δ5     |
| STDTX                | 1433Δ6     |

† If you use programs written for ND-100 computers with the microprogram version numbers 015xx A-J (48 bit) or 026xx A-F (32 bit), LDBTX would have been followed by a word containing 177777₈. 

This is not necessary for later ND-100 versions nor the ND-110. Running these earlier programs may change the status of the K bit in the ND-110 and later ND-100s.

--------------------------------------------------------------------------------

### Monitor Calls

| Instruction | Description |
|:------------|:------------|
| [`MON`](#mon) | The MON instruction is used in special different contexts when running under an operating system. |

--------------------------------------------------------------------------------

### Physical Memory Control Instructions

| Instruction | Description |
|:------------|:------------|
| [`DEPO`](#depo) | Deposit |
| [`EXAM`](#exam) | Examine |

#### Description


These instructions are PRIVILEGED and only available to:

* programs running in system mode (rings 2-3)
* programs running without memory protection 
 
These instructions monitor physical memory location contents.
--------------------------------------------------------------------------------

### privileged

| Instruction | Description |
|:------------|:------------|
| [`LWCS`](#lwcs) | Writable Control Store Instruction |

--------------------------------------------------------------------------------

### Register Block Instructions

| Instruction | Description |
|:------------|:------------|
| [`LRB`](#lrb) | Load register block |
| [`SRB`](#srb) | Store register block |

--------------------------------------------------------------------------------

### Register Operations

| Instruction | Description |
|:------------|:------------|
| [`COPY`](#copy) | Copy source to destination |
| [`EXIT`](#exit) | Return from subroutine |
| [`MIX3`](#mix3) | Multiply index by 3. |
| [`RADD`](#radd) | Add source to destination (dr): = (dr) + (sr) |
| [`RAND`](#rand) | Logical AND to destination  |
| [`RCLR`](#rclr) | Register clear |
| [`RDCR`](#rdcr) | Register decrement |
| [`RDIV`](#rdiv) | Divide double accumulator with source; quotient in A, remainder in D |
| [`REXO`](#rexo) | Logical exclusive OR  |
| [`RINC`](#rinc) | Register increment |
| [`RMPY`](#rmpy) | Multiply source with destination; result in double accumulator |
| [`RORA`](#rora) | OR register |
| [`RSUB`](#rsub) | Register subtract |
| [`SWAP`](#swap) | Register swap |

#### Description

These instructions specify operations between source (sr) and destination (dr) registers.




--------------------------------------------------------------------------------

### Register Transfer

| Instruction | Description |
|:------------|:------------|
| [`MCL`](#mcl) | Masked clear |
| [`MST`](#mst) | Masked set |
| [`TRA`](#tra) | Transfer to A register |
| [`TRR`](#trr) | Transfer A to internal register |

--------------------------------------------------------------------------------

### Sequencing Instructions

| Instruction | Description |
|:------------|:------------|
| [`JAF`](#jaf) | Condtion: Jump if (A) != 0 (jump if A filled) |
| [`JAN`](#jan) | Condtion: Jump if (A) < 0 (jump if A is negative) |
| [`JAP`](#jap) | Condtion: Jump if (A) > 0 (jump if A positive) |
| [`JAZ`](#jaz) | Condtion: Jump if (A) == 0 (jump if A is zero) |
| [`JMP`](#jmp) | Jump - Unconditional jump to specified address |
| [`JNC`](#jnc) | Increment X and jump if X is negative     |
| [`JPC`](#jpc) | Increment X and jump if X is positive     |
| [`JPL`](#jpl) | Jump if Plus - Jump to specified address if the result of the last operation was positive (sign bit is 0) |
| [`JXN`](#jxn) | Condtion: Jump if (X) < 0 (jump if X negative) |
| [`JXZ`](#jxz) | Condtion: Jump if (X) == 0 (jump if X is zero) |

#### Description

FOR ALL JUMP ON CONDITION TRUE INSTRUCTIONS:

<displacement> range: -128 to +127 locations

General description:
  (P) <- (EA)

If condition true, jump to the address of the program counter plus the value of disp.
If condition false, continue program execution at (P) + 1.
--------------------------------------------------------------------------------

### Shift Instructions

| Instruction | Description |
|:------------|:------------|
| [`SAD`](#sad) | Shift A and D registers connected |
| [`SHA`](#sha) | Shift A register |
| [`SHD`](#shd) | Shift D register |
| [`SHT`](#sht) | Shift T register |

--------------------------------------------------------------------------------

### Skip Instruction

| Instruction | Description |
|:------------|:------------|
| [`SKP`](#skp) | The next instruction is skipped if a specified condition is true. |

--------------------------------------------------------------------------------

### Stack Operations

| Instruction | Description |
|:------------|:------------|
| [`ELEAV`](#eleav) | Error leave stack. |
| [`ENTR`](#entr) | Enter stack. |
| [`INIT`](#init) | Initialize stack.  |
| [`LEAVE`](#leave) | Leave stack. |


#### Description

These instructions handle stack operations improving the execution time of high-level language-based programs.

The **B** register will always point to a "stack-frame" containing:

| Stack frame content mnemonic | Pointed to by B= (octal) | Description                            |
|-----------------------------|---------------------------|----------------------------------------|
| LINK                        | -200₈                    | next instruction address †             |
| PREVB                       | -177₈                    | previous stack frame address           |
| STP                         | -176₈                    | next stack frame address               |
| SMAX                        | -175₈                    | top of stack address                   |
| —                           | -174₈                    | reserved for system use                |
| ERRCODE                     | -173₈                    | (A) after an ELEAV instruction         |

† In the case of a `LEAVE` instruction.

The stack-handling instructions are page-fault tolerant in the ND-110.

--------------------------------------------------------------------------------

### Standard Floating Instructions

| Instruction | Description |
|:------------|:------------|
| [`FAD`](#fad) | Add to floating point accumulator |
| [`FDV`](#fdv) | Divide floating point accumulator |
| [`FMU`](#fmu) | Multiply floating point accumulator. |
| [`FSB`](#fsb) | Subtract from floating point accumulator. |
| [`LDF`](#ldf) | Load floating accumulator (TAD) from memory (FW) |
| [`STF`](#stf) | Store floating accumulator (TAD) to memory (ea) |

--------------------------------------------------------------------------------

### System/CPU Information

| Instruction | Description |
|:------------|:------------|
| [`VERSN`](#versn) | ** ND-110/ND-120 ONLY** |

--------------------------------------------------------------------------------

### Undocumented Instructions

| Instruction | Description |
|:------------|:------------|
| [`GECO`](#geco) | GECO is a customer-specifed instruction which appears to be included as part of the standard instruction set from ND-100/CE and later.  |

--------------------------------------------------------------------------------

### Word Block Instructions

| Instruction | Description |
|:------------|:------------|
| [`MOVEW`](#movew) | Word block move. |

--------------------------------------------------------------------------------

================================================================================
## 📖 Instruction Reference
================================================================================

### AAA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AAA <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Add argument to A

#### 📋 Format

```
AAA <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Add 3 to the contents of the A register.
```
AAA 3
```

--------------------------------------------------------------------------------

### AAB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AAB <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Add argument to B

#### 📋 Format

```
AAB <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Add -26 to the contents of the B register.
```
AAB -26
```

--------------------------------------------------------------------------------

### AAT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AAT <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Add argument to T

#### 📋 Format

```
AAT <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Add 13 to the contents of the T register.
```
AAT 13
```

--------------------------------------------------------------------------------

### AAX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AAX <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Add argument to X

#### 📋 Format

```
AAX <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Add 5 to the contents of the X register.
```
AAX 5
```

--------------------------------------------------------------------------------

### ADD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ADD <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Add to A register

A ⟵ A + (EL)
Add the contents of the memory location pointed to by the effective address to the A register, leaving the result in A.


#### 📋 Format

```
ADD <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

--------------------------------------------------------------------------------

### ADDD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ADDD` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Add two decimal operands.

(op1) ⟵ (op1) + (op2)

Add the second operand to the first operand, leaving the result in the first operand's location.

If the first operand field is too short to contain all the significant digits of the result, then decimal overflow occurs.

If bit 13 of D2 in the first operand is set, the sign of the result will be 17₈ (BCD unsigned).

Any empty operand, that is with a field length of zero, is treated as a positive zero.

**Instruction sequence:**

    ADDD
    error handling instruction
    next instruction after ADDD or after error handling routine

Note:
Operands should be normalized before this instruction is executed using the SHDE instruction.

**Example:**

    ADDD
    JMP *12₈
    STX 20₈

The ADDD instruction causes the program counter to skip the next instruction UNLESS an error has been generated. In this case, the instruction immediately after ADDD will handle the error in some way (in this example a jump is executed on error to ((P) + 12₈)).

(* is the assembler mnemonic for the P register)


#### 📋 Format

```
ADDD
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### AND

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AND <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Logical AND to A register
A ⟵ A & (EA)

Perform a bitwise AND operation between the contents of the A register and the contents of the memory location pointed to by the effective address, leaving the result in A.


#### 📋 Format

```
AND <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### BANC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BANC` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

ogical AND with bit compl

K ⟵ K & (B)0

Perform a bitwise AND with the contents of the B register and the contents of the K register, leaving the result in K.


#### 📋 Format

```
BANC
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BAND

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BAND` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical AND to K

K ⟵ K & (B)


#### 📋 Format

```
BAND
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BFILL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BFILL` |
| Category | Byte Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Byte fill.

Only the destination is used as an operand in this instruction (it is placed in the X and T registers). The lower byte of the A register is then filled with the destination field.

After execution, bit 15 of the T register points to the end of the field (after the last byte position) and the field length equals zero.


#### 📋 Format

```
BFILL
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### BLDA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BLDA` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Load K

K ⟵ bit


#### 📋 Format

```
BLDA
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BLDC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BLDC` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Load K and complement

K ⟵ (B) (complemented)


#### 📋 Format

```
BLDC
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BORA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BORA` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical OR to K

K ⟵ K | (B)


#### 📋 Format

```
BORA
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BORC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BORC` |
| Category | Bit instructions_with_destination |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical OR with bit complement

K ⟵ K | (B)0


#### 📋 Format

```
BORC
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

#### 📚 Examples

##### Complement bit 6 in the X register, then OR the bit with K, leaving the result in K
```
BORC 60 DX
```

--------------------------------------------------------------------------------

### BSET

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BSET <condition> <bit_no>` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Set specified bit in equal to specified condition


#### 📋 Format

```
BSET <condition> <bit_no>
```

#### Bit Layout

```
   │ 15  14  13  12  11│ 10  9   8   7 │ 7   6   5   4   3    │
   ├───────────────────┼───────────────┼──────────────────────┤
   │       opcode      │   condition   │       bit_no         │
   └───────────────────┴───────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `condition` | enum | 10-7 | Specify condition for BSKP and BSET<br><br>**Values:**<br>- `ZRO`: Specificed bit equals zero<br>- `ONE`: Specified bit equals one<br>- `BAC`: Specified bit equals K<br>- `BCM`: Complement specified bit<br> |
| `bit_no` | numeric | 7-3 | Specify bit number<br><br>**Values:**<br>- `SSTG`: Floating rounding flag<br>- `SSK`: 1-bit accumulator (K)<br>- `SSZ`: Error flag (Z)<br>- `SSQ`: Dynamic overflow flag (Q)<br>- `SSO`: Static overflow flag (O)<br>- `SSC`: Page table flag<br>- `SSM`: Page table flag<br> |

#### 📚 Examples

##### Reset the static overflow flag
```
BSET ZRO SSO
```

--------------------------------------------------------------------------------

### BSKP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BSKP <condition> <bit_no>` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Skip next location if specified condition is true

P ⟵ P+1


#### 📋 Format

```
BSKP <condition> <bit_no>
```

#### Bit Layout

```
   │ 15  14  13  12  11│ 10  9   8   7 │ 7   6   5   4   3    │
   ├───────────────────┼───────────────┼──────────────────────┤
   │       opcode      │   condition   │       bit_no         │
   └───────────────────┴───────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `condition` | enum | 10-7 | Specify condition for BSKP and BSET<br><br>**Values:**<br>- `ZRO`: Specificed bit equals zero<br>- `ONE`: Specified bit equals one<br>- `BAC`: Specified bit equals K<br>- `BCM`: Complement specified bit<br> |
| `bit_no` | numeric | 7-3 | Specify bit number<br><br>**Values:**<br>- `SSTG`: Floating rounding flag<br>- `SSK`: 1-bit accumulator (K)<br>- `SSZ`: Error flag (Z)<br>- `SSQ`: Dynamic overflow flag (Q)<br>- `SSO`: Static overflow flag (O)<br>- `SSC`: Page table flag<br>- `SSM`: Page table flag<br> |

#### 📚 Examples

##### Skip the next location if the cary flag is set
```
BSKP ONE SSC
```

--------------------------------------------------------------------------------

### BSTA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BSTA` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Store and clear K

(B) ⟵  K; K ⟵ 0


#### 📋 Format

```
BSTA
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### BSTC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BSTC` |
| Category | Bit instructions |
| Privilege | User |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Store complement and set K to 1

(B) ⟵ K (complemented); K ⟵ 1


#### 📋 Format

```
BSTC
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `K` | Accumulator |

--------------------------------------------------------------------------------

### CHREENTPAGES

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CHREENTPAGES` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Change page tables.

The X register is used to address the current (R1) and previous (Rp) scratch registers.

If the R1 is zero, the reentrant page has nothing to change so the loop is left, otherwise the contents of the memory location pointed to by the R1 + 2 are loaded into T.

T then contains the protect table entry, if the page has not been written to (WIP bit 12 is zero) T and R1 are loaded with Rp. R1 (now containing Rp) is tested again for zero. If the page has been written to, the T register is loaded with the contents of the second scratch register (R2), pointed to by R1, and R2 becomes the address of Rp. X is loaded with R1 as the new pointer to the reentrant pages and Rp is loaded into the D register pointed to by A.


#### 📋 Format

```
CHREENTPAGES
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### CLEPT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CLEPT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Clear page tables.


#### 📚 Detailed Information

This instruction can replace the following instructions:
  CLEPT: JXZ * 10₈
         LDBTX 10₈
         LDA ,B
         JAZ *3₈
         STATX 20₈
         STZ ,B
         LDXTX 00
         JMP *-7₈

Each time the loop is executed (until X becomes zero) the physical memory location addressed by X is loaded into the B register.

The B register contents provide the address of a page table entry, which is loaded into the A register.

If the page table entry is zero (unused) the loop is restarted.

If the page table entry is not zero (used) it is stored in a physical location addressed by X (8 locations away from its original entry) and the original page table entry cleared by placing zero in the location addressed by the B register.

The physical location addressed by X is then loaded into the X register itself and the loop restarted.

* is the mnemonic for P relative addressing.


#### 📋 Format

```
CLEPT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### CLEPU

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CLEPU` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Clear page tables and collect PGU information.


#### 📚 Detailed Information

This instruction collects information on the PGU (page used) bit of a page table entry whilst executing CLEPT.

The instruction places PGU information in an eight word table called the page map bank. Each bit in the bank represents the status of a page's PGU bit as follows:

  15                                              0
+-------------------------------------------------+
|     |     |     |     |     |     |     |   β   | <- word 0
+-------------------------------------------------+
|     |  ε  |     |     |     |     |     |       | <- word 1
+-------------------------------------------------+
.                                                 .
.                                                 .
+-------------------------------------------------+
|  Δ  |     |     |     |     |     |     |       | <- word 7
+-------------------------------------------------+

Δ denotes page 177₈ PGU bit
ε denotes page 32₈ PGU bit
β denotes page 0₈ PGU bit

The L register contains the address of the map entry.


#### 📋 Format

```
CLEPU
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### CLNREENT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CLNREENT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Clear non reentrant pages.

The contents of the memory address at A + 2 are read to find the page table to be cleared along with the SINTRAN RT bitmap (addressed by the X and T registers). 
The page table entries corresponding to those bits set in the RT bitmap are then cleared.


#### 📋 Format

```
CLNREENT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### CLPT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CLPT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Clear segment from the page tables.


#### 📋 Format

```
CLPT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### CNREK

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `CNREK` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Clear non reentrant pages (SINTRAN K only).    


#### 📋 Format

```
CNREK
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### COMD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `COMD` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Compare two decimal operands.

(A) ⟵ (op1) compared to (op2)

(A) =  0   if (op1) = (op2)
(A) =  1   if (op1) > (op2)
(A) = -1   if (op1) < (op2)

Compare the first operand with the second operand, leaving the result in the A register.

If the two operands are unequal in field length, the shorter operand is extended with zeros to allow comparison. The operands are unaffected by the instruction.

The positions of the decimal points are not taken into account when the two operands are compared, so the two operands should be normalized using the SHDE instruction first.

Any empty operand, that is with a field length of zero, is treated as a positive zero. An unsigned number is treated as positive. Positive and negative zeros are equal.

**Instruction sequence:**

    COMD
    error handling instruction
    next instruction after COMD or after error handling routine

**Example:**

    COMD
    JMP *30₈
    AAA 20₈

The COMD instruction causes the program counter to skip the next instruction UNLESS an error has been generated. In this case, the instruction immediately after COMD will handle the error in some way (in this example a jump is executed on error to ((P) + 30₈)).

(* is the assembler mnemonic for the P register)


#### 📋 Format

```
COMD
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### COPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `COPY [sub-instruction(s)] <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Copy source to destination

COPY is a compound mnemonic for RADD CLD.

Optional subinstructions:
The following sub-instructions are allowed:
  CM1, CM2, ADC, AD1

Note:
Using ADC and AD1i in the same instruction generates a no-operation.


#### 📋 Format

```
COPY [sub-instruction(s)] <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6 │ 5   4   3 │  2    1    0    │
   ├───────────────────────────────────────┼───────────┼─────────────────┤
   │                 opcode                │   source  │  destination    │
   └───────────────────────────────────────┴───────────┴─────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-6 | The opcode for register arithmetic |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Copy the contents of the A register to the D register.
```
COPY SA DD
```

##### 2's complement the A register. (Equivalent to RADD CM1 ADC SA DA
```
COPY CM2 SA DA
```

--------------------------------------------------------------------------------

### DEPO

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `DEPO` |
| Category | Physical Memory Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Deposit

Store the contents of the T register in the physical memory location pointed to by the A and D register contents.


#### 📋 Format

```
DEPO
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### DNZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `DNZ <scaling_factor>` |
| Category | Floating Conversion (Standard Format) |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Denormalise     
Convert the floating number in TAD to a fixed point number in A


#### 📚 Detailed Information

The number in the floating point accumulator (A and D registers) is converted to its Single precision fixed point equivalent in the A register using the scaling factor given.

When converting to an integer, a scaling factor of -16 should always be used and will give a fixed point number with the same value as the integer part of the floating point number. 
Other scaling factors will have the same result but the overflow test will be affected.

The D register will be cleared after this instruction.

If the conversion causes underflow, the A and D registers will be set to zero. If overflow occurs (the resulting integer has an absolute value greater than decimal 32767), the error flag (Z) is set to one.

 48-bit: 
 * 48-bit CPUs allow different scaling factors to be used for DNZ operations. However, the overflow test is only failproof for a scaling factor of 16 (decimal)


#### 📋 Format

```
DNZ <scaling_factor>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │         scaling_factor          │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode for floating conversion |
| `scaling_factor` | numeric | 7-0 | Scaling factor in range -128 to 127 (gives converting range from 10^-39 to 10^39) |

#### 📚 Examples

##### Convert the number in TAD to a fixed point number in A
```
DNZ
```

##### Floating to integer conversation. Opcode 152360
```
DNZ -20
```

--------------------------------------------------------------------------------

### ELEAV

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ELEAV` |
| Category | Stack Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Error leave stack.

If an error occurs, leave the stack.

This instruction saves the previous stack pointer in LINK and restores the B register to its previous value (PREVB) before leaving the stack. The stack is left by loading the P register (program counter) with the return address (LINK). The A register is loaded with an error code which is saved in the ERRCODE stack entry (pointed to by B = 173₈).

  (B = 200₈) ⟵ (B = 200₈) - 1
  (P)       ⟵ (B = 200₈)   {LINK}
  (B)       ⟵ (B = 177₈)   {PREVB}
  (A)       ⟵ ERRCODE₈
  (B = 173₈) ⟵ (A)         {ERRCODE}

Format:
  ELEAV


#### 📋 Format

```
ELEAV
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### ENPT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ENPT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Enter segment in page tables. 


#### 📋 Format

```
ENPT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### ENTR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ENTR` |
| Category | Stack Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Enter stack.

This instruction saves the current stack pointer (B), the return address (LINK), and previous stack pointer (PREVB). It transfers the top of stack address (SMAX) and establishes the new stack demand and pointer.

  (B = 177₈) ⟵ (B)                {save current pointer in PREVB}
  (B = 175₈) ⟵ (B = 175₈)         {SMAX}
  (B = 200₈) ⟵ (L) + 1            {save return address in LINK}
  (B)        ⟵ (B = 176₈) + 200₈   {new pointer}
  (B = 176₈) ⟵ stack demand + (B)

Stack overflow causes an error return, that is the program continues at the address following the stack demand value. In all other cases, the program skips this address to find the return address from the stack.

Format:
  ENTR 
  <stack demand-value in words>
  <error return address>
  <return address>
  


#### 📋 Format

```
ENTR
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `PTM` | Page Table Flag |

--------------------------------------------------------------------------------

### EXAM

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `EXAM` |
| Category | Physical Memory Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Examine

Load the contents of the physical memory location, pointed to by the A and D register contents, into the T register.


#### 📋 Format

```
EXAM
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### EXIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `EXIT` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Return from subroutine

This compound mnemonic represents the specific COPY instruction: 
  COPY SL DP

It causes the return from a subroutine by copying the stored return address into the program counter (P register).


#### 📋 Format

```
EXIT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

--------------------------------------------------------------------------------

### EXR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `EXR <sr>` |
| Category | Execute |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Execute instruction found in specified register

The contents of the sr register are executed as the next instruction.
If sr contains a memory reference instruction, the address is given as part of the instruction.    

EXR <sr> cannot be used to fetch an <sr> register containing another EXR instruction. 
If you attempt this the error flag (Z) is set.


#### 📋 Format

```
EXR <sr>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3 │ 2   1   0   │
   ├───────────────────────────────────────────────────┼─────────────┤
   │                       opcode                      │  register   │
   └───────────────────────────────────────────────────┴─────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-3 | The opcode for execute instruction |
| `register` | enum | 2-0 | Register containing instruction to execute<br><br>**Values:**<br>- `SD`: D register<br>- `SB`: B register<br>- `SL`: L register<br>- `SA`: A register<br>- `ST`: T register<br>- `SX`: X register<br> |

#### 📚 Examples

##### [(A) = 014177, STX *177] Execute the instruction held in the A register. Store the K register contents in the memory location pointed to by the program counter plus 177.
```
EXR SA
```

##### [(B) = 134020, JPL 20] Execute the instruction held in the B register. The instruction is 4a jump to a subroutine at memory location ({P} + 20.). The return address (the address of the instruction after EXR) returned to once the subroutine has been completed is heid in the L register.
```
EXR SB
```

--------------------------------------------------------------------------------

### FAD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `FAD <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Add to floating point accumulator

(A) ⟵ (ea)     + (T)
(D) ⟵ (ea + 1) + (A)

The contents of two sequential memory locations, pointed to by the effective address, are added to the contents of the floating point accumulator (T and A registers). The result is held in the accumulator.


#### 📋 Format

```
FAD <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### FDV

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `FDV <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Divide floating point accumulator

The contents of the floating point accumulator (A and D registers) are divided by the contents of two sequential memory locations, pointed to by the effective address.   


#### 📚 Detailed Information

Flags affected:
- Division by zero sets the error flag (2).
- This can be detected by the BSKP instruction


#### 📋 Format

```
FDV <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `Z` | Error flag |

--------------------------------------------------------------------------------

### FMU

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `FMU <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Multiply floating point accumulator.

The contents of the floating point accumulator (A and D registers) are multiplied by the contents of two sequential memory locations, pointed to by the effective address. 
The result is held in the accumulator.


#### 📋 Format

```
FMU <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `Z` | Error flag |

--------------------------------------------------------------------------------

### FSB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `FSB <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Subtract from floating point accumulator.

The contents of two sequential memory locations, pointed to by the effective address, are subtracted from the contents of the floating point accumulator (A and D registers). 
The result is held in the accumulator.


#### 📋 Format

```
FSB <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `Z` | Error flag |

--------------------------------------------------------------------------------

### GECO

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `GECO` |
| Category | Undocumented Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

GECO is a customer-specifed instruction which appears to be included as part of the standard instruction set from ND-100/CE and later. 
The name comes from the customer, GECO (Geophysical Company of Norway)

SINTRAN III version L, and probably version K and possibly earlier, tests for GECO as part of the startup.     

The instruction is found in the ND-110 microcode.


#### 📋 Format

```
GECO
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### IDENT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` IDENT <level_code>` |
| Category | Interrupt Control Instructions |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Transfer IDENT code of interrupting device with highest priority on the specified level to A register. 


#### 📋 Format

```
 IDENT <level_code>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6 │ 5   4   3   2   1   0   │
   ├───────────────────────────────────────┼─────────────────────────┤
   │                 opcode                │       level_code        │
   └───────────────────────────────────────┴─────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-6 | The opcode determines what type of operation occurs |
| `level_code` | enum | 5-0 | The interrupt level code<br><br>**Values:**<br>- `PL10`: Level 10<br>- `PL11`: Level 11<br>- `PL12`: Level 12<br>- `PL13`: Level 13<br> |

--------------------------------------------------------------------------------

### INIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `INIT` |
| Category | Stack Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Initialize stack. 

Loads the addresses pointed to by B with the stack frame addresses. 
Sets up 
  LINK  ⟵ L + 1                          {stack start}  
  PREVB ⟵ (B)                            {save current pointer}  
  SMAX  ⟵ stack start address + maximum stack size  
  (B)   ⟵ (B = 200₈) + 200₈              {establish new pointer}  
  STP   ⟵ stack demand + (B)  

Load the addresses pointed to by **B** with the stack frame addresses.

Stack overflow and flag error causes an error return, that is the program continues at the address following the stack demand value. 
In all other cases, the program skips this address to find the return address from the stack.  

Format:
  INIT 
  <number of words allocated to stack>
  <address of stack start>
  <maximum stack size>
  <flag>
  <address left empty>
  <error return address>
  <return address>


#### 📋 Format

```
INIT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `PTM` | Page Table Flag |

--------------------------------------------------------------------------------

### INSPL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `INSPL` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Insert page in page list. (See Appendix B for a software description.)


#### 📋 Format

```
INSPL
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### IOF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOF` |
| Category | Interrupt Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Interrupt System OFF

Disables the interrupt system.
On IOF the ND-110 continues operation at the same program level.


#### 📋 Format

```
IOF
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### ION

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ION` |
| Category | Interrupt Control Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Interrupt System ON

Enables the interrupt system. On ION the ND-i10 resumes operation in the program level with highest priority.


#### 📋 Format

```
ION
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### IOT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOT <device_register_address>` |
| Category | Input and Output |
| Privilege | Privileged |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

NORD-1 INSTRUCTION (DO NOT USE)


#### 📚 Detailed Information

ND-110 microcode implements this instruction like this:

1861  000627  % IOT - INSTRUCTION IS PRIVILEGED WHEN RING = 0 OR 1
1862  000627  %                  AND ILLEGAL    WHEN RING = 2 OR 3
1863  000627  
1864  000627  IOT1:     ALUD,NONE           IDBS,ALU            T,JMP               T,PUSH              PVCHK;
1865  000630            ALUD,NONE           IDBS,ALU            T,JMP               T,HOLD              ILLIN;


#### 📋 Format

```
IOT <device_register_address>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### IOX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOX <device_register_address>` |
| Category | Input and Output |
| Privilege | Privileged |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Exchange information between I/O system and A register.


#### 📚 Detailed Information


This instruction is PRIVILEGED and only available to:
   - programs running in system mode (rings 2-3)
   - programs running without memory protection
 
 The instruction controls all transfers between the ND-110 and any external devices.
 
 IOX can be used to address a maximum of 2048 device registers for external devices connected to the ND-110 CPU. Data, control, and status between device and CPU can be exchanged.
 
 The transfer direction is determined by the least significant bit of the device register address:
   - If 0: input (from device to CPU)
   - If 1: output (from CPU to device)
 
 Register addresses can hold data, command, or status information for a device.
 An external device may require more than one register address (e.g., a magnetic tape unit may need several). 
 
 These should be given successive device-register addresses (odd for input, even for output).
 
 Note: The number of external devices that can be controlled by the CPU depends on the configuration of the devices.        
     


#### 📋 Format

```
IOX <device_register_address>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10│ 10  9   8   7   6   5   4   3   2   1   0   │
   ├───────────────────────┼─────────────────────────────────────────────┤
   │         opcode        │          device_register_address            │
   └───────────────────────┴─────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-10 | The opcode determines what type of operation occurs (fixed code 11101₂) |
| `device_register_address` | displacement | 10-0 | 11-bit field limiting the number of external devices that can be addressed by the CPU.<br>Bit 0 gives the direction of transfer:<br>  - 0: input (from device to CPU)<br>  - 1: output (from CPU to device)<br> |

#### 📚 Examples

##### Read status of console device into A register
```
IOX 0306
```

##### Write character in A register to console device
```
IOX 0305
```

--------------------------------------------------------------------------------

### IOXT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOXT` |
| Category | Input and Output |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Exchange information between I/O system and A register.


#### 📚 Detailed Information

This instruction is PRIVILEGED and only available to:
  - programs running in system mode (rings 2-3)
  - programs running without memory protection

The instruction controls all transfers between the ND-110 and external devices.

IOXT can be used to address a maximum of 65536 device-register addresses for external devices connected to the ND-110 CPU. 
The 16-bit T register holds the device register address, allowing a much larger address space than IOX. Data, control, and status between device and CPU can be exchanged.

The transfer direction is determined by the least significant bit of the device register address:
  - If 0: input (from device to CPU)
  - If 1: output (from CPU to device)

Instruction format:
  LDT <address mode> <disp>
  IOXT
    
LDT instruction:
  The IOXT instruction uses the T register contents as the device register address. The 16-bit T register gives a limit of 65536 register addresses.
  This address MUST be loaded into the T register before IOXT is executed, hence LDT is used. (See Memory Transfer Instructions, LOAD for explanation of LDT.)
  
  

IOXT instruction:
  IOXT is used as a single mnemonic.

Note: The number of external devices that can be controlled by the CPU depends on the configuration of the devices.


#### 📋 Format

```
IOXT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### IRR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IRR <level> <register>` |
| Category | Inter-level Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Inter Register Read
A: = specified register on specified level


#### 📋 Format

```
IRR <level> <register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7 │ 6   5   4   3 │ 2   1   0    │
   ├───────────────────────────────────┼───────────────┼──────────────┤
   │               opcode              │     level     │  register    │
   └───────────────────────────────────┴───────────────┴──────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-7 | The opcode for inter-register read |
| `level` | uint4 | 6-3 | Privilege level to access (0-15) |
| `register` | enum | 2-0 | Register to read from specified level<br><br>**Values:**<br>- `STS`: Status register<br>- `DD`: D register<br>- `DP`: P register<br>- `DB`: B register<br>- `DL`: L register<br>- `DA`: A register<br>- `DT`: T register<br>- `DX`: X register<br> |

#### 📚 Examples

##### Copy the program counter on program level 14 into the A register of the current program level.
```

```

--------------------------------------------------------------------------------

### IRW

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IRW <level> <register>` |
| Category | Inter-level Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Inter Register Write
Write A to specified register on specified level


#### 📋 Format

```
IRW <level> <register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7 │ 6   5   4   3 │ 2   1   0    │
   ├───────────────────────────────────┼───────────────┼──────────────┤
   │               opcode              │     level     │  register    │
   └───────────────────────────────────┴───────────────┴──────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-7 | The opcode for inter-register read |
| `level` | uint4 | 6-3 | Privilege level to access (0-15) |
| `register` | enum | 2-0 | Register to read from specified level<br><br>**Values:**<br>- `STS`: Status register<br>- `DD`: D register<br>- `DP`: P register<br>- `DB`: B register<br>- `DL`: L register<br>- `DA`: A register<br>- `DT`: T register<br>- `DX`: X register<br> |

#### 📚 Examples

##### Copy the A register on the current program level into the B register on program level 8
```

```

--------------------------------------------------------------------------------

### JAF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JAF <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (A) != 0 (jump if A filled)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JAF <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JAN

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JAN <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (A) < 0 (jump if A is negative)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JAN <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JAP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JAP <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (A) > 0 (jump if A positive)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JAP <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JAZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JAZ <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (A) == 0 (jump if A is zero)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JAZ <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JMP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `JMP <address mode> <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |

#### 📝 Description

Jump - Unconditional jump to specified address
The next instruction is taken from the effective address of the JMP instruction(the effective address is ioaded into the program counter).  


#### 📋 Format

```
JMP <address mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JNC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JNC <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Increment X and jump if X is negative    


#### 📚 Detailed Information

X = X + 1
Condtion: Jump if (X) < 0 (jump if X is negative)

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.     

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JNC <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JPC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JPC <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Increment X and jump if X is positive    


#### 📚 Detailed Information

X = X + 1
Condtion: Jump if (X) > 0 (jump if X is positive)

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.     

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JPC <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JPL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `JPL <address mode> <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |

#### 📝 Description

Jump if Plus - Jump to specified address if the result of the last operation was positive (sign bit is 0)

The contents of the program counter are transferred to the L register and the next instruction is taken from the effective address of the JPL instruction.
Note that the L register points to the instruction after the jump(the program counter incremented before transfer to the L register).	


#### 📋 Format

```
JPL <address mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JXN

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JXN <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (X) < 0 (jump if X negative)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JXN <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### JXZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JXZ <displacement>` |
| Category | Sequencing Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Condtion: Jump if (X) == 0 (jump if X is zero)


#### 📚 Detailed Information

If condition true, jump to the address of the program counter plus the value of displacement.
If condition false, continue program execution at (P) + 1.    

'displacement' = The eight displacement (disp) bits give a signed range of -128 to 127 locations to be jumped if the condition is true.


#### 📋 Format

```
 JXZ <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          displacement           │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LACB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LACB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load the A register from the core map-table bank (CMBNK).

(A) <--- (ea)

ea = (B) + Δ = CMBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)


#### 📋 Format

```
LACB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LASB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LASB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load the A register with the contents of the segment-table bank (STBNK).

(A) <--- (ea)

ea = (B) + Δ = STBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)


#### 📋 Format

```
LASB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LBIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LBIT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Load single bit accumulator (K) with logical memory bit.

(X) points to the start of a bit array
(A) points to the bit within the array


#### 📋 Format

```
LBIT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LBITP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LBITP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Load single bit accumulator (K) with physical memory bit.

(T) points to the bank number containing the bit array
(X) points to the start of a bit array
(A) points to the bit within the array


#### 📋 Format

```
LBITP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LBYT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LBYT` |
| Category | Byte Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Load byte from memory to A register

Addressing: EL = (T) + (X)/2
- If least significant bit of X = 1: Load right byte
- If least significant bit of X = 0: Load left byte

Load the byte addressed by the contents of the T and X register into the lower byte of the A register. The higher byte of the A register is cleared.

The contents of T point to the beginning of a character string and the contents of X to a byte within the string.


#### 📋 Format

```
LBYT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LBYTP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LBYTP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Load the A register with a byte from physical memory.

(D) points to the bank number containing the byte array
(T) points to the start of a byte array
(X) points to the actual byte within the array


#### 📋 Format

```
LBYTP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LDA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDA <addressing_mode> <disp>` |
| Category | Memory Transfer - Load Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load A register

Load the contents of the memory location pointed to by the effective address into the A register.


#### 📋 Format

```
LDA <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LDATX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDATX <disp>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load A register

(A) ⟵ (ea)

Load the contents of the physical memory location pointed to by the effective address into the A register.


#### 📋 Format

```
LDATX <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LDBTX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDBTX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load B register

(B) ⟵ 177000₈ OR (2(ea))

Load the contents of the physical memory location pointed to by twice the effective address contents into the B register, then OR the value with 177000₈.

See description for usage.


#### 📋 Format

```
LDBTX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LDD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDD <addressing_mode> <displacement>` |
| Category | Memory Transfer - Double word instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load double word
  A ⟵ (ea)
  D ⟵ (ea) + 1

Load the contents of the memory location pointed to by the effective address into the A register 
Load the contents of the memory location pointed to by the effective address plus one into the D register.


#### 📋 Format

```
LDD <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LDDTX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDDTX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load double word

(A) ⟵ (ea)
(D) ⟵ (ea + 1)

Load the contents of the physical memory location pointed to by the effective address into the A register and the contents of the effective address plus one into the D register.


#### 📋 Format

```
LDDTX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LDF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDF <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load floating accumulator (TAD) from memory (FW)
Memory format:
 (T) ⟵ EL
 (A) ⟵ EL+1
 (D) ⟵ EL+2

Load the contents of the memory location pointed to by the effective address into the T register, the contents of the effective address plus one into the A register and the contents of the effective address plus two into the D register.


#### 📋 Format

```
LDF <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LDT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDT <addressing_mode> <disp>` |
| Category | Memory Transfer - Load Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load T register

(T) ⟵ (ea)

Load the contents of the memory location pointed to by the effective address into the T register.


#### 📋 Format

```
LDT <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LDX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDX <addressing_mode> <disp>` |
| Category | Memory Transfer - Load Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load X register

(X) ⟵ (ea)

Load the contents of the memory location pointed to by the effective address into the X register.


#### 📋 Format

```
LDX <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### LDXTX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDXTX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load X register

(X) ⟵ (ea)

Load the contents of the physical memory location pointed to by the effective address into the X register.


#### 📋 Format

```
LDXTX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LEAVE

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LEAVE` |
| Category | Stack Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Leave stack.

This instruction saves the previous stack pointer in LINK. 
The B register is restored to its previous value (PREVB) and the stack is left by loading the P register (program counter) with the return address (LINK).

  (P) ⟵ (B = 200₈)   {LINK}
  (B) ⟵ (B = 177₈)   {PREVB}

Format:
  LEAVE


#### 📋 Format

```
LEAVE
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LRB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LRB <level>` |
| Category | Register Block Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Load register block

Load the contents of a memory block pointed to by the X register into the register block of the program level given in the instruction.

If the instruction specifies the current program level, the P register (program counter) is not loaded from memory and is unchanged.


#### 📚 Detailed Information

The memory block pointed to by the X register is loaded into the register block for the specified program level. The mapping is as follows:

    P (level N) ⟵ (ea)
    X (level N) ⟵ (ea + 1)
    T (level N) ⟵ (ea + 2)
    A (level N) ⟵ (ea + 3)
    D (level N) ⟵ (ea + 4)
    L (level N) ⟵ (ea + 5)
    STS (level N) ⟵ (ea + 6)
    B (level N) ⟵ (ea + 7)

Where (ea) is the address in memory pointed to by the X register.

If the current program level is specified, the P register is not loaded and remains unchanged.


#### 📋 Format

```
LRB <level>
```

#### Bit Layout

```
   │ 15  14  13  12  11│ 6   5   4   3 │ 2   1   0    │
   ├───────────────────┼───────────────┼──────────────┤
   │       opcode      │     level     │    type      │
   └───────────────────┴───────────────┴──────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `level` | uint4 | 6-3 | The level to load the register block to |
| `type` | enum | 2-0 | The type of register block function<br><br>**Values:**<br>- `SRB`: Store Register Block<br>- `LRB`: Load Register Block<br> |

#### 📚 Examples

##### LRB 160₈ (152760₈)

Load the memory block pointed to by the X register into the register on program level 14.

    P (level 14) ⟵ (ea)
    X (level 14) ⟵ (ea + 1)
    ↓             ↓
    B (level 14) ⟵ (ea + 7)

```
LRB 160
```

--------------------------------------------------------------------------------

### LWCS

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LWCS` |
| Category | privileged |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Writable Control Store Instruction

This instruction is PRIVILEGED and only available to:
  - programs running in system mode (rings 2-3)
  - programs running without memory protection

LWCS is a no-operation in the ND-110.

The ND-110 is software compatible but not microcode compatible and writing to the writable control store has no meaning in the ND-110. 
A no-operation is executed so that programs written for the ND-100 and NORD-10 can continue.

    
Unused areas of the microprogram can be read or written to using the TRR CS or TRA CS instruction.

Further information on the LWCS instruction for the ND-100 can be found in the ND-100 Reference Manual (ND-06.014).


#### 📋 Format

```
LWCS
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### LXCB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LXCB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load the X register from the core table bank (CMBNK).

(X) <--- (ea)

ea = (B) + Δ = CMBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)


#### 📋 Format

```
LXCB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### LXSB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LXSB <diplacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Load the X register from the segment table bank (STBNK).

(X) <--- (ea)

ea = (B) + Δ = STBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)


#### 📋 Format

```
LXSB <diplacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### MCL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MCL <internal_register>` |
| Category | Register Transfer |
| Privilege | Privileged |
| Mask | `1111_1111_1111_0000` |

#### 📝 Description

Masked clear

The A register is used as a mask to clear bits within the selected internal register.
Setting a bit in the A register clears the corresponding bit in the internal register.

See table for internal registers that allow MCL.


#### 📋 Format

```
MCL <internal_register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  3    2    1    0    │
   ├───────────────────────────────────────────────┼──────────────────────┤
   │                     opcode                    │  internal_register   │
   └───────────────────────────────────────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode determines what type of operation occurs |
| `internal_register` | enum | 3-0 | The CPU internal register<br><br>**Values:**<br>- `STS`: <br>- `PID`: <br>- `PIE`: <br> |

#### 📚 Examples

##### MCL STS (150201₈)   [ (A) = 000100₈ ]

Clear the carry flag (bit 6) in the status (STS) register.

```
MCL STS
```

--------------------------------------------------------------------------------

### MIN

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MIN <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Memory increment and skip next instruction if zero (EA): = (EA) + 1

(ea) ⟵ (ea) + 1
(P)  ⟵ (P) + 2 IF new (ea) = 0

The contents of the memory location pointed to by the effective address are incremented by one. 
If the new memory location when incremented becomes zero, the next instruction is skipped.


#### 📋 Format

```
MIN <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### MIX3

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MIX3` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Multiply index by 3.

(X) ⟵ [(A)-1] * 3

Take the contents of the A register as an operand and subtract one. Multiply the result by three and place it in the X register.


#### 📋 Format

```
MIX3
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### MON

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MON <monitor_call_number>` |
| Category | Monitor Calls |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

The MON instruction is used in special different contexts when running under an operating system.
It provides system call functionality through different monitor call numbers.


#### 📚 Detailed Information

The unsigned <monitor_call_number> allows 256 monitor calls.
This field is loaded into the T register on level 14.
The higher byte of the T register is sign exended.


#### 📋 Format

```
MON <monitor_call_number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │      monitor_call_number        │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `monitor_call_number` | numeric | 7-0 | Monitor call number that determines the system function |

#### 📚 Examples

##### End of program or stop
```
MON 0
```

##### Read a character from specified device into the A register
```
MON 1
```

##### Output the character contained in the A register on the specified device
```
MON 2
```

--------------------------------------------------------------------------------

### MOVB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MOVB` |
| Category | Byte Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Move byte.

This instruction moves a block of bytes from the memory location addressed by the source operand to that of the memory location addressed by the destination operand.

After execution, bit 15 of the D and T registers point to the end of the field that has been moved. The field length of the D register (source) equals zero and the T register (destination) field length is equal to the number of bytes moved.


#### 📋 Format

```
MOVB
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### MOVBF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MOVBF` |
| Category | Byte Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Move bytes forward.

This instruction moves a block of bytes from the memory location addressed by the source operand to that of the memory location addressed by the destination operand.

After execution, bit 15 of the D and T registers point to the end of the field that has been moved. The field length of the D register (source) equals zero and the T register (destination) field length is equal to the number of bytes moved.


#### 📋 Format

```
MOVBF
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### MOVEW

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MOVEW` |
| Category | Word Block Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1000` |

#### 📝 Description

Word block move.

This instruction moves a block of words from one area of memory to another. The type of transfer is determined by the opcode field ΔΔ, with the base octal code 1431ΔΔ.

  ┌───────┬─────────────┬─────────────┐
  │ ΔΔ₈   │ move from:  │ move to:    │
  ├───────┼─────────────┼─────────────┤
  │ 00₈   │ PT          │ PT          │
  │ 01₈   │ PT          │ APT         │
  │ 02₈   │ PT          │ phy.memory* │
  │ 03₈   │ APT         │ PT          │
  │ 04₈   │ APT         │ APT         │
  │ 05₈   │ APT         │ phy.memory* │
  │ 06₈   │ phy.memory  │ PT*         │
  │ 07₈   │ phy.memory  │ APT*        │
  │ 10₈   │ phy.memory  │ phy.memory* │
  └───────┴─────────────┴─────────────┘
  * = privileged instruction
  PT  = normal page table
  APT = alternative page table
  phy.memory = physical memory

The following registers control the transfer:
- A and D: source address
- X and T: destination address
- L: number of words to be moved (maximum 2048 words)

For physical memory-block moves, A and/or X are incremented when D and/or T overflow.

If the L register contains a value greater than 2048 (L = 4000₈), no words are moved and A, D, T, and X remain unchanged.

After transfer, the registers contain:
- A, D, T, X: addresses after the last moved word
- L: zero

Special cases:
If the memory management system is off (bit PTM of the STS register is zero), bank 0 of physical memory is addressed and the following transfer fields become equivalent:
- ΔΔ = 00 = 01 = 03 = 04
- ΔΔ = 02 = 05
- ΔΔ = 06 = 07

MOVEW can be interrupted. L, A, D, X, T, and P registers are then changed to restart execution.


#### 📋 Format

```
MOVEW
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### MPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MPY <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Multiply integer
A ⟵ A * (EL)

Multiply the contents of the memory location pointed to by the effective address with the contents of the A register, leaving the result in A.


#### 📚 Detailed Information

**Flags affected**
  O ⟵ 1; Q ⟵ 1
  If an overflow occurs, that is if the result has an absolute value greater than decimal 32767


#### 📋 Format

```
MPY <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

--------------------------------------------------------------------------------

### MST

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MST <register>` |
| Category | Register Transfer |
| Privilege | Privileged |
| Mask | `1111_1111_1111_0000` |

#### 📝 Description

Masked set

The A register is used as a mask to set bits within the selected internal register.
Setting a bit in the A register sets the corresponding bit in the internal register.

See table for internal registers that allow MST.


#### 📋 Format

```
MST <register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  3    2    1    0    │
   ├───────────────────────────────────────────────┼──────────────────────┤
   │                     opcode                    │  internal_register   │
   └───────────────────────────────────────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode determines what type of operation occurs |
| `internal_register` | enum | 3-0 | The CPU internal register<br><br>**Values:**<br>- `STS`: <br>- `PID`: <br>- `PIE`: <br> |

--------------------------------------------------------------------------------

### NLZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `NLZ <scaling_factor>` |
| Category | Floating Conversion (Standard Format) |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Normalize

Convert the number in A to a floating number in TAD


#### 📚 Detailed Information


The number in the A register is converted to its floating point equivalent in the floating point accumulator (A and D registers), 
using the scaling factor given, For integers, a scaling factor of +16 will give a floating point number with the same value as the integer.
The larger the scaling factor, the larger the Ploating point number. 

The D register will be cleared when using single precision fixed point numbers.


Comments on 48-bit floating point CP
For the ND-110, 48-bit floating point CPU option, a further register (T) and memory location fea + 2) are used. In this case, the T register is linked to location ea, A to ea + 1 and D to ea + 2.

How to test for a 32-bit or 48-bit floating point CPU:
  SAT 0
  SAA 1
  NLZ 20

This tests whether T is changed, if so, the CPU is 48-bit; otherwise it is 32-bit.


#### 📋 Format

```
NLZ <scaling_factor>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │         scaling_factor          │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode for floating conversion |
| `scaling_factor` | numeric | 7-0 | Scaling factor in range -128 to 127 (gives converting range from 10^-39 to 10^39) |

#### 📚 Examples

##### Convert the number in A to a floating number in TAD
```
NLZ
```

##### Integer to floating conversation
```
NLZ +20
```

--------------------------------------------------------------------------------

### OPCOM

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `OPCOM` |
| Category | Interrupt Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Operator Communication

This instruction is PRIVILEGED and only available to:
  - programs running in system mode (rings 2-3)
  - programs running without memory protection

This instruction allows the programmer to use a terminal in direct communication with the CPU board.
When the CPU is running, MOPC can be used to read input from the console.
This is the software equivalent to pressing the OPCOM button on the control panel of the ND-110.


#### 📋 Format

```
OPCOM
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### ORA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ORA <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Logical inclusive OR to A register
A ⟵ A | (EA)

Perform a bitwise OR operation between the contents of the A register and the contents of the memory location pointed to by the effective address, leaving the result in A.


#### 📋 Format

```
ORA <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### PACK

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `PACK` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Convert to BCD.

(op2) ⟵ (op1) in BCD format

Convert the first operand from its ASCII format to BCD format, placing the result in the second operand location.

**Conversion process:**
PACK carries out the following steps:
1. Checks the sign and digits of the operand (op1) are encoded as ASCII digits. (Reporting illegal codes as error code 2.)
2. Takes the 4 least significant bits of each ASCII digit as the equivalent BCD digit.
3. Converts the ASCII sign of the operand to BCD:

       ASCII   BCD   Sign
       53₈    14₈   +
       55₈    15₈   -
    Note: If bit 13 of the descriptor D2 is set, then the code 17₈ (BCD unsigned) is used.
4. Extends the second operand field (op2) with zeros if the result is too small to fill the field.
   Reports overflow has occurred (error code 3) if the second operand field length is too short to contain the significant digits of the result (the remaining digits are ignored).

**Error code:**
An error code is placed in bits 0 to 4 of the D register if an illegal code conversion is attempted or if overflow occurs. The first detected error will be reported.

    Error code: 2   illegal code†
                3   overflow
    † bit 15 of both the A and D registers point to the byte containing the illegal code.


#### 📋 Format

```
PACK
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### PIOF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `PIOF` |
| Category | Memory Management Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Memory management and interrupt system OFF

Disables both the memory management and interrupt systems. This combines the functions of the IOF and POF instructions.

Before_use:
  Check conditions of the IOF instruction.


#### 📋 Format

```
PIOF
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### PION

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `PION` |
| Category | Memory Management Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Memory management and interrupt system ON

Enable both the memory management and interrupt systems. This combines the functions of the ION and PON instructions.


#### 📋 Format

```
PION
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### POF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `POF` |
| Category | Interrupt Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Memory management OFF

Disable memory management system. The next instruction will be taken from a physical address given by the address following the POF instruction.
Note: The CPU will be in an unrestricted mode without any hardware protection features - all instructions are legal and all memory accessible.


#### 📋 Format

```
POF
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### PON

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `PON` |
| Category | Memory Management Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Memory management ON

Enable memory management system. The next instruction after PON will then use the pageindex table specified by PCR.
BEFORE USE ENSURE:
  - Interrupt system is enabled,
  - Internal hardware interrupts are enabled,
  - Page tables and PCR registers are initialized.


#### 📋 Format

```
PON
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### RADD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RADD [sub-instruction(s)] <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1100_0000_0000` |

#### 📝 Description

Add source to destination (dr): = (dr) + (sr)


#### 📚 Detailed Information

These instructions specify operations between source (sr) and destination (dr) registers.

Optional sub-instructions:

- **CLD**: CLD=1. Zero is used instead of the destination register as an operand (dr register contents are unchanged).
- **CM1**: CM1=1. The 1's complement of the source register is used as an operand (sr register contents are unchanged).
- **ADC**: This mnemonic represents C=0 I=1. Add previous carry to destination register.
- **AD1**: This mnemonic represents C=1 I=0. Add 1 to destination register.
- **CM2**: This compound sub-instruction is equivalent to CM1 ADC.

Flags affected:

RADD instructions affect the carry (C) and overflow (O and Q) flags as follows:

- **C = 1**: If a carry occurs from the signed bit positions of the adder.
- **O = 1 ; Q = 1**: If an overflow occurs, that is if the signs of the two operands are equal and the sign of the result is different.
- **O = 1 ; Q = 0**: If overflow does not exist, the dynamic overflow flag (O) is reset while the static overflow flag (Q) is left unchanged.

---

### Instruction combinations

COPY
RADD <sr> <dr>           dr ⟵ dr + sr
RADD CLD <sr> <dr>       dr ⟵ sr         ≡ COPY
RADD CM1 <sr> <dr>       dr ⟵ dr + ~sr
RADD CM1 CLD <sr> <dr>   dr ⟵ ~sr
RADD AD1 <sr> <dr>       dr ⟵ dr + sr + 1
RADD CLD AD1 <sr> <dr>   dr ⟵ sr + 1
RADD CM1 AD1 <sr> <dr>   dr ⟵ dr - sr    ≡ RSUB
RADD CM1 CLD AD1 <sr> <dr> dr ⟵ -sr
RADD ADC <sr> <dr>       dr ⟵ dr + sr + c
RADD CLD ADC <sr> <dr>   dr ⟵ sr + c
RADD CM1 ADC <sr> <dr>   dr ⟵ dr + ~sr + c
RADD CM1 CLD ADC <sr> <dr> dr ⟵ ~sr + c

RINC
RADD AD1 <dr> is equivalent to RINC <dr>, increment dr register by one.

RDCR
RADD CM1 <dr> is equivalent to RDCR <dr>, decrement dr register by one.

RCLR
RADD CLD 0 (COPY 0) is equivalent to RCLR <dr>, register clear.

EXIT
RADD CM1 SL DP (COPY SL DP) is equivalent to EXIT, return from subroutine.


#### 📋 Format

```
RADD [sub-instruction(s)] <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Arithmetic operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Add the contents of the B register to the contents of the A register, leaving the result in the A register.
```
RADD SB DA
```

##### Add the contents of the A and X registers together and place the result in the X register. (146057₈)
```
RADD SA DX
```

##### Use zero as the destination operand, add the X register contents and leave the result in B. THAT IS copy the contents of the X register into B. (146173₈)
Equivalent to COPY SX DB.

```
RADD CLD SX DB
```

##### Copy the negative value of the X register contents into B. (146773₈)
Equivalent to COPY CLD AD1 or RSUB CM1.

```
RADD CM1 CLD AD1 SX DB
```

--------------------------------------------------------------------------------

### RAND

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RAND [sub-instruction(s)] <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Logical AND to destination 
(dr): = (dr) & (sr)


#### 📚 Detailed Information

Optional sub-instructions:

- **CLD=1**: Zero is used instead of the destination register as an operand (dr register contents are unchanged).
- **CM1=1**: The 1's complement of the source register is used as an operand (sr register contents are unchanged).

---

### Instruction combinations

RAND <sr> <dr>           dr ⟵ dr AND sr
RAND CLD <sr> <dr>       dr ⟵ 0
RAND CM1 <sr> <dr>       dr ⟵ dr AND ~sr
RAND CM1 CLD <sr> <dr>   dr ⟵ 0


#### 📋 Format

```
RAND [sub-instruction(s)] <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Logical operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 📚 Examples

##### AND the contents of the L and X registers. Store the result in the X register. (144447₈)
```
RAND SL DX
```

##### AND the contents of the T and B registers, taking the 1's complement of the sr as the source operand. (144663₈)
Result in B.

```
RAND CM1 ST DB
```

--------------------------------------------------------------------------------

### RCLR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RCLR <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1000` |

#### 📝 Description

Register clear


#### 📚 Detailed Information

RCLR is a compound mnemonic for "COPY 0" (or RADD CLD 0).


#### 📋 Format

```
RCLR <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  2    1    0   │
   ├───────────────────────────────────────────────┼────────────────┤
   │                     opcode                    │  destination   │
   └───────────────────────────────────────────────┴────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode for register arithmetic |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Clear the contents of the A register.
```
RCLR DA
```

--------------------------------------------------------------------------------

### RDCR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RDCR <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1000` |

#### 📝 Description

Register decrement

dr ⟵ dr - 1

RDCR is a compound mnemonic for RADD CM1.


#### 📋 Format

```
RDCR <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  2    1    0   │
   ├───────────────────────────────────────────────┼────────────────┤
   │                     opcode                    │  destination   │
   └───────────────────────────────────────────────┴────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode for register arithmetic |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Decrement the contents of the B register by one. (146203₈)
```
RDCR DB
```

--------------------------------------------------------------------------------

### RDIV

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RDIV <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Divide double accumulator with source; quotient in A, remainder in D
A: = AD/(sr)
(AD = A * (sr) + D)


#### 📚 Detailed Information

The 32-bit signed integer held in the double accumulator AD is divided by the contents of sr.
If division causes overflow, the error flag (Z) is set.

The numbers are fixed point integers with the fixed point after the rightmost position.


#### 📋 Format

```
RDIV <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6 │ 5   4   3 │  2    1    0    │
   ├───────────────────────────────────────┼───────────┼─────────────────┤
   │                 opcode                │   source  │  destination    │
   └───────────────────────────────────────┴───────────┴─────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-6 | The opcode for register arithmetic |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `Z` | Error flag |
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

--------------------------------------------------------------------------------

### RDUS

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RDUS` |
| Category | Memory Examine and Test Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Read a word without using cache

(T) points to the virtual memory word to be accessed
(A) ⟵ memory word addressed by T

The address given by the T register is a logical memory address. It is normally translated into a physical address using page tables (if the memory management system is on). The contents of the location addressed by T are loaded into the A register. The old content of the memory address is always read from the memory and never from cache.

The execution time of this instruction includes two read bus cycles (semaphore cycles – see ND-110 Functional Description Manual ND.06.027).


#### 📋 Format

```
RDUS
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### RDUSP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RDUSP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Read a physical memory word without using cache.

(T) points to the physical memory bank to be accessed
(X) points to the address within the bank
(A) is loaded with the memory word

The old content of the memory address is always read from the memory and never from cache.

Note: The execution time of this instruction includes two read-bus cycles (The CPU uses semaphore cycles - see ND-110 Functional Description Manual ND.06.027)


#### 📋 Format

```
RDUSP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### REMPL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `REMPL` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Remove page from page list.


#### 📋 Format

```
REMPL
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### REPT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `REPT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Enter reentrant segment in page tables. (See Appendix B for a software description.)


#### 📋 Format

```
REPT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### REX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `REX` |
| Category | Memory Management Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Reset extended address mode


#### 📋 Format

```
REX
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### REXO

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `REXO [flags] <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Logical exclusive OR 

(dr): = (dr) ⊕ (sr)


#### 📚 Detailed Information

Optional sub instructions:
  CLD=1 Zero is used instead of the destination register as an operand (dr register contents are unchanged).
  CM1=1 The 1's complement of the source register is used as an operand (sr register contents are unchanged).

Instruction combinations:

  REXO <sr> <dr>          dr ⟵ dr XOR sr
  REXO CLD <sr> <dr>      dr ⟵ sr
  REXO CML <sp> <dr>      dr ⟵ dr XOR sr's (1's complement)
  REXO CML CLD <sr> <dr>  dr ⟵ sr (1's complement)


#### 📋 Format

```
REXO [flags] <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Logical operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 📚 Examples

##### Exclusive OR the contents of the B and Y registers, leaving the result in B
```
REXO ST DB
```

--------------------------------------------------------------------------------

### RGLOB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RGLOB` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Examine global pointers.

(T) <--- bank number of segment table (STBNK)
(A) <--- start address within bank (STSRT)
(D) <--- bank number of core map table (CMBNK)


#### 📋 Format

```
RGLOB
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### RINC

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RINC <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1111_1000` |

#### 📝 Description

Register increment

dr ⟵ dr + 1

RINC is a compound mnemonic for RADD AD1.


#### 📋 Format

```
RINC <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  2    1    0   │
   ├───────────────────────────────────────────────┼────────────────┤
   │                     opcode                    │  destination   │
   └───────────────────────────────────────────────┴────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode for register arithmetic |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Increment the contents of the A register by one.
```
RINC DA
```

--------------------------------------------------------------------------------

### RMPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RMPY <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Multiply source with destination; result in double accumulator
AD ⟵ (sr) * (dr)


#### 📚 Detailed Information

The sr and dr registers hold the two operands to be multiplied together. 
The result is a 32-bit signed integer held in the A and D register (the A register contains the 16 most significant bits).


#### 📋 Format

```
RMPY <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6 │ 5   4   3 │  2    1    0    │
   ├───────────────────────────────────────┼───────────┼─────────────────┤
   │                 opcode                │   source  │  destination    │
   └───────────────────────────────────────┴───────────┴─────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-6 | The opcode for register arithmetic |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Multiply the contents of the A and X registers together, leaving the result in the A and D registers.
```
RMPY SA DX
```

--------------------------------------------------------------------------------

### RORA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RORA <sub-instruction(s)> <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

OR register

dr ⟵ dr OR sr


#### 📚 Detailed Information

Optional sub-instructions:

- **CLD=1**: Zero is used instead of the destination register as an operand (dr register contents are unchanged).
- **CM1=1**: The 1's complement of the source register is used as an operand (sr register contents are unchanged).

---

### Instruction combinations

RORA <sr> <dr>           dr ⟵ dr OR sr
RORA CLD <sr> <dr>       dr ⟵ sr
RORA CM1 <sr> <dr>       dr ⟵ dr OR ~sr
RORA CM1 CLD <sr> <dr>   dr ⟵ ~sr


#### 📋 Format

```
RORA <sub-instruction(s)> <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Logical operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### OR the contents of the B and T registers leaving the result in B. (145463₈)
```
RORA ST DB
```

--------------------------------------------------------------------------------

### RSUB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RSUB <sub-instruction> <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Register subtract

dr ⟵ dr - sr

RSUB is a compound mnemonic for RADD AD1 CM1 (or RADD CM2 using the AD1 CM2 compound mnemonic).


#### 📚 Detailed Information

The following sub-instruction is allowed:

- **CLD**

Note:
Sometimes the RADD form will be more readable.


#### 📋 Format

```
RSUB <sub-instruction> <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Arithmetic operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Subtract the contents of the T register from the contents of the B register leaving the result in B.
```
RSUB ST DB
```

--------------------------------------------------------------------------------

### SAA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAA <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Set argument to A

#### 📋 Format

```
SAA <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Set the A register to 10.
```
SAA 10
```

--------------------------------------------------------------------------------

### SAB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAB <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Set argument to B

#### 📋 Format

```
SAB <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### The contents of the B register are set to 177752, bits 0-7 have been extended with ones as the argument is negative; bits 0-7 have the argument in its 2's complement form.
```
SAB -26
```

--------------------------------------------------------------------------------

### SACB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SACB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store the A register in the core map table bank (CMBNK).

(ea) <--- (A)

ea = (B) + Δ = CMBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)  


#### 📋 Format

```
SACB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### SAD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAD <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | User |
| Mask | `1111_1001_1000_0000` |

#### 📝 Description

Shift A and D registers connected
Every shift instruction places the last bit discarded in the multi-shift flag (M). M can be used as an inputfor the next shift instruction. M is bit 8 of the STS Register.

#### 📋 Format

```
SAD <shift_type> <shift_counter>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10      9  │           │    6   │ 5   4   3   2   1   0 │   0        │
   ├───────────────────┼─────────────┼───────────┼────────┼───────────────────────┼────────────┤
   │       opcode      │  shift_type │  register │  mask6 │     shift_counter     │  SHR       │
   └───────────────────┴─────────────┴───────────┴────────┴───────────────────────┴────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `shift_type` | enum | 10-9 | <br><br>**Values:**<br>- `Arithmetic`: Arithmetic shift. During right shift, bit 15 is extended. During left shift, zeros are shifted in from right.<br>- `ROT`: Rotational shift. Most and least significant bits are connected.<br>- `ZIN`: Zero end input<br>- `LIN`: Link end input. The last vacated bit is fed to M after every shift instruction.<br> |
| `register` | enum | 7-8 | The register to shift |
| `mask6` | numeric | 6 | Always 0 |
| `SHR` | assembler_flag | 0 | This a feature of the assembler. This mnemonic can be used to specify shift right, so that instead of calculating the 2's complement for the number of right shifts required, SHR can be used. |
| `shift_counter` | int6 | 5-0 | The number of bits to shift the register.<br>Bits 0-4 are the number of shifts.<br>Bit 5=1, then shift right (max 32 times)<br>Bit 5=0, then shift left (max 31 times)<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `M` | Multi-shift link flag |

--------------------------------------------------------------------------------

### SASB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SASB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store the A register contents in the segment table bank (STBNK).

(ea) <--- (A)

ea = (B) + Δ = STBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)  


#### 📋 Format

```
SASB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### SAT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAT <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Set argument to T

#### 📋 Format

```
SAT <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Set the T register to 20₈.
```
SAT 20₈
```

--------------------------------------------------------------------------------

### SAX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAX <number>` |
| Category | Argument Instruction |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Set argument to X

#### 📋 Format

```
SAX <number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │             number              │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `number` | numeric | 7-0 | 8-bit argument extended to 16 bits using sign extension |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Set the X register to -5.
```
SAX -5
```

--------------------------------------------------------------------------------

### SBIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SBIT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Store the single bit accumulator (K) in a logical memory bit.

(X) points to the start of a bit array
(A) points to the bit within the array


#### 📋 Format

```
SBIT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SBITP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SBITP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Store the single bit accumulator (K) in a physical memory bit.

(T) points to the bank number containing the bit array
(X) points to the start of a bit array
(A) points to the bit within the array


#### 📋 Format

```
SBITP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SBYT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SBYT` |
| Category | Byte Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Store byte from A register to memory
Addressing: EL = (T) + (X)/2
- If least significant bit of X = 1: Store right byte
- If least significant bit of X = 0: Store left byte

The contents of T point to the beginning of a character string and the contents of X to a byte within the string.


#### 📋 Format

```
SBYT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SBYTP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SBYTP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Store a byte in physical memory.

(D) points to the bank number containing the byte array
(T) points to the start of a byte array
(X) points to the actual byte within the array


#### 📋 Format

```
SBYTP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SETPT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SETPT` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Set page tables.


#### 📚 Detailed Information

This instruction can replace the following instructions:
  SETPT: JXZ * 7₈
         LDDTX 20₈
         BSET ZRO 130₈ DA
         LDBTX 10₈
         STD ,B
         LDXTX 00
         JMP *-6₈

Each time the loop is executed (until X becomes zero) two consecutive physical memory locations addressed by X are loaded into the A and D registers.

The word in A is the protect field of the page table, bit 11 (the PGU bit) is cleared to set the page table. The double word (in A and D) is then stored in two consecutive locations pointed to by the contents of the B register, the page table address.

* is the mnemonic for P relative addressing.


#### 📋 Format

```
SETPT
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SEX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SEX` |
| Category | Memory Management Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Set extended address mode


#### 📚 Detailed Information

Set the paging system to the 24-bit address mode instead of the 19-bit address mode.
(Creating a page table for 24-bit addresses)

Flags affected: SEXI (bit 13 of the STS register) cleared


#### 📋 Format

```
SEX
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `SEXI` | Memory management is in extended mode (24-bit address used, not 19 bit) |

--------------------------------------------------------------------------------

### SHA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHA <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | User |
| Mask | `1111_1001_1000_0000` |

#### 📝 Description

Shift A register
Every shift instruction places the last bit discarded in the multi-shift flag (M). M can be used as an inputfor the next shift instruction. M is bit 8 of the STS Register.

#### 📋 Format

```
SHA <shift_type> <shift_counter>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10      9  │           │    6   │ 5   4   3   2   1   0 │   0        │
   ├───────────────────┼─────────────┼───────────┼────────┼───────────────────────┼────────────┤
   │       opcode      │  shift_type │  register │  mask6 │     shift_counter     │  SHR       │
   └───────────────────┴─────────────┴───────────┴────────┴───────────────────────┴────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `shift_type` | enum | 10-9 | <br><br>**Values:**<br>- `Arithmetic`: Arithmetic shift. During right shift, bit 15 is extended. During left shift, zeros are shifted in from right.<br>- `ROT`: Rotational shift. Most and least significant bits are connected.<br>- `ZIN`: Zero end input<br>- `LIN`: Link end input. The last vacated bit is fed to M after every shift instruction.<br> |
| `register` | enum | 7-8 | The register to shift |
| `mask6` | numeric | 6 | Always 0 |
| `SHR` | assembler_flag | 0 | This a feature of the assembler. This mnemonic can be used to specify shift right, so that instead of calculating the 2's complement for the number of right shifts required, SHR can be used. |
| `shift_counter` | int6 | 5-0 | The number of bits to shift the register.<br>Bits 0-4 are the number of shifts.<br>Bit 5=1, then shift right (max 32 times)<br>Bit 5=0, then shift left (max 31 times)<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `M` | Multi-shift link flag |

--------------------------------------------------------------------------------

### SHD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHD <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | User |
| Mask | `1111_1001_1000_0000` |

#### 📝 Description

Shift D register
Every shift instruction places the last bit discarded in the multi-shift flag (M). M can be used as an inputfor the next shift instruction. M is bit 8 of the STS Register.

#### 📋 Format

```
SHD <shift_type> <shift_counter>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10      9  │           │    6   │ 5   4   3   2   1   0 │   0        │
   ├───────────────────┼─────────────┼───────────┼────────┼───────────────────────┼────────────┤
   │       opcode      │  shift_type │  register │  mask6 │     shift_counter     │  SHR       │
   └───────────────────┴─────────────┴───────────┴────────┴───────────────────────┴────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `shift_type` | enum | 10-9 | <br><br>**Values:**<br>- `Arithmetic`: Arithmetic shift. During right shift, bit 15 is extended. During left shift, zeros are shifted in from right.<br>- `ROT`: Rotational shift. Most and least significant bits are connected.<br>- `ZIN`: Zero end input<br>- `LIN`: Link end input. The last vacated bit is fed to M after every shift instruction.<br> |
| `register` | enum | 7-8 | The register to shift |
| `mask6` | numeric | 6 | Always 0 |
| `SHR` | assembler_flag | 0 | This a feature of the assembler. This mnemonic can be used to specify shift right, so that instead of calculating the 2's complement for the number of right shifts required, SHR can be used. |
| `shift_counter` | int6 | 5-0 | The number of bits to shift the register.<br>Bits 0-4 are the number of shifts.<br>Bit 5=1, then shift right (max 32 times)<br>Bit 5=0, then shift left (max 31 times)<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `M` | Multi-shift link flag |

--------------------------------------------------------------------------------

### SHDE

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHDE` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Decimal shift.

(op2) ⟵ (op1) shifted

This instruction is used to normalize operands for decimal operations.

The shift count determines whether the operand is shifted to the left or right:
  (op2 - op1) positive then shift op1 to right
  (op2 - op1) negative then shift op1 to left

If significant digits are lost by carrying out a left shift, an error is generated, directing the program counter to the instruction after the SHDE (the error return). If no errors occur this instruction is skipped.

The digits of the first operand are shifted and the result is placed in the second operand's memory location.

The number of places shifted is given by the difference in decimal position of the two operands. This normalizes the first operand (op1) to the second (op2) for decimal operations such as ADDD.

The sign of the normalized operand (op1) is as follows:

    BCD    Sign
    14₈    +
    15₈    -

An unsigned operand is converted to a plus unless bit 13 of the descriptor D2 is set, when the BCD equivalent of unsigned (17₈) is used.
The sign and digits of the first operand are checked before execution and any illegal digit codes reported.

If bit 10 of descriptor D2 (op2) is set the result is rounded, that is a 1 is added to the operand if the last digit shifted out of the field is ≥ 5.

**Instruction sequence:**

    SHDE
    error handling instruction
    next instruction after SHDE or after error handling routine

**Example:**

    SHDE
    JMP *10₈
    SAD 20₈

The SHDE instruction causes the program counter to skip the next instruction UNLESS an error has been generated, when the instruction immediately following the SHDE will handle the error in some way (in this example a jump is executed on error to (P) + 10₈)).

(* is the assembler mnemonic for the P register)


#### 📋 Format

```
SHDE
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SHT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHT <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | User |
| Mask | `1111_1001_1000_0000` |

#### 📝 Description

Shift T register
Every shift instruction places the last bit discarded in the multi-shift flag (M). M can be used as an inputfor the next shift instruction. M is bit 8 of the STS Register.

#### 📋 Format

```
SHT <shift_type> <shift_counter>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10      9  │           │    6   │ 5   4   3   2   1   0 │   0        │
   ├───────────────────┼─────────────┼───────────┼────────┼───────────────────────┼────────────┤
   │       opcode      │  shift_type │  register │  mask6 │     shift_counter     │  SHR       │
   └───────────────────┴─────────────┴───────────┴────────┴───────────────────────┴────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `shift_type` | enum | 10-9 | <br><br>**Values:**<br>- `Arithmetic`: Arithmetic shift. During right shift, bit 15 is extended. During left shift, zeros are shifted in from right.<br>- `ROT`: Rotational shift. Most and least significant bits are connected.<br>- `ZIN`: Zero end input<br>- `LIN`: Link end input. The last vacated bit is fed to M after every shift instruction.<br> |
| `register` | enum | 7-8 | The register to shift |
| `mask6` | numeric | 6 | Always 0 |
| `SHR` | assembler_flag | 0 | This a feature of the assembler. This mnemonic can be used to specify shift right, so that instead of calculating the 2's complement for the number of right shifts required, SHR can be used. |
| `shift_counter` | int6 | 5-0 | The number of bits to shift the register.<br>Bits 0-4 are the number of shifts.<br>Bit 5=1, then shift right (max 32 times)<br>Bit 5=0, then shift left (max 31 times)<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `M` | Multi-shift link flag |

--------------------------------------------------------------------------------

### SKP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SKP <dr> <condition> <sr>` |
| Category | Skip Instruction |
| Privilege | User |
| Mask | `1111_1000_1100_0000` |

#### 📝 Description

The next instruction is skipped if a specified condition is true.


#### 📚 Detailed Information

SKP can be qualified by eight different mnemonics which use the flags set by the following expression:

    (dr) - (sr)

Four flags are affected by this calculation:

    S  sign
    Z  zero result (error)
    C  carry
    O  overflow

The condition codes are as follows:

    ┌───────┬─────────┬──────────────────────────────┬────────────────────────┐
    │ bits  │ mnemonic│ description                  │ flag(s) condition      │
    │10 9 8 │         │                              │ if true                │
    ├───────┼─────────┼──────────────────────────────┼────────────────────────┤
    │ 0 0 0 │ EQL     │ Equal                        │ Z = 1                  │
    │ 0 0 1 │ GEQ     │ Greater or equal to †        │ S = 0                  │
    │ 0 1 0 │ GRE     │ Greater or equal to * †      │ S + O = 0              │
    │ 0 1 1 │ MGRE    │ Magnitude greater or equal * │ C = 1                  │
    │ 1 0 0 │ UEQ     │ Unequal                      │ Z = 0                  │
    │ 1 0 1 │ LSS     │ Less than †                  │ S = 1                  │
    │ 1 1 0 │ LST     │ Less than * †                │ S + O = 1              │
    │ 1 1 1 │ MLST    │ Magnitude less than *        │ C = 0                  │
    └───────┴─────────┴──────────────────────────────┴────────────────────────┘

* Overflow is taken care of
† Contents of sr and dr are treated as signed numbers

Note:
By swapping the sr and dr fields, these relationships can be tested:
  >  Greater than
  <= Less than or equal


#### 📋 Format

```
SKP <dr> <condition> <sr>
```

#### Bit Layout

```
   │ 15  14  13  12  11│ 10  9   8  │ 7   6  │ 5   4   3 │ 2   1   0      │
   ├───────────────────┼────────────┼────────┼───────────┼────────────────┤
   │       opcode      │  condition │  zeros │     sr    │     dr         │
   └───────────────────┴────────────┴────────┴───────────┴────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `condition` | enum | 10-8 | Condition to skip the next instruction<br><br>**Values:**<br>- `EQL`: Equal<br>- `GEQ`: Greater or equal to (signed)<br>- `GRE`: Greater or equal to (overflow, signed)<br>- `MGRE`: Magnitude greater or equal to (overflow, unsigned)<br>- `UEQ`: Unequal<br>- `LSS`: Less than (overflow, unsigned)<br>- `LST`: Less than (overflow, signed)<br>- `MLST`: Magnitude less than (overflow, unsigned)<br> |
| `zeros` | value | 7-6 | Must be 00 |
| `sr` | src_register | 5-3 | The source register to be compared with the destination register |
| `dr` | dst_register | 2-0 | The destination register to be compared with the source register |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `Z` | Error flag |
| `C` | Carry flag |
| `O` | Static overflow flag |

#### 📚 Examples

##### Skip next instruction if the D register contents equal that of the A register.
```
SKP DD EQL SL
```

##### Skip the next instruction if the contents of the A register are less than the B register contents.
OR
Skip the next instruction if the contents of the B register are greater than the A register contents.

```
SKP DB LSS SA
```

##### Skip the next instruction if the contents of the L register do not equal zero.
```
SKP DL UEQ
```

##### Skip the next instruction if the D register is less than zero.
```
SKP LSS SD
```

--------------------------------------------------------------------------------

### SRB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SRB <level>` |
| Category | Register Block Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Store register block

Load the register block of the program level given in the instruction into the memory block pointed to by the X register.

If the instruction specifies the current program level, the P register points to the instruction following SRB.


#### 📚 Detailed Information

The register block of the specified program level is stored in the memory block pointed to by the X register. The mapping is as follows:

    (ea)     ⟵ P (level N)
    (ea + 1) ⟵ X (level N)
    (ea + 2) ⟵ T (level N)
    (ea + 3) ⟵ A (level N)
    (ea + 4) ⟵ D (level N)
    (ea + 5) ⟵ L (level N)
    (ea + 6) ⟵ STS (level N)
    (ea + 7) ⟵ B (level N)

Where (ea) is the address in memory pointed to by the X register.

If the current program level is specified, the P register points to the instruction following SRB.


#### 📋 Format

```
SRB <level>
```

#### Bit Layout

```
   │ 15  14  13  12  11│ 6   5   4   3 │ 2   1   0    │
   ├───────────────────┼───────────────┼──────────────┤
   │       opcode      │     level     │    type      │
   └───────────────────┴───────────────┴──────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `level` | uint4 | 6-3 | The level to load the register block to |
| `type` | enum | 2-0 | The type of register block function<br><br>**Values:**<br>- `SRB`: Store Register Block<br>- `LRB`: Load Register Block<br> |

#### 📚 Examples

##### SRB 100₈ (152702₈)

Store the register block of program level 8 in the memory block pointed to by X.

    (ea)     ⟵ P (level 8)
    (ea + 1) ⟵ X (level 8)
    ↓         ↓
    (ea + 7) ⟵ B (level 8)

```
SRB 100
```

--------------------------------------------------------------------------------

### STA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STA <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store A register to memory location 
(EA) ⟵ (A)

Store the contents of the A register in the memory location pointed to by the effective address.


#### 📋 Format

```
STA <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STATX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STATX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store A register

(ea) ⟵ (A)

Store the contents of the A register in the memory location given by the effective address.


#### 📋 Format

```
STATX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### STD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STD <addressing_mode> <displacement>` |
| Category | Memory Transfer - Double word instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store double word
  (ea)     ⟵ (A)
  (ea) + 1 ⟵ (D)

Store the contents of the A register in the memory location pointed to by the effective address
Store the contents of the D register in the memory location pointed to by the effective address plus one.


#### 📋 Format

```
STD <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STDTX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STDTX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store double word

(ea) ⟵ (A)
(ea) + 1 ⟵ (D)

Store the double word held in the A and D registers in the memory locations given by the effective address and the effective address plus one.


#### 📋 Format

```
STDTX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### STF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STF <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store floating accumulator (TAD) to memory (ea)
Memory format:
 (ea)     ⟵ (T)
 (ea) + 1 ⟵ (A)
 (ea) + 2 ⟵ (D)

Store the contents of the floating accumulator (T,A and D registers) into the memory location pointed to by the effective address.


#### 📋 Format

```
STF <addressing_mode> <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STT <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store T register to memory location
(EA) ⟵ (T)

Store the contents of the T register in the memory location pointed to by the effective address.


#### 📋 Format

```
STT <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STX <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store X register to memory location 
(EA) ⟵ (X)

Store the contents of the X register in the memory location pointed to by the effective address.


#### 📋 Format

```
STX <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STZ <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store zero to memory location
(EA) ⟵ 0

Store the value zero in the memory location pointed to by the effective address.


#### 📋 Format

```
STZ <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### STZTX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STZTX <displacement>` |
| Category | Memory Transfer Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store zero

(ea) ⟵ 000000₈

Store zero in the memory location given by the effective address.


#### 📋 Format

```
STZTX <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### SUB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SUB <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | User |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Subtract from A register
A ⟵ A - (EL)

Subtract the contents of the memory location pointed to by the effective address from the A register, leaving the result in A.


#### 📚 Detailed Information

Flags affected:
- C=1: If a carry occurs from the signed bit positions of the adder.
- O=1, Q=1: If an overflow occurs, that is if the signs of the two operands are equal and the sign of the result is different.
- O=1, Q=0: If overflow does not exist, the dynamic overflow flag (O) is reset to 0 while the static overflow flag (Q) is left unchanged.


#### 📋 Format

```
SUB <addressing_mode> <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10    9     8   │ 7   6   5   4   3   2   1   0    │
   │                   │  X     I     B   │                                  │
   ├───────────────────┼──────────────────┼──────────────────────────────────┤
   │       opcode      │  addressing_mode │          displacement            │
   └───────────────────┴──────────────────┴──────────────────────────────────┘

```

> **Note:** This instruction uses addressing modes. See [Addressing Modes](#addressing-modes) for details.

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode determines what type of operation occurs |
| `addressing_mode` | addressing_modes | 10-8 | These three bits give the addressing mode for the instruction |
| `displacement` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `Q` | Dynamic overflow flag |

--------------------------------------------------------------------------------

### SUBD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SUBD` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Subtract two decimal operands.

(op1) ⟵ (op1) - (op2)

Subtract the second operand from the first operand, leaving the result in the location of the first operand.

If the first operand field is too short to contain all the significant digits of the result then decimal overflow occurs.

If bit 13 of D2 in the first operand is set, the sign of the result will be 17₈ (BCD unsigned).

Any empty operand, that is with a field length of zero, is treated as a positive zero.

A zero difference can have either a negative or positive sign.

**Instruction sequence:**

    SUBD
    error handling instruction
    next instruction after SUBD or after error handling routine

**Example:**

    SUBD
    JPL *30₈
    ADD *15₈

The SUBD instruction causes the program counter to skip the next instruction UNLESS an error has been generated, when the instruction immediately following the SUBD will handle the error in some way (in this example, a jump is executed on error to a subroutine at (P) + 15₈)).

(* is the assembler mnemonic for the P register)


#### 📋 Format

```
SUBD
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### SWAP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SWAP <sub-instruction(s)> <source> <destination>` |
| Category | Register Operations |
| Privilege | User |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Register swap

dr ⟵ sr


#### 📚 Detailed Information

Optional sub-instructions:

- **CLD=1**: Zero is used instead of the destination register as an operand (dr register contents are unchanged).
- **CM1=1**: The 1's complement of the source register is used as an operand (sr register contents are unchanged).

---

### Instruction combinations

SWAP <sr> <dr>           dr ⟵ sr
SWAP CLD <sr> <dr>       dr ⟵ sr; sr ⟵ 0
SWAP CM1 <sr> <dr>       dr ⟵ sr; sr ⟵ dr
SWAP CM1 CLD <sr> <dr>   dr ⟵ sr; sr ⟵ 0


#### 📋 Format

```
SWAP <sub-instruction(s)> <source> <destination>
```

#### Bit Layout

```
   │ 15  14  13  12  11│  10  │   9  │   8  │ 8   7 │   7  │   6  │ 5   4   3 │  2    1    0          │
   ├───────────────────┼──────┼──────┼──────┼───────┼──────┼──────┼───────────┼───────────────────────┤
   │       opcode      │  rad │  ADC │  AD1 │  CM2  │  CM1 │  CLD │   source  │  destination          │
   └───────────────────┴──────┴──────┴──────┴───────┴──────┴──────┴───────────┴───────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-11 | The opcode for register inclusive OR |
| `rad` | constant | 10 | Logical operation flag |
| `ADC` | flag | 9 | Also addd old carry to destination |
| `AD1` | flag | 8 | Also add one to destination |
| `CM1` | flag | 7 | Use one's complement of source |
| `CM2` | flag | 8-7 | Two's complement (CM1 AD1) |
| `CLD` | flag | 6 | Clear destination before operation |
| `source` | enum | 5-3 | Source register (sr)<br><br>**Values:**<br>- `SD`: D register as source<br>- `SP`: P register as source<br>- `SB`: B register as source<br>- `SL`: L register as source<br>- `SA`: A register as source<br>- `ST`: T register as source<br>- `SX`: X register as source<br>- `ZERO`: Source value equals zero<br> |
| `destination` | enum | 2-0 | Destination register (dr)<br><br>**Values:**<br>- `DD`: D register as destination<br>- `DP`: P register as destination<br>- `DB`: B register as destination<br>- `DL`: L register as destination<br>- `DA`: A register as destination<br>- `DT`: T register as destination<br>- `DX`: X register as destination<br> |

#### 🚩 Flags Affected

| Flag | Description |
|:-----|:------------|
| `C` | Carry flag |
| `O` | Static overflow flag |
| `Q` | Dynamic overflow flag |

#### 📚 Examples

##### Exchange A and D register contents.
```
SWAP SA DD
```

##### Use zero as the destination operand. Exchange the A and X register contents. A register contents are zero, X register contents are the previous contents of A.

```
SWAP CLD SA DX
```

--------------------------------------------------------------------------------

### SZCB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SZCB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store zero in the core map-table bank (CMBNK).

(ea) <--- 0

ea = (B) + Δ = CMBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)  


#### 📋 Format

```
SZCB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### SZSB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SZSB <displacement>` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1100_0111` |

#### 📝 Description

Store zero in the segment-table bank (STBNK).

(ea) <--- 0

ea = (B) + Δ = STBNK entry

Δ: 3-bit displacement added to B included in the instruction opcode (bits 5-3)  


#### 📋 Format

```
SZSB <displacement>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

--------------------------------------------------------------------------------

### TRA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `TRA <internal_register>` |
| Category | Register Transfer |
| Privilege | Privileged |
| Mask | `1111_1111_1111_0000` |

#### 📝 Description

Transfer to A register
The internal register given in the instruction is copied into the A register.  


#### 📋 Format

```
TRA <internal_register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  3    2    1    0    │
   ├───────────────────────────────────────────────┼──────────────────────┤
   │                     opcode                    │  internal_register   │
   └───────────────────────────────────────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode determines what type of operation occurs |
| `internal_register` | enum | 3-0 | The CPU internal register<br><br>**Values:**<br>- `PANS`: Panel status<br>- `STS`: Status register<br>- `OPR`: Operator's panel switch register<br>- `PSR`: Paging status register<br>- `PVL`: Previous level code register<br>- `IIC`: Internal interrupt code register<br>- `PID`: Priority interrupt detect register<br>- `PIE`: Priority enable detect register<br>- `CSR`: Cache status register<br>- `ACTL`: Active level register<br>- `ALD`: Automatic load descriptor<br>- `PES`: Parity error status register<br>- `PGC`: Paging control register<br>- `PEA`: Parity error address register<br>- `CS`: Control store<br> |

--------------------------------------------------------------------------------

### TRR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `TRR <internal_register>` |
| Category | Register Transfer |
| Privilege | Privileged |
| Mask | `1111_1111_1111_0000` |

#### 📝 Description

Transfer A to internal register
The internal register given in the instruction is loaded with the contents of the A register.


#### 📋 Format

```
TRR <internal_register>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4 │  3    2    1    0    │
   ├───────────────────────────────────────────────┼──────────────────────┤
   │                     opcode                    │  internal_register   │
   └───────────────────────────────────────────────┴──────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-4 | The opcode determines what type of operation occurs |
| `internal_register` | enum | 3-0 | The CPU internal register<br><br>**Values:**<br>- `PANC`: Panel control<br>- `STS`: Status register<br>- `LMP`: Panel data display buffer register<br>- `PCR`: Paging control register<br>- `IIE`: Internal interrupt enable register<br>- `PID`: Priority interrupt detect register<br>- `PIE`: Priority interrupt enable register<br>- `CCL`: Cache clear register<br>- `LCIL`: Lower cache inhibit limit register<br>- `UCIL`: Upper cache inhibit limit register<br>- `CILP`: Cache inhibit page register<br>- `ECCR`: Error correction control register<br>- `CS`: Control Store<br> |

--------------------------------------------------------------------------------

### TSET

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `TSET` |
| Category | Memory Examine and Test Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Test and set

(T) points to the virtual memory word to be accessed
(A) ⟵ memory word addressed by T

The address given by the T register is a logical memory address. It is normally translated into a physical address using page tables (if memory management is on). The contents of the location addressed by T are simultaneously loaded into the A register as the location is written to with all 1s. The memory system is dedicated to this task and no other memory access is allowed during the operation. This can be used for processor synchronization.

The old content of the memory address is always read from the memory and never from cache. The all 1s' data word is never written to cache.


#### 📋 Format

```
TSET
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### TSETP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `TSETP` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Test and set physical memory word.


#### 📚 Detailed Information

(T) points to the physical memory bank to be accessed
(X) points to the address within the bank
(A) is loaded with the word

The contents of the location addressed by T and X are simultaneously loaded into the A register as the location is written to with all 1s. No other memory access is allowed during this operation.
The old content of the memory address is always read from the memory and never from cache. The all 1s' data word is never written to cache.
This instruction can be used for processor synchronization.


#### 📋 Format

```
TSETP
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### UPACK

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `UPACK` |
| Category | Decimal Instructions |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Convert to ASCII.

(op2) ⟵ (op1) in ASCII format

Convert the first operand from its BCD format to ASCII format, placing the result in the second operand location.

**Conversion process:**
The instruction carries out the following steps:
1. Checks the sign and digits of the operand (op1) are encoded as BCD digits. (Illegal codes generate an error code 2.)
2. Takes each BCD digit as the lower nibble of an equivalent ASCII digit. Sets the upper nibble of the ASCII byte (the zone) to 0011₂.
3. Converts the BCD sign of the operand to ASCII:

       BCD   ASCII   Sign
       0     53₈     +
       1     55₈     -
    Note: If bit 13 of the descriptor D2 is set then the code 17₈ (BCD unsigned) is used.
4. Extends the second operand field (op2) with ASCII zeros (60₈) if the result is too small to fill the field. Reports overflow has occurred (error code 3), if the second operand field length is too short to contain the significant digits of the result (the remaining digits are ignored).

**Error code:**
An error code is placed in bits 0 to 4 of the D register if an illegal code conversion is attempted or if overflow occurs. The first detected error will be reported.

    Error code: 2   illegal code†
                3   overflow
    † bit 15 of both the A and D registers point to the byte containing the illegal code.


#### 📋 Format

```
UPACK
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### VERSN

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `VERSN` |
| Category | System/CPU Information |
| Privilege | User |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

** ND-110/ND-120 ONLY**
Read ND-110 CPU version and installation number.

This instruction is used to read the version of ND-110 CPU installed. Three registers are loaded simultaneously with information in the following format:


#### 📚 Detailed Information

A register

    15                             3 0
    ┌──────────────────────────────┬───┐
    │      Print version           │ALD│
    └──────────────────────────────┴───┘

T register

    15                             0
    ┌───────────────────────────────┐
    │   Microprogram version        │
    └───────────────────────────────┘

D register

    15       7                   0
    ┌────────┬───────────────────┐
    │        │ Installation num. │
    └────────┴───────────────────┘


The installation number of the CPU is 16 bytes long.

The VERSN instruction can only load one byte of the installation number into the D register each time it is executed. The A register is used to address the sixteen bytes.

To read a byte of the installation number the A register must be loaded with an installation byte address before the VERSN instruction is executed. The address of the byte to be read is given by the value of bits 8-11 in the A register (equivalent to bytes 0-16).

To read the complete installation number both the A register bit field must be incremented and the VERSN instruction executed sixteen times.


#### 📋 Format

```
VERSN
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

### WAIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `WAIT <wait_number>` |
| Category | Interrupt Control Instructions |
| Privilege | User |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

When interrupt system off: halts the program and enters the operator's communication. 
When interrupt system on: give up priority. If there are no interrupt requests on any level, the program on level zero is entered.


#### 📚 Detailed Information

This operates as follows:

**If the interrupt system is OFF:**
The ND-110 stops with the program counter (P register) pointing to the instruction after the WAIT and the front panel RUN indicator is turned off. (To restart the system, type `!` on the console terminal.)

**If the interrupt system is ON:**
The ND-110 exits from the current program level (resetting the corresponding PID bit) and enters the program level with the highest priority, normally a program level lower than the one which executes the WAIT instruction. If there are no interrupt requests awaiting service, then program level 0 is entered.

**Note:**
A WAIT on program level 0 is ignored.

WAIT followed by a number less than 400₈ can be used to detect which location caused the program stop.


#### 📋 Format

```
WAIT <wait_number>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │          wait_number            │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `wait_number` | numeric | 7-0 | Wait number |

#### 📚 Examples

##### Wait
```
WAIT
```

##### Wait, but with a comment saying its WAIT #1
```
WAIT 1
```

--------------------------------------------------------------------------------

### WGLOB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `WGLOB` |
| Category | Control Instructions |
| Privilege | Privileged |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Initialize global pointers.

(T) = bank number of segment table (STBNK)
(A) = start address within bank (STSRT)*
(D) = bank number of core map table (CMBNK)

* must be divisible by 8


#### 📋 Format

```
WGLOB
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0  │
   ├────────────────────────────────────────────────────────────────┤
   │                             opcode                             │
   └────────────────────────────────────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-0 | The opcode determines what type of operation occurs |

--------------------------------------------------------------------------------

