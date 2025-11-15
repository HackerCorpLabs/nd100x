# ND-100 Instruction Analysis Functions

## Overview

This document describes the instruction analysis helper functions implemented in the debugger (`src/debugger/debugger.c`) for detecting and analyzing ND-100 procedure calls, returns, and calling conventions.

## ND-100 Calling Conventions

The ND-100 architecture supports two distinct calling conventions:

### 1. JPL/EXIT Convention (Simple Calling)

**Used by:** Assembly code, simple subroutines

**Call sequence:**
```assembly
JPL subroutine    ; Jump and Link - saves return address to L register
```

**Return sequence:**
```assembly
EXIT              ; Copy L to P - returns via L register
```

**Characteristics:**
- Return address stored in **L register**
- Simple, fast calling convention
- No automatic stack frame management
- JPL opcode: `0134xxx` (octal) where xxx = addressing mode + displacement

### 2. ENTR/LEAVE Convention (Structured Stack Frames)

**Used by:** C compiler, functions requiring stack frames

**Call sequence:**
```assembly
JPL function      ; Jump to function entry point
ENTR              ; (Inside function) Set up stack frame
<stack demand>    ; Word specifying stack space needed
<error return>    ; Error return address
<normal return>   ; Normal return address
```

**Return sequence:**
```assembly
LEAVE             ; Restore stack frame, return via LINK
; or
ELEAV             ; Error return from stack frame
```

**Characteristics:**
- Return address stored in **Memory[B-128]** (LINK)
- Previous frame pointer in **Memory[B-127]** (PREVB)
- Automatic stack overflow checking
- Stack frame layout managed by hardware

**Stack Frame Layout:**
```
Offset from B    Name      Description
--------------   -------   -----------
B - 128 (0200)   LINK      Return address
B - 127 (0177)   PREVB     Previous B register value
B - 126 (0176)   STP       Stack top pointer
B - 125 (0175)   SMAX      Stack maximum
B - 123 (0173)   ERRCODE   Error code (used by ELEAV)
B + 0 onwards    Locals    Local variables
```

## Function Reference

### `is_procedure_call()`

**Purpose:** Detect procedure call instructions

**Signature:**
```c
static bool is_procedure_call(uint16_t operand)
```

**Detects:**
- **JPL** (Jump and Link) - Opcode `0134xxx` with mask `0xF800`

**Does NOT detect:**
- ENTR - This is a stack frame setup instruction, not a call

**Returns:** `true` if the operand is a procedure call instruction

**Implementation:**
```c
if ((operand & 0xF800) == 0134000) {
    return true;  // JPL instruction
}
return false;
```

**Usage:** Called by debugger stepping logic to identify when to set breakpoints at call targets for "step into" functionality.

---

### `is_procedure_return()`

**Purpose:** Detect procedure return instructions that return via L register

**Signature:**
```c
static bool is_procedure_return(uint16_t operand)
```

**Detects:**
- **EXIT** - Opcode `0146142` (compound mnemonic for `COPY SL DP`)

**Does NOT detect:**
- LEAVE (`0140136`) - Returns via stack LINK, not L register
- ELEAV (`0140137`) - Returns via stack LINK, not L register

**Returns:** `true` if the operand is an EXIT instruction

**Implementation:**
```c
if (operand == 0146142) {
    return true;  // EXIT instruction (P = L)
}
return false;
```

**Note:** This function specifically detects returns that use the L register (paired with JPL). For stack frame returns, use `is_c_function_epilogue()`.

**Usage:** Used by stack trace building to detect when a function returns via the JPL/EXIT convention.

---

### `get_jpl_target_address()`

**Purpose:** Calculate the target address of a JPL instruction

**Signature:**
```c
static uint16_t get_jpl_target_address(uint16_t pc, uint16_t operand)
```

**Parameters:**
- `pc` - Current program counter (already incremented)
- `operand` - The JPL instruction word

**Returns:** Target address of the JPL instruction

**Instruction Format:**
```
Bits 15-11: Opcode (01340)
Bits 10-8:  Addressing mode (X, I, B flags)
  Bit 10 (X): Index by X register
  Bit 9  (I): Indirect addressing
  Bit 8  (B): Base-relative (vs P-relative)
Bits 7-0:   8-bit signed displacement
```

**Implementation (Simplified):**
```c
// Extract 8-bit displacement and sign-extend
int8_t disp = (int8_t)(operand & 0xFF);
uint16_t displacement = (uint16_t)(int16_t)disp;

// Extract addressing mode (bits 10-8)
uint8_t mode = (operand >> 8) & 0x07;

// Simplified: assume P-relative addressing (mode 0)
uint16_t ea = pc + displacement;

return ea;
```

