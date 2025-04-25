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
- [Control Transfer](#control-transfer)
- [Execute](#execute)
- [Floating Conversion (Standard Format)](#floating-conversion-(standard-format))
- [Input/Output](#input/output)
- [Inter-level Instructions](#inter-level-instructions)
- [Interrupt Control Instructions](#interrupt-control-instructions)
- [Interrupt Identification](#interrupt-identification)
- [Memory Transfer - Double word instructions](#memory-transfer---double-word-instructions)
- [Memory Transfer - Load Instruction](#memory-transfer---load-instruction)
- [Memory Transfer - Store Instruction](#memory-transfer---store-instruction)
- [Monitor Calls](#monitor-calls)
- [Register Block Instructions](#register-block-instructions)
- [Register Operations](#register-operations)
- [Register Transfer](#register-transfer)
- [Sequencing Instructions](#sequencing-instructions)
- [Shift Instructions](#shift-instructions)
- [Skip Instruction](#skip-instruction)
- [Standard Floating Instructions](#standard-floating-instructions)
- [System Control Instructions](#system-control-instructions)

### 📖 Instruction Reference
- [AAA](#aaa)
- [AAB](#aab)
- [AAT](#aat)
- [AAX](#aax)
- [ADD](#add)
- [AND](#and)
- [BANC](#banc)
- [BAND](#band)
- [BLDA](#blda)
- [BLDC](#bldc)
- [BORA](#bora)
- [BORC](#borc)
- [BSET](#bset)
- [BSKP](#bskp)
- [BSTA](#bsta)
- [BSTC](#bstc)
- [COPY](#copy)
- [DNZ](#dnz)
- [EXIT](#exit)
- [EXR](#exr)
- [FAD](#fad)
- [FDV](#fdv)
- [FMU](#fmu)
- [FSB](#fsb)
- [IDENT](#ident)
- [IOF](#iof)
- [ION](#ion)
- [IOX](#iox)
- [IOXT](#ioxt)
- [IRR](#irr)
- [IRR](#irr)
- [JAF](#jaf)
- [JAN](#jan)
- [JAP](#jap)
- [JAZ](#jaz)
- [JMP](#jmp)
- [JNC](#jnc)
- [JPC](#jpc)
- [JPL](#jpl)
- [JXN](#jxn)
- [JXZ](#jxz)
- [LBYT](#lbyt)
- [LDA](#lda)
- [LDD](#ldd)
- [LDF](#ldf)
- [LRB](#lrb)
- [MCL](#mcl)
- [MIN](#min)
- [MON](#mon)
- [MPY](#mpy)
- [MST](#mst)
- [NLZ](#nlz)
- [ORA](#ora)
- [POF](#pof)
- [PON](#pon)
- [RADD](#radd)
- [RAND](#rand)
- [RCLR](#rclr)
- [RDIV](#rdiv)
- [REXO](#rexo)
- [RMPY](#rmpy)
- [RORA](#rora)
- [RSUB](#rsub)
- [SAA](#saa)
- [SAB](#sab)
- [SAD](#sad)
- [SAT](#sat)
- [SAX](#sax)
- [SBYT](#sbyt)
- [SHA](#sha)
- [SHD](#shd)
- [SHT](#sht)
- [SKP](#skp)
- [SRB](#srb)
- [STA](#sta)
- [STD](#std)
- [STF](#stf)
- [STT](#stt)
- [STX](#stx)
- [STZ](#stz)
- [SUB](#sub)
- [SWAP](#swap)
- [TRA](#tra)
- [TRR](#trr)
- [WAIT](#wait)

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

--------------------------------------------------------------------------------

### Arithmetic and Logical

| Instruction | Description |
|:------------|:------------|
| [`ADD`](#add) | Add to A register (C, O and Q may also be affected) A: = A + (EL) |
| [`AND`](#and) | Logical AND to A register A: = A & (EA) |
| [`MPY`](#mpy) | Multiply integer (O and Q may also be affected) A: = A * (EL) |
| [`ORA`](#ora) | Logical inclusive OR to A register A: = A | (EA) |
| [`SUB`](#sub) | Subtract from A register (C and Q may also be affected) A: = A - (EA) |

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
| [`BSTC`](#bstc) | Store complement and set K |

--------------------------------------------------------------------------------

### Bit instructions_with_destination

| Instruction | Description |
|:------------|:------------|
| [`BORC`](#borc) | Logical OR with bit complement |

--------------------------------------------------------------------------------

### Byte Instructions

| Instruction | Description |
|:------------|:------------|
| [`LBYT`](#lbyt) | Load byte from memory to A register |
| [`SBYT`](#sbyt) | Store byte from A register to memory |

--------------------------------------------------------------------------------

### Control Transfer

| Instruction | Description |
|:------------|:------------|
| [`TRA`](#tra) | Transfer control to the address specified by the effective address. |

--------------------------------------------------------------------------------

### Execute

| Instruction | Description |
|:------------|:------------|
| [`EXR`](#exr) | Execute instruction found in specified register |

--------------------------------------------------------------------------------

### Floating Conversion (Standard Format)

| Instruction | Description |
|:------------|:------------|
| [`DNZ`](#dnz) | Convert the floating number in TAD to a fixed point number in A |
| [`NLZ`](#nlz) | Normalize |

--------------------------------------------------------------------------------

### Input/Output

| Instruction | Description |
|:------------|:------------|
| [`IOX`](#iox) | Exchange information between I/O system and A register.     |
| [`IOXT`](#ioxt) | Exchange information between I/O system and A register.     |

--------------------------------------------------------------------------------

### Inter-level Instructions

| Instruction | Description |
|:------------|:------------|
| [`IRR`](#irr) | Inter Register Read |
| [`IRR`](#irr) | Inter Register Write |

--------------------------------------------------------------------------------

### Interrupt Control Instructions

| Instruction | Description |
|:------------|:------------|
| [`IOF`](#iof) | Interrupt System OFF |
| [`ION`](#ion) | Interrupt System ON |
| [`POF`](#pof) | Memory management OFF |
| [`PON`](#pon) | Memory management ON |

--------------------------------------------------------------------------------

### Interrupt Identification

| Instruction | Description |
|:------------|:------------|
| [`IDENT`](#ident) | Transfer IDENT code of interrupting device with highest priority on the specified level to A register.  |

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

--------------------------------------------------------------------------------

### Memory Transfer - Store Instruction

| Instruction | Description |
|:------------|:------------|
| [`MIN`](#min) | Memory increment and skip next instruction if zero (EA): = (EA) + 1 |
| [`STA`](#sta) | Store A register to memory location (EA): = A |
| [`STT`](#stt) | Store T register to memory location (EA): = T |
| [`STX`](#stx) | Store X register to memory location (EA): = X |
| [`STZ`](#stz) | Store zero to memory location (EA): = 0 |

--------------------------------------------------------------------------------

### Monitor Calls

| Instruction | Description |
|:------------|:------------|
| [`MON`](#mon) | The MON instruction is used in special different contexts when running under an operating system. |

--------------------------------------------------------------------------------

### Register Block Instructions

| Instruction | Description |
|:------------|:------------|
| [`LRB`](#lrb) | Load register block from memory to specified level |
| [`SRB`](#srb) | Store register block from specified level to memory |

--------------------------------------------------------------------------------

### Register Operations

| Instruction | Description |
|:------------|:------------|
| [`COPY`](#copy) | Copy source to destination |
| [`EXIT`](#exit) | Return from subroutine |
| [`RADD`](#radd) | Add source to destination (dr): = (dr) + (sr) |
| [`RAND`](#rand) | Logical AND to destination (dr): = (dr) ∧ (sr) |
| [`RCLR`](#rclr) | Registr clear |
| [`RDIV`](#rdiv) | Divide double accumulator with source; quotient in A, remainder in D |
| [`REXO`](#rexo) | Logical exclusive OR  |
| [`RMPY`](#rmpy) | Multiply source with destination; result in double accumulator |
| [`RORA`](#rora) | Logical inclusive OR (dr): = (dr) ∨ (sr) |
| [`RSUB`](#rsub) | Subtract source from destination (dr): = (dr) - (sr) |
| [`SWAP`](#swap) | Register exchange |

--------------------------------------------------------------------------------

### Register Transfer

| Instruction | Description |
|:------------|:------------|
| [`MCL`](#mcl) | Masked clear |
| [`MST`](#mst) | Masked set |
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

### Standard Floating Instructions

| Instruction | Description |
|:------------|:------------|
| [`FAD`](#fad) | Add floating word (FW) to floating accumulator (TAD) |
| [`FDV`](#fdv) | Divide floating point accumulator The contents of the floating point accumulator (A and D registers) are divided by the contents of two sequential memory locations, pointed to by the effective address.  |
| [`FMU`](#fmu) | Multiply floating point accumulator. |
| [`FSB`](#fsb) | Subtract floating point accumulator. |
| [`LDF`](#ldf) | Load floating accumulator (TAD) from memory (FW) |
| [`STF`](#stf) | Store floating accumulator (TAD) to memory (ea) |

--------------------------------------------------------------------------------

### System Control Instructions

| Instruction | Description |
|:------------|:------------|
| [`WAIT`](#wait) | When interrupt system off: halts the program and enters the operator’s communication.  |

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
| Privilege | user |
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
| Privilege | user |
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
| Privilege | user |
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
| Privilege | user |
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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Add to A register (C, O and Q may also be affected) A: = A + (EL)

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

### AND

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `AND <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Logical AND to A register A: = A & (EA)

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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

ogical AND with bit compl

K: = K & (B)0


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical AND to K

K: = K & (B)


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

### BLDA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `BLDA` |
| Category | Bit instructions |
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Load K

K := bit


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Load K and complement

K := (B)0;


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical OR to K

K: = K | (B)


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Logical OR with bit complement

K: = K | (B)0


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Set specified bit in equal to specified condition

P = P+1


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Skip next location if specified condition is true

P = P+1


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Store and clear K

(B): = K; K: = 0


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
| Privilege | user |
| Mask | `1111_1111_1000_0000` |

#### 📝 Description

Store complement and set K

(B): = K0; K: = 1


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

### COPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `COPY <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Copy source to destination

#### 📋 Format

```
COPY <source> <destination>
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

--------------------------------------------------------------------------------

### DNZ

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `DNZ <scaling_factor>` |
| Category | Floating Conversion (Standard Format) |
| Privilege | user |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Convert the floating number in TAD to a fixed point number in A

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

### EXIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `EXIT` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Return from subroutine

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

--------------------------------------------------------------------------------

### EXR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `EXR <register>` |
| Category | Execute |
| Privilege | user |
| Mask | `1111_1111_1111_1000` |

#### 📝 Description

Execute instruction found in specified register

#### 📋 Format

```
EXR <register>
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

--------------------------------------------------------------------------------

### FAD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `FAD <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Add floating word (FW) to floating accumulator (TAD)
TAD := TAD + FW

# Add floating word to floating accumulator
TAD := TAD + (FW)

# Check for overflow/underflow conditions
if overflow:
  set_overflow_flag()
if underflow:
  set_underflow_flag()


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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Divide floating point accumulator The contents of the floating point accumulator (A and D registers) are divided by the contents of two sequential memory locations, pointed to by the effective address. 
The result is held in the accumulator


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
| Privilege | user |
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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Subtract floating point accumulator.

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

### IDENT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` IDENT <level_code>` |
| Category | Interrupt Identification |
| Privilege | user |
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

### IOF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOF` |
| Category | Interrupt Control Instructions |
| Privilege | system |
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
| Privilege | user |
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

### IOX

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IOX <device_register_address>` |
| Category | Input/Output |
| Privilege | system |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Exchange information between I/O system and A register.    

The transfer direction is input if the device-register address is even, and output if the device-register address is odd.


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
| `opcode` | opcode | 15-10 | The opcode determines what type of operation occurs |
| `device_register_address` | displacement | 10-0 | 11-bit unigned field gives the device register address |

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
| Category | Input/Output |
| Privilege | system |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Exchange information between I/O system and A register.    

The transfer direction is input if the device-register address is even, and output if the device-register address is odd.

The IOXT instruction uses the 16-bit T register to hold the device-register address, and, in theory, can address 64 K register
addresses (0-177777). Only some of these addresses are legal however.


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
| Privilege | privileged |
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

--------------------------------------------------------------------------------

### IRR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `IRW <level> <register>` |
| Category | Inter-level Instructions |
| Privilege | privileged |
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

--------------------------------------------------------------------------------

### JAF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` JAF <displacement>` |
| Category | Sequencing Instructions |
| Privilege | user |

#### 📝 Description

Condtion: Jump if (A) != 0 (jump if A filled)

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
| Privilege | user |

#### 📝 Description

Condtion: Jump if (A) < 0 (jump if A is negative)

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
| Privilege | user |

#### 📝 Description

Condtion: Jump if (A) > 0 (jump if A positive)

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
| Privilege | user |

#### 📝 Description

Condtion: Jump if (A) == 0 (jump if A is zero)

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
| Format | `JMP <address mode> <disp>` |
| Category | Sequencing Instructions |
| Privilege | user |

#### 📝 Description

Jump - Unconditional jump to specified address
The next instruction is taken from the effective address of the JMP instruction(the effective address is ioaded into the program counter).  


#### 📋 Format

```
JMP <address mode> <disp>
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
| Privilege | user |

#### 📝 Description

Increment X and jump if X is negative    

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
| Privilege | user |

#### 📝 Description

Increment X and jump if X is positive    

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
| Format | `JPL <address mode> <disp>` |
| Category | Sequencing Instructions |
| Privilege | user |

#### 📝 Description

Jump if Plus - Jump to specified address if the result of the last operation was positive (sign bit is 0)

The contents of the program counter are transferred to the L register and the next instruction is taken from the effective address of the JPL instruction.
Note that the L register points to the instruction after the jump(the program counter incremented before transfer to the L register).	


#### 📋 Format

```
JPL <address mode> <disp>
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
| Privilege | user |

#### 📝 Description

Condtion: Jump if (X) < 0 (jump if X negative)

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
| Privilege | user |

#### 📝 Description

Condtion: Jump if (X) == 0 (jump if X is zero)

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

### LBYT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LBYT` |
| Category | Byte Instructions |
| Privilege | user |
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

### LDA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDA <addressing_mode> <disp>` |
| Category | Memory Transfer - Load Instruction |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load A register

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

### LDD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDD <addressing_mode> <displacement>` |
| Category | Memory Transfer - Double word instructions |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load double word
  A <- (ea)
  D <- (ea) + 1

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

### LDF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LDF <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Load floating accumulator (TAD) from memory (FW)
Memory format:
- EL: Exponent
- EL+1, EL+2: Mantissa


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

### LRB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `LRB <level>` |
| Category | Register Block Instructions |
| Privilege | privileged |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Load register block from memory to specified level
P on spec. level: = (EL)
X on spec. level: = (EL) + 1
T on spec. level: = (EL) + 2
A on spec. level: = (EL) + 3
D on spec. level: = (EL) + 4
L on spec. level: = (EL) + 5
STS on spec. level: = (EL) + 6
B on spec. level: = (EL) + 7

Where EL = (X) on current level


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

--------------------------------------------------------------------------------

### MCL

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MCL <internal_register>` |
| Category | Register Transfer |
| Privilege | system |
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

--------------------------------------------------------------------------------

### MIN

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MIN <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Memory increment and skip next instruction if zero (EA): = (EA) + 1

(ea) <- (ea) + 1
(P)  <- (P) + 2 IF new (ea) = 0

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

### MON

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MON <monitor_call_number>` |
| Category | Monitor Calls |
| Privilege | user |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

The MON instruction is used in special different contexts when running under an operating system.
It provides system call functionality through different monitor call numbers.


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

### MPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `MPY <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Multiply integer (O and Q may also be affected) A: = A * (EL)

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
| Privilege | system |
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
| Privilege | user |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Normalize

Convert the number in A to a floating number in TAD

The number in the A register is converted to its floating point equivalent in the floating point accumulator (A and D registers), 
using the scaling factor given, For integers, a scaling factor of +16 will give a floating point number with the same value as the integer.
The larger the scaling factor, the larger the Ploating point number. 

The D register will be cleared when using single precision fixed point numbers.

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

### ORA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `ORA <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Logical inclusive OR to A register A: = A | (EA)

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

### POF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `POF` |
| Category | Interrupt Control Instructions |
| Privilege | system |
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
| Category | Interrupt Control Instructions |
| Privilege | user |
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
| Format | `RADD <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Add source to destination (dr): = (dr) + (sr)

#### 📋 Format

```
RADD <source> <destination>
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

--------------------------------------------------------------------------------

### RAND

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RAND [flags] <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Logical AND to destination (dr): = (dr) ∧ (sr)

#### 📋 Format

```
RAND [flags] <source> <destination>
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

--------------------------------------------------------------------------------

### RCLR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RCLR <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1111_1111` |

#### 📝 Description

Registr clear

#### 📋 Format

```
RCLR <destination>
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

### RDIV

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RDIV <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Divide double accumulator with source; quotient in A, remainder in D
A: = AD/(sr)
(AD = A * (sr) + D)

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

### REXO

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `REXO [flags] <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Logical exclusive OR 

(dr): = (dr) ⊕ (sr)

Optional sub instructions:
  CLD=1 Zero is used instead of the destination register as an operand (dr register contents are unchanged).
  CM1=1 The 1's complement of the source register is used as an operand (sr register contents are unchanged).

Instruction combinations:

  REXO <sr> <dr>          dr <- dr XOR sr
  REXO CLD <sr> <dr>      dr <- sr
  REXO CML <sp> <dr>      dr <- dr XOR sr's (1's complement)
  REXO CML CLD <sr> <dr>  dr <- sr (1's complement)


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

### RMPY

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RMPY <source> <destination>` |
| Category | Register Operations |
| Privilege |  |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Multiply source with destination; result in double accumulator
AD: = (sr) * (dr)


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

--------------------------------------------------------------------------------

### RORA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RORA [flags] <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Logical inclusive OR (dr): = (dr) ∨ (sr)

#### 📋 Format

```
RORA [flags] <source> <destination>
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

--------------------------------------------------------------------------------

### RSUB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `RSUB <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Subtract source from destination (dr): = (dr) - (sr)

#### 📋 Format

```
RSUB <source> <destination>
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

--------------------------------------------------------------------------------

### SAA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAA <number>` |
| Category | Argument Instruction |
| Privilege | user |
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
| Privilege | user |
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

### SAD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAD <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | user |
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

### SAT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SAT <number>` |
| Category | Argument Instruction |
| Privilege | user |
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
| Privilege | user |
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

### SBYT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SBYT` |
| Category | Byte Instructions |
| Privilege | user |
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

### SHA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHA <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | user |
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
| Privilege | user |
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

### SHT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SHT <shift_type> <shift_counter>` |
| Category | Shift Instructions |
| Privilege | user |
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
| Privilege | user |
| Mask | `1111_1000_1100_0000` |

#### 📝 Description

The next instruction is skipped if a specified condition is true.


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
| Privilege | privileged |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Store register block from specified level to memory
(EL): = P on spec. level
(EL) + 1: = X on spec. level
(EL) + 2: = T on spec. level
(EL) + 3: = A on spec. level
(EL) + 4: = D on spec. level
(EL) + 5: = L on spec. level
(EL) + 6: = STS on spec. level
(EL) + 7: = B on spec. level

Where EL = (X) on current level


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

--------------------------------------------------------------------------------

### STA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STA <addressing_mode> <displacement>` |
| Category | Memory Transfer - Store Instruction |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store A register to memory location (EA): = A

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

### STD

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STD <addressing_mode> <displacement>` |
| Category | Memory Transfer - Double word instructions |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store double word
  (ea)     <- (A)
  (ea) + 1 <- (D)

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

### STF

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `STF <addressing_mode> <disp>` |
| Category | Standard Floating Instructions |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store floating accumulator (TAD) to memory (ea)
Memory format:
- EA: Exponent
- EA+1, EA+2: Mantissa


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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store T register to memory location (EA): = T

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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store X register to memory location (EA): = X

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
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Store zero to memory location (EA): = 0

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

### SUB

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SUB <addressing_mode> <displacement>` |
| Category | Arithmetic and Logical |
| Privilege | user |
| Mask | `1111_1000_0000_0000` |

#### 📝 Description

Subtract from A register (C and Q may also be affected) A: = A - (EA)

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

### SWAP

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `SWAP <source> <destination>` |
| Category | Register Operations |
| Privilege | user |
| Mask | `1111_1111_1100_0000` |

#### 📝 Description

Register exchange
(sr): = (dr)
(dr): = (sr)


#### 📋 Format

```
SWAP <source> <destination>
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

--------------------------------------------------------------------------------

### TRA

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | ` TRA <disp>` |
| Category | Control Transfer |
| Privilege | user |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

Transfer control to the address specified by the effective address.
The effective address is calculated by adding the displacement to the program counter.
The program counter is set to the effective address.


#### 📋 Format

```
 TRA <disp>
```

#### Bit Layout

```
   │ 15  14  13  12  11  10  9   8 │ 7   6   5   4   3   2   1   0   │
   ├───────────────────────────────┼─────────────────────────────────┤
   │             opcode            │              disp               │
   └───────────────────────────────┴─────────────────────────────────┘

```

#### 🔧 Operands

| Name | Type | Bits | Description |
|:-----|:-----|:-----|:------------|
| `opcode` | opcode | 15-8 | The opcode determines what type of operation occurs |
| `disp` | displacement | 7-0 | 8-bit signed field gives the memory address displacement (2's complement notation giving a displacement range of -128 to 127 memory locations) |

--------------------------------------------------------------------------------

### TRR

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `TRR <internal_register>` |
| Category | Register Transfer |
| Privilege | system |
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

### WAIT

#### ⚡ Quick Reference

| Property | Value |
|:---------|:-------|
| Format | `WAIT <wait_number>` |
| Category | System Control Instructions |
| Privilege | user |
| Mask | `1111_1111_0000_0000` |

#### 📝 Description

When interrupt system off: halts the program and enters the operator’s communication. 
When interrupt system on: give up priority. If there are no interrupt requests on any level, the program on level zero is entered.


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

