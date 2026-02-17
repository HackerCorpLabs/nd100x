# Critical Issues Found in C Debugging Support

## 🔴 CRITICAL FINDINGS

During analysis of the DAP integration for mixed-language support, **three critical issues** were identified that **break C source-level debugging**:

1. ❌ **Step Over ignores STABS** - Only checks MAP table → C stepping broken
2. ❌ **Step In is instruction-only** - No source-aware stepping → Poor C UX
3. ⚠️ **Stack tracking assumes JPL/EXIT** - May not work if C uses different calling convention

## Issue #1: Step Over Ignores STABS (CRITICAL)

### Problem

**File:** `src/debugger/debugger.c:460-464`

```c
// PROBLEM: Only checks MAP table!
if (symbol_tables.symbol_table_map && 
    ((ctx->granularity == DAP_STEP_GRANULARITY_LINE) || 
     (ctx->granularity == DAP_STEP_GRANULARITY_STATEMENT)))
{
    target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_map, current_pc);
    // ...
}
```

**Never checks:** `symbol_tables.symbol_table_stabs`

### Impact

⚠️ **CRITICAL** - C debugging is broken:
- C programs don't have MAP files
- Step Over always falls back to single instruction
- Users can't step through C code by source line

### Fix (2-3 hours)

```c
// FIXED VERSION:
// Try STABS first (for C programs)
if (symbol_tables.symbol_table_stabs && 
    ((ctx->granularity == DAP_STEP_GRANULARITY_LINE) || 
     (ctx->granularity == DAP_STEP_GRANULARITY_STATEMENT)))
{
    target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_stabs, current_pc);
}

// Fallback to MAP (for assembly)
if ((!target_pc || target_pc == current_pc) && symbol_tables.symbol_table_map) {
    target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_map, current_pc);
}
```

### Priority

🔴 **MUST FIX** before claiming C debugging support

## Issue #2: Step In is Instruction-Only (HIGH)

### Problem

**File:** `src/debugger/debugger.c:496-505`

```c
// Step in - step one instruction (F11)
if (step_type == STEP_IN)
{
    // PROBLEM: Always single instruction, no source awareness!
    breakpoint_manager_step_one();
    ensure_cpu_running();
    return 0;
}
```

**Never checks:**
- Function calls
- Source line boundaries
- Granularity setting

### Impact

⚠️ **HIGH** - Poor C debugging experience:
- Steps into compiler-generated prologue
- Doesn't respect source line boundaries
- Inconsistent with Step Over behavior

### Scenarios

**C calling C:**
```c
int result = add(5, 10);  // User presses F11 (Step In)
```
- **Expected:** Jump to first line of `add()` function
- **Actual:** Single instruction step (might be in prologue)

**C calling Assembly:**
```c
delay(100);  // User presses F11
```
- **Expected:** Jump to `delay` function entry in .s file
- **Actual:** Single instruction step (works but unclear)

### Fix (3-4 hours)

See `docs/STEPPING_ANALYSIS.md` for complete implementation.

### Priority

🟡 **SHOULD FIX** for good C debugging experience

## Issue #3: Stack Tracking Assumes JPL/EXIT (HIGH)

### Problem

**File:** `src/debugger/debugger.c:348-412`

```c
void debugger_build_stack_trace(uint16_t pc, uint16_t operand)
{
    // PROBLEM: Only recognizes JPL and EXIT instructions
    bool is_jpl = ((operand & 0xF800) == 0134000);
    bool is_exit = (operand == 0146142);
    
    if (is_jpl) {
        // Push stack frame
    }
    else if (is_exit) {
        // Pop stack frame
    }
}
```

**Doesn't recognize:**
- C function prologue/epilogue
- Standard PUSH/POP patterns
- Other calling conventions

### Impact

⚠️ **DEPENDS ON C COMPILER**:
- **If C compiler generates JPL/EXIT** → Works fine ✅
- **If C compiler uses different convention** → Stack traces broken ❌

### Investigation Required

**MUST determine what your C compiler generates:**