**Limitations:**
- Current implementation assumes **P-relative addressing** (mode 0)
- Full implementation would require access to:
  - `gB` register (for B-relative modes)
  - `gX` register (for indexed modes)
  - Memory access (for indirect modes)

**Reference:** See `New_GetEffectiveAddr()` in `src/cpu/cpu.c` for complete addressing mode logic.

**Usage:** Used by debugger stepping logic to calculate where a JPL instruction will jump, enabling "step into" to set breakpoints at the target address.

---

### `is_c_function_prologue()`

**Purpose:** Detect C function entry/prologue pattern

**Signature:**
```c
static bool is_c_function_prologue(uint16_t pc)
```

**Parameters:**
- `pc` - Program counter to check

**Detects:**
- **ENTR** instruction at the given PC - Opcode `0140135`

**Returns:** `true` if ENTR instruction is found at PC

**Implementation:**
```c
uint16_t operand = ReadVirtualMemory(pc, false);
if (operand == 0140135) {
    return true;  // ENTR instruction
}
return false;
```

**ENTR Behavior:**
1. Reads stack demand from `Memory[PC]`
2. Checks for stack overflow
3. Saves old B to PREVB: `Memory[new_B - 127] = old_B`
4. Saves return address to LINK: `Memory[new_B - 128] = L + 1`
5. Advances stack: `B = STP + 128`
6. Updates STP: `Memory[B - 126] = B + demand - 122`
7. Skips ahead: `PC += 2` (skips stack demand and error return words)

**Usage:** Used by stack trace building to detect when entering a structured stack frame, enabling proper frame unwinding.

---

### `is_c_function_epilogue()`

**Purpose:** Detect C function exit/epilogue pattern (stack frame returns)

**Signature:**
```c
static bool is_c_function_epilogue(uint16_t pc)
```

**Parameters:**
- `pc` - Program counter to check

**Detects:**
- **LEAVE** - Opcode `0140136` - Normal stack frame return
- **ELEAV** - Opcode `0140137` - Error stack frame return

**Does NOT detect:**
- EXIT (`0146142`) - Returns via L register, not stack frame

**Returns:** `true` if LEAVE or ELEAV instruction is found at PC

**Implementation:**
```c
uint16_t operand = ReadVirtualMemory(pc, false);
if (operand == 0140136) {
    return true;  // LEAVE instruction
}
if (operand == 0140137) {
    return true;  // ELEAV instruction
}
return false;
```

**LEAVE Behavior:**
1. Restore PC from LINK: `P = Memory[B - 128]`
2. Restore B from PREVB: `B = Memory[B - 127]`

**ELEAV Behavior:**
1. Calculate error return: `tmp = Memory[B - 128] - 1`
2. Update LINK: `Memory[tmp] = B - 128`
3. Save error code: `Memory[B - 123] = A`
4. Restore PC: `P = Memory[B - 128]`
5. Restore B: `B = Memory[B - 127]`

**Usage:** Used by stack trace building to detect when exiting a structured stack frame, distinguishing between stack-based returns (LEAVE/ELEAV) and register-based returns (EXIT).

---

## Instruction Opcodes Reference

### Procedure Call/Return Instructions

| Instruction | Opcode (Octal) | Opcode (Hex) | Mask        | Description                          |
|-------------|----------------|--------------|-------------|--------------------------------------|
| JPL         | `0134xxx`      | `0x2Cxx`     | `0xF800`    | Jump and Link - saves PC to L        |
| EXIT        | `0146142`      | `0x6462`     | exact match | Return via L (COPY SL DP)            |
| ENTR        | `0140135`      | `0x605D`     | exact match | Enter stack frame                    |
| LEAVE       | `0140136`      | `0x605E`     | exact match | Leave stack frame (normal return)    |
| ELEAV       | `0140137`      | `0x605F`     | exact match | Leave stack frame (error return)     |

### Addressing Modes (for JPL and other memory reference instructions)

| Mode | Bits 10-8 | Description                          | Calculation              |
|------|-----------|--------------------------------------|--------------------------|
| 0    | 000       | P-relative                           | EA = P + disp            |
| 1    | 001       | B-relative                           | EA = B + disp            |
| 2    | 010       | Indirect P-relative                  | EA = Mem[P + disp]       |
| 3    | 011       | Indirect B-relative                  | EA = Mem[B + disp]       |
| 4    | 100       | X-indexed                            | EA = X + disp            |
| 5    | 101       | B-relative indexed                   | EA = B + X + disp        |
| 6    | 110       | Indirect P-relative indexed          | EA = Mem[P + disp] + X   |
| 7    | 111       | Indirect B-relative indexed          | EA = Mem[B + disp] + X   |

