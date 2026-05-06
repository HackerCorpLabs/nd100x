# DAP Debugger Update -- May 2026

To: nd100x emulator / libdap / BSD kernel team
From: Ronny (nd100x emulator)

---

## Summary

Major debugger improvements for split I/D (PTM=1) kernel debugging. All changes are backward compatible -- existing commands work exactly as before.

---

## 1. Pause command fixed

`debug_pause()` now actually stops the CPU. Previously returned "Unknown error" because the emulator never registered a pause callback. Fixed in both libdap and nd100x.

---

## 2. I-space / D-space memory access

When the kernel runs with PTM=1, the same virtual address maps to different physical pages for instruction fetch (I-space) vs data access (D-space). The debugger now lets you choose which space to read.

### Memory read/write

```
debug_read_memory(address="ispace:0xBA60", count=20)   # overlay code (I-space)
debug_read_memory(address="dspace:0xBA60", count=20)   # kernel data/BSS (D-space)
debug_read_memory(address="phys:0x10000", count=20)    # physical (bypass MMU)
debug_read_memory(address="0x1000", count=20)           # virtual (default)
```

Short forms: `I:`, `D:`, `P:`, `V:`

### Disassembly

Disassembly defaults to I-space (instructions are always in I-space):

```
debug_disassemble(address="0xBA60", count=5)           # I-space by default
debug_disassemble(address="dspace:0xBA60", count=5)    # read D-space as instructions (unusual but possible)
```

The disassembler was previously broken for overlay code with PTM=1 -- it read D-space garbage instead of actual instructions. This is now fixed.

---

## 3. @PIL suffix -- inspect memory from another level's view

Append `@N` (N=0-15) to any address to use a different PIL's page table:

```
debug_read_memory(address="0x1000@1")              # read as PIL 1 (user process) sees it
debug_disassemble(address="0x1000@1", count=10)    # disassemble PIL 1's code
debug_read_memory(address="ispace:0xBA60@0")       # overlay code, PIL 0's page table
debug_write_memory(address="dspace:0x100@1", data="00FF")  # write to PIL 1's D-space
```

Omit `@N` to use the current PIL (backward compatible). Physical addresses ignore @PIL since they bypass the MMU.

**Use case**: You hit a breakpoint in the kernel at PIL 14 (trap handler). You want to see what the user process (PIL 1) has at address 0x1000. Previously impossible without computing physical addresses manually. Now: `debug_read_memory(address="0x1000@1")`.

---

## 4. I-space / D-space watchpoints with PIL filtering

Watchpoints now support address-space and PIL filtering:

```
# Watch overlay code reads (I-space only)
debug_set_data_breakpoints(variables=["ispace:0xBA60"], access_type="read")

# Watch kernel data writes at PIL 0 only
debug_set_data_breakpoints(variables=["dspace:0xBA60@0"], access_type="write")

# Physical watchpoint, PIL 14 only
debug_set_data_breakpoints(variables=["phys:0x10000@14"], access_type="write")

# Backward compatible (any space, any PIL)
debug_set_data_breakpoints(variables=["0x1000"], access_type="write")
```

Previously, virtual watchpoints fired on any access to the address regardless of UseAPT or PIL. A watchpoint at 0xBA60 would fire for both overlay code fetches AND data reads, and at every PIL level. Now you can target exactly the access you care about.

---

## 5. Watchpoint performance optimization

Virtual watchpoints now use an 8KB bitmap (1 bit per 16-bit address) for O(1) address rejection:

| Scenario | Cost per memory access |
|----------|----------------------|
| No watchpoints set | 1 int compare |
| Watchpoints active, address not watched | + 1 byte load + 1 bit test |
| Watchpoints active, watched address hit | + PIL/space/type check |

Previously, watchpoints used a linear scan of up to 32 entries on every memory access. The bitmap eliminates this for >99.99% of addresses when watchpoints are active.

---

## Address encoding quick reference

All memory commands use: `[prefix:]address[@pil]`

| Prefix | Meaning | Short |
|--------|---------|-------|
| *(none)* | Virtual (current page table) | `V:` |
| `phys:` | Physical (bypass MMU) | `P:` |
| `ispace:` | Instruction page table (PT field) | `I:` |
| `dspace:` | Data page table (APT field) | `D:` |

| Suffix | Meaning |
|--------|---------|
| *(none)* | Current PIL |
| `@N` | PIL N's page table (0-15) |

**Watchpoint dataId format**: `PREFIX:OCTAL_ADDR[@PIL]`
Example: `"I:135140@0"` = I-space, octal address 135140, PIL 0 only

---

## Commits

**libdap**: `a5a425b` (docs), `19663b1` (@PIL + prefix parsing), `08420bc` (pause fix)
**nd100x**: `c628e9d` (watchpoints + PIL + bitmap), `ecf3de2` (pause callback)