```bash
# Compile simple C program
cat > test.c << 'EOF'
int add(int a, int b) {
    return a + b;
}
int main() {
    return add(5, 10);
}
EOF

# Compile to assembly
gcc -S -o test.s test.c

# Look for calling convention
grep -E "jpl|exit|call|ret|push|pop" test.s

# Check function boundaries
cat test.s | grep -A5 "add:"
cat test.s | grep -A5 "main:"
```

### Three Possible Outcomes

**Scenario A:** C generates JPL/EXIT
- ✅ No changes needed
- Step Out works
- Stack traces work

**Scenario B:** C generates different instructions
- ❌ Must update `debugger_build_stack_trace()`
- Add detection for C prologue/epilogue
- Update Step Out logic

**Scenario C:** C doesn't use frame pointers
- ❌ Stack reconstruction difficult
- May need DWARF frame info (.eh_frame)
- May need CPU stack pointer tracking

### Priority

🟡 **INVESTIGATE FIRST** (2 hours), then decide on fix

## Issue #4: Variable Names Not Tracked (HIGH)

### Problem

**File:** `src/debugger/debugger.h:23-39`

```c
/// @brief Variable in the current stack frame
/// TODO: Not yet implemented and supported by the DAP
typedef struct {
    char *name;
    char *value;
    char *type;
} Variable;

typedef struct {
    int number_of_variables;  // ALWAYS 0 - never populated!
    Variable *variables;      // ALWAYS NULL - never allocated!
} LocalVariables;
```

**Never populated:**
- STABS has variable info (N_LSYM, N_PSYM, N_RSYM)
- Parser doesn't extract it
- Variables scope shows "dummy" only

### Impact

⚠️ **HIGH** - Can't inspect C local variables:
- Can only see CPU registers
- Can't see function parameters
- Can't see local variables
- Professional debuggers show all variables

### Fix (8-10 hours - part of STABS parser)

Must parse:
- `N_PSYM` (0xA0) - Parameters
- `N_LSYM` (0x80) - Local variables
- `N_RSYM` (0x40) - Register variables
- `N_LBRAC` (0xC0) - Block start (scope)
- `N_RBRAC` (0xE0) - Block end (scope)

See `docs/STABS_PARSER_IMPLEMENTATION.md` for details.

### Priority

🟡 **SHOULD FIX** with STABS parser enhancement

## Issue #5: C ↔ Assembly View Switching Untested (MEDIUM)

### Status

⚠️ **UNKNOWN** - Infrastructure exists but never tested

**Should work:**
- ✅ `supportsDisassembleRequest` is advertised
- ✅ `cmd_disassemble()` is implemented
- ✅ `instructionPointerReference` is set in stack frames

**Testing needed:**
```
1. Debug C program
2. Set breakpoint
3. When stopped, right-click → "Open Disassembly View"
4. Should show generated ND-100 assembly
5. Should synchronize with C source view
```

### VS Code Requirements

- VS Code 1.65+ (disassembly view added)
- Setting: `"debug.disassemblyView.showSourceCode": true`

### Priority

🟢 **TEST FIRST** (30 minutes) before implementing anything

## Critical Path Forward

### Phase 1: Investigation (2-4 hours) 🔴 URGENT

```
1. Investigate C calling convention (2 hours)
   ├─ Compile sample C program to assembly
   ├─ Check if uses JPL/EXIT
   ├─ Document findings
   └─ Determine if Issue #3 is real
   
2. Test disassembly view (30 minutes)
   └─ Verify view switching works
   
3. Decision point:
   ├─ If C uses JPL/EXIT → Skip to Phase 2
   └─ If C uses different convention → Must fix stack tracking first
```

### Phase 2: Quick Wins (4-6 hours) 🟡 HIGH PRIORITY

```
1. Fix Step Over multi-table (2-3 hours)
   └─ Change to check STABS then MAP
   
2. Fix Step In source-aware (3-4 hours)
   └─ Add function call detection
   └─ Add source line stepping
   
3. Test C debugging
   └─ Verify improvements work
```

### Phase 3: Complete STABS Support (11-15 hours) 🟡 MEDIUM PRIORITY