Note: `disp` is an 8-bit signed displacement (bits 7-0 of the instruction word)

---

## Code Examples

### Example 1: Detecting Calls in Assembly Code

```assembly
; Simple subroutine using JPL/EXIT
main:
    JPL putch      ; is_procedure_call() returns true
    OPCOM          ; Stop

putch:
    COPY SA DD     ; Save A
    IOX 0306       ; Get status
    BSKP ONE 030 DA
    JMP .-2        ; Wait loop
    COPY SD DA     ; Restore A
    IOX 0305       ; Write character
    EXIT           ; is_procedure_return() returns true
```

### Example 2: C Function with Stack Frame

```c
// C source
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

```assembly
; Compiled output (conceptual)
factorial:
    JPL .+1           ; Call to function (is_procedure_call() = true)
    ENTR              ; is_c_function_prologue() = true
    .WORD 10          ; Stack demand
    .WORD error_ret   ; Error return
    .WORD normal_ret  ; Normal return
normal_ret:
    ; Function body...
    LEAVE             ; is_c_function_epilogue() = true
```

---

## Debugger Usage

These functions are used by the debugger in several contexts:

### Stack Trace Building (`debugger_build_stack_trace`)

Located in `src/debugger/debugger.c` around line 485:

```c
void debugger_build_stack_trace(uint16_t pc, uint16_t operand)
{
    bool is_jpl = ((operand & 0xF800) == 0134000);
    bool is_exit = (operand == 0146142);

    // Check for C calling convention
    bool is_c_call = is_c_function_prologue(pc);
    bool is_c_return = is_c_function_epilogue(pc);

    // Handle function calls (JPL or C calling convention)
    if (is_jpl || is_c_call) {
        // Push new stack frame
    }

    // Handle returns
    if (is_exit || is_c_return) {
        // Pop stack frame
    }
}
```

### Step Into Logic (`step_cpu`)

Located in `src/debugger/debugger.c` around line 685:

```c
// Check if this is a procedure call
if (is_procedure_call(current_operand)) {
    // Calculate where the call will jump to
    uint16_t call_target = get_jpl_target_address(current_pc, current_operand);

    // Set temporary breakpoint at function entry for step-into
    breakpoint_manager_add(call_target, BP_TYPE_TEMPORARY, NULL, NULL, NULL);
}
```

---

## Implementation Files

- **Main implementation:** `/home/ronny/repos/nd100x/src/debugger/debugger.c` (lines 318-451)
- **CPU instruction implementation:** `/home/ronny/repos/nd100x/src/cpu/cpu_instr.c`
  - JPL: lines 451-464
  - ENTR: lines 1570-1588
  - LEAVE: lines 1590-1596
  - ELEAV: lines 1598-1608
- **Disassembly:** `/home/ronny/repos/nd100x/src/cpu/cpu_disasm.c`
- **Addressing modes:** `/home/ronny/repos/nd100x/src/cpu/cpu.c` (New_GetEffectiveAddr, lines 132-179)

---

## Future Improvements

1. **Full addressing mode support in `get_jpl_target_address()`**
   - Requires access to CPU registers (gB, gX)
   - Requires memory read capability for indirect modes
   - Would enable accurate target calculation for all JPL variants

2. **Additional call patterns**
   - Investigate if there are other call patterns used by different compilers
   - Consider detection of tail calls or jump tables

3. **Performance optimization**
   - Cache instruction decoding results
   - Avoid repeated memory reads for same PC

---

## References

- **ND-100 CPU Documentation:** `/home/ronny/repos/nd100x/docs/cpu_documentation.md`
- **Instruction Set Reference:** Lines 2846-2897 (EXIT), 4899-4945 (LEAVE)
- **Assembly Examples:** `/home/ronny/repos/nd100x/asm/samples/hello-read.s`
- **ND-100 Assembler:** [nd100-as](https://github.com/ragge0/norsk_data/tree/main/nd100-as)

---

## Revision History

| Date       | Version | Author | Description                                    |
|------------|---------|--------|------------------------------------------------|
| 2025-11-15 | 1.0     | -      | Initial documentation of instruction analysis  |
|            |         |        | functions with corrected implementation        |