```
1. Enhance STABS parser (8-10 hours)
   └─ Parse N_SO, N_FUN, N_SLINE
   └─ Build line tables
   └─ Parse variable entries
   
2. Test mixed debugging (3-4 hours)
   └─ C programs
   └─ Mixed C/assembly
   └─ Variable inspection
```

### Phase 4: dap_debugger Client (6-7 hours) 🟢 LOW PRIORITY

```
1. Add command line options (2 hours)
2. Update help system (1 hour)
3. Test client (2 hours)
```

## Quick Fix Available

### Minimum Viable Fix (2 hours)

Just fix Step Over to check STABS:

```c
// Change line 461 from:
if (symbol_tables.symbol_table_map && ...)

// To:
if ((symbol_tables.symbol_table_stabs || symbol_tables.symbol_table_map) && ...)

// And line 464 from:
target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_map, current_pc);

// To:
if (symbol_tables.symbol_table_stabs) {
    target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_stabs, current_pc);
}
if ((!target_pc || target_pc == current_pc) && symbol_tables.symbol_table_map) {
    target_pc = symbols_get_next_line_address(symbol_tables.symbol_table_map, current_pc);
}
```

**Benefit:** C step-over will work (if STABS has line tables)

**Limitation:** Still depends on STABS parser having line tables

## Recommendations

### Immediate Action

1. **RUN INVESTIGATION** (2 hours)
   - Compile C test program
   - Check calling convention
   - Document findings

2. **APPLY QUICK FIX** (2 hours)
   - Fix Step Over multi-table
   - Test with existing symbol tables
   - Document limitations

3. **TEST THOROUGHLY** (2 hours)
   - Assembly debugging (should still work)
   - C debugging (improved but still limited)
   - Mixed debugging (better)

### Future Work

1. **Enhanced STABS parser** (11-15 hours)
   - Enables full C debugging
   - Enables variable inspection
   - Enables proper line breakpoints

2. **Fix Step In** (3-4 hours)
   - Source-aware stepping
   - Function call detection
   - Better C debugging UX

3. **Update dap_debugger client** (6-7 hours)
   - Command-line parity
   - Complete feature set
   - Alternative to VS Code

## Summary Table

| Issue | Severity | Impact | Fix Time | Depends On |
|-------|----------|--------|----------|------------|
| Step Over ignores STABS | 🔴 CRITICAL | C stepping broken | 2-3 hours | None |
| Step In instruction-only | 🟡 HIGH | Poor C UX | 3-4 hours | None |
| Stack tracking JPL/EXIT | 🟡 HIGH | May break Step Out | 4-6 hours | Investigation |
| Variable names not tracked | 🟡 HIGH | Can't see C variables | 8-10 hours | STABS parser |
| View switching untested | 🟢 LOW | Unknown if works | 30 min | None |

**Total Quick Fixes:** 5-7 hours  
**Total Complete Solution:** 17-23 hours  

## Conclusion

The C debugging support has **critical gaps** that must be addressed:

**Must Fix Now** (2-4 hours):
1. Investigate C calling convention
2. Fix Step Over multi-table lookup

**Should Fix Soon** (7-10 hours):
1. Fix Step In source-aware
2. Fix stack tracking (if needed)

**Can Fix Later** (11-15 hours):
1. Enhanced STABS parser
2. Variable tracking
3. dap_debugger client updates

**Current Status:**
- Assembly debugging: ✅ Production ready
- C debugging: ⚠️ Partially broken, needs fixes
- Mixed debugging: ⚠️ Limited, needs investigation

**After Quick Fixes:**
- Assembly debugging: ✅ Still production ready
- C debugging: 🟡 Basic but usable
- Mixed debugging: 🟡 Improved

**After Complete Fixes:**
- Assembly debugging: ✅ Excellent
- C debugging: ✅ Professional quality
- Mixed debugging: ✅ Full support

---

**Action Required:** Investigate C calling convention and apply quick fixes before broader release.

**See Also:**
- `docs/STEPPING_ANALYSIS.md` - Detailed step analysis
- `docs/STABS_PARSER_IMPLEMENTATION.md` - Complete parser guide
- `docs/DAP_DEBUGGER_CLIENT_UPDATES.md` - Client update guide

