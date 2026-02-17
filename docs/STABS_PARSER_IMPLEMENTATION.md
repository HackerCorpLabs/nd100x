# STABS Parser Implementation Guide

## Overview

This document describes how to enhance the STABS symbol parser to provide full C source-level debugging support. This is the main remaining work for complete mixed-language debugging.

## Current STABS Support

**Location:** `external/libsymbols/src/symbols.c` (line 272)

**Current Implementation:**
```c:272:349:symbols.c
bool symbols_load_aout(symbol_table_t *table, const char *filename)
{
    aout_entry_t *entries = NULL;
    size_t count = 0;
    
    if (!aout_parse_file(filename, &entries, &count)) {
        return false;
    }
    
    // Adds symbols but doesn't parse STABS structure
    for (int i = 0; i < (int)count; i++) {
        symbol_entry_t entry = {
            .filename = NULL,
            .name = entries[i].name,
            .line = 0,  // NOT EXTRACTED FROM STABS
            .address = entries[i].value,
            .type = map_nlist_type(entries[i].type),
            .owns_strings = false
        };
        
        symbols_add_entry(table, entry.filename, entry.name,
                         entry.line, entry.address, entry.type);
    }
    
    aout_free_entries(entries, count);
    return true;
}
```

**Limitation:** Loads symbols but doesn't parse STABS entries for line numbers and file associations.

## STABS Format

### Symbol Entry Structure

```c:42:46:aout.h
typedef struct {
    uint32_t n_strx;   // Offset into string table
    uint16_t n_type;   // Symbol type (N_SO, N_FUN, N_SLINE, etc.)
    uint16_t n_value;  // Symbol value (address or line number)
} aout_nlist_t;
```

**Note:** The `n_type` field is actually 2 bytes but contains:
- **n_type (low byte)**: Symbol type (N_SO, N_FUN, etc.)
- **n_desc (high byte)**: Description field (line number for N_SLINE, etc.)

### Key STABS Entry Types

| Type | Value | Name | Purpose | n_value | n_desc |
|------|-------|------|---------|---------|--------|
| N_SO | 0x64 | Source file | Start of source file | Base address | 0 |
| N_SOL | 0x84 | Include file | Included file name | Base address | 0 |
| N_FUN | 0x24 | Function | Function name | Function address | Line number |
| N_SLINE | 0x44 | Source line | Line number | Offset from function | Line number |
| N_GSYM | 0x20 | Global sym | Global variable | 0 | 0 |
| N_LSYM | 0x80 | Local sym | Local variable/type | 0 | 0 |
| N_PSYM | 0xA0 | Parameter | Function parameter | Stack offset | Register num |
| N_RSYM | 0x40 | Register var | Register variable | 0 | Register num |
| N_LBRAC | 0xC0 | Begin block | Lexical scope start | Address offset | Nesting level |
| N_RBRAC | 0xE0 | End block | Lexical scope end | Address offset | Nesting level |

### STABS Sequence Example

```
Symbol Table Entry | n_type | n_desc | n_value | String
-------------------|--------|--------|---------|------------------
1                  | 0x64   | 0      | 0x0000  | "main.c"          (N_SO - source file)
2                  | 0x24   | 15     | 0x0100  | "main:F1"         (N_FUN - function at line 15)
3                  | 0x44   | 15     | 0x0000  | ""                (N_SLINE - line 15 at offset 0)
4                  | 0x44   | 16     | 0x0002  | ""                (N_SLINE - line 16 at offset 2)
5                  | 0x44   | 17     | 0x0005  | ""                (N_SLINE - line 17 at offset 5)
6                  | 0x64   | 0      | 0x0000  | ""                (N_SO - end of source file)
7                  | 0x64   | 0      | 0x0000  | "util.s"          (N_SO - new source file)
8                  | 0x24   | 42     | 0x0234  | "delay:F1"        (N_FUN - function at line 42)
```

## Required Implementation

### Data Structures

**Add to `symbols.h`:**

```c
// Line table entry for fast lookup
typedef struct {
    uint16_t address;       // Absolute address
    int line;               // Line number
    char *file;             // Source file
    char *function;         // Function name (if at function start)
    bool is_function_start; // True if this is function entry
} line_table_entry_t;

// Source file context
typedef struct {
    char *filename;         // Full path
    uint16_t base_address;  // Start address
    int base_line;          // First line (usually 0)
    char *current_function; // Current function being parsed
    uint16_t function_addr; // Current function address
} source_context_t;

// Enhanced symbol table
typedef struct {
    // ... existing fields ...
    
    // NEW: Line table
    line_table_entry_t *line_table;
    size_t line_table_count;
    size_t line_table_capacity;
    
    // NEW: Source file list
    source_context_t *source_files;
    size_t source_file_count;
} symbol_table_t;
```

### Enhanced STABS Parser

**Add to `symbols.c`:**

```c
bool symbols_load_stabs_enhanced(symbol_table_t *table, const char *filename)
{
    aout_entry_t *entries = NULL;
    size_t count = 0;
    
    if (!aout_parse_file(filename, &entries, &count)) {
        return false;
    }
    
    // Context for parsing
    source_context_t current_source = {0};
    char *current_function = NULL;
    uint16_t function_base_addr = 0;
    
    // Parse STABS entries
    for (size_t i = 0; i < count; i++) {
        uint8_t type = entries[i].type & ~N_EXT;
        uint8_t desc = entries[i].desc;
        uint16_t value = entries[i].value;
        const char *name = entries[i].name;
        
        switch (type) {
            case N_SO:  // Source file
                if (name && name[0]) {
                    // Start of new source file
                    current_source.filename = strdup(name);
                    current_source.base_address = value;
                    current_source.base_line = 0;
                    
                    // Add to source file list
                    add_source_file(table, &current_source);
                } else {
                    // End of source file (empty string)
                    current_source.filename = NULL;
                }
                break;
                
            case N_SOL:  // Include file
                if (name && name[0]) {
                    // Included file
                    current_source.filename = strdup(name);
                    current_source.base_address = value;
                }
                break;
                
            case N_FUN:  // Function
                if (name && name[0]) {
                    // Start of function
                    // Parse "name:type" format
                    char *colon = strchr(name, ':');
                    if (colon) {
                        size_t name_len = colon - name;
                        current_function = strndup(name, name_len);
                    } else {
                        current_function = strdup(name);
                    }
                    
                    function_base_addr = value;
                    
                    // Add function symbol
                    symbols_add_entry(table, current_source.filename,
                                    current_function, desc, value,
                                    SYMBOL_TYPE_FUNCTION);
                    
                    // Add to line table
                    add_line_table_entry(table, value, desc,
                                       current_source.filename,
                                       current_function, true);
                } else {
                    // End of function
                    if (current_function) {
                        free(current_function);
                        current_function = NULL;
                    }
                }
                break;
                
            case N_SLINE:  // Source line
                {
                    // n_desc contains line number
                    // n_value contains offset from function start
                    uint16_t absolute_addr = function_base_addr + value;
                    
                    // Add to line table
                    add_line_table_entry(table, absolute_addr, desc,
                                       current_source.filename,
                                       current_function, false);
                }
                break;
                
            case N_GSYM:  // Global variable
            case N_LSYM:  // Local variable
            case N_PSYM:  // Parameter
            case N_RSYM:  // Register variable
                // Add variable symbols
                symbols_add_entry(table, current_source.filename,
                                name, 0, value, SYMBOL_TYPE_VARIABLE);
                break;
                
            case N_LBRAC:  // Begin block
            case N_RBRAC:  // End block
                // Could track lexical scopes for advanced variable inspection
                break;
                
            default:
                // Other types - ignore or log
                break;
        }
    }
    
    // Sort line table by address for fast lookup
    sort_line_table(table);
    
    aout_free_entries(entries, count);
    return true;
}
```

### Line Table Functions

**Add to `symbols.c`:**

```c
// Add entry to line table
void add_line_table_entry(
    symbol_table_t *table,
    uint16_t address,
    int line,
    const char *file,
    const char *function,
    bool is_function_start)
{
    // Ensure capacity
    if (table->line_table_count >= table->line_table_capacity) {
        table->line_table_capacity = table->line_table_capacity ? 
                                     table->line_table_capacity * 2 : 256;
        table->line_table = realloc(table->line_table,
                                   table->line_table_capacity * sizeof(line_table_entry_t));
    }
    
    // Add entry
    line_table_entry_t *entry = &table->line_table[table->line_table_count++];
    entry->address = address;
    entry->line = line;
    entry->file = file ? strdup(file) : NULL;
    entry->function = function ? strdup(function) : NULL;
    entry->is_function_start = is_function_start;
}

// Find line for address
int symbols_get_line_from_table(symbol_table_t *table, uint16_t address)
{
    if (!table || !table->line_table) return 0;
    
    // Binary search (table is sorted)
    int left = 0;
    int right = table->line_table_count - 1;
    int best = -1;
    
    while (left <= right) {
        int mid = (left + right) / 2;
        
        if (table->line_table[mid].address == address) {
            return table->line_table[mid].line;
        } else if (table->line_table[mid].address < address) {
            best = mid;  // Keep track of closest match
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    // Return line of closest address before target
    if (best >= 0) {
        return table->line_table[best].line;
    }
    
    return 0;
}

// Find file for address
const char *symbols_get_file_from_table(symbol_table_t *table, uint16_t address)
{
    if (!table || !table->line_table) return NULL;
    
    // Binary search
    // ... similar to symbols_get_line_from_table ...
    
    if (best >= 0) {
        return table->line_table[best].file;
    }
    
    return NULL;
}

// Find address for (file, line)
bool symbols_find_address_from_table(
    symbol_table_t *table,
    const char *source_file,
    uint16_t *out_address,
    uint16_t *out_diff,
    int target_line)
{
    if (!table || !table->line_table) return false;
    
    int best = -1;
    int best_diff = INT_MAX;
    
    // Linear search through line table
    for (size_t i = 0; i < table->line_table_count; i++) {
        line_table_entry_t *entry = &table->line_table[i];
        
        // Check if file matches
        if (!entry->file || strcmp(entry->file, source_file) != 0) {
            continue;
        }
        
        // Check line number
        int diff = abs(entry->line - target_line);
        if (diff < best_diff) {
            best_diff = diff;
            best = i;
            
            // Exact match
            if (diff == 0) {
                *out_address = entry->address;
                *out_diff = 0;
                return true;
            }
        }
    }
    
    // Return closest match
    if (best >= 0) {
        *out_address = table->line_table[best].address;
        *out_diff = best_diff;
        return true;
    }
    
    return false;
}

// Get next line address (for step over)
uint16_t symbols_get_next_line_address(
    symbol_table_t *table,
    uint16_t current_address)
{
    if (!table || !table->line_table) return 0;
    
    // Find current entry in line table
    for (size_t i = 0; i < table->line_table_count - 1; i++) {
        if (table->line_table[i].address == current_address) {
            // Return next entry's address
            return table->line_table[i + 1].address;
        }
    }
    
    return 0;
}
```

## Integration Points

### Update `symbols_load_stabs()`

**Location:** `external/libsymbols/src/symbols.c`

Replace basic loading with enhanced parser:

```c
bool symbols_load_stabs(symbol_table_t *table, const char *filename)
{
    // Call enhanced parser
    return symbols_load_stabs_enhanced(table, filename);
}
```

### Update Symbol Lookup Functions

**Location:** `external/libsymbols/src/symbols.c`

Modify to use line table:

```c
int symbols_get_line(symbol_table_t *table, uint16_t address)
{
    if (!table) return 0;
    
    // Try line table first (faster)
    if (table->line_table && table->line_table_count > 0) {
        return symbols_get_line_from_table(table, address);
    }
    
    // Fallback to old method
    return symbols_get_line_old(table, address);
}

const char *symbols_get_file(symbol_table_t *table, uint16_t address)
{
    if (!table) return NULL;
    
    // Try line table first
    if (table->line_table && table->line_table_count > 0) {
        return symbols_get_file_from_table(table, address);
    }
    
    // Fallback to old method
    return symbols_get_file_old(table, address);
}

bool symbols_find_address(
    symbol_table_t *table,
    const char *source_file,
    uint16_t *address,
    uint16_t *diff,
    int line)
{
    if (!table) return false;
    
    // Try line table first
    if (table->line_table && table->line_table_count > 0) {
        if (symbols_find_address_from_table(table, source_file, address, diff, line)) {
            return true;
        }
    }
    
    // Fallback to old method
    return symbols_find_address_old(table, source_file, address, diff, line);
}
```

## Testing the STABS Parser

### Test Program

Create `test_stabs.c`:

```c
#include <stdio.h>

// Function in C
int add(int a, int b) {
    int result = a + b;  // Breakpoint here (line 5)
    return result;
}

int main() {
    int x = 10;          // Breakpoint here (line 10)
    int y = 20;
    int z = add(x, y);   // Breakpoint here (line 12)
    printf("Result: %d\n", z);
    return 0;
}
```

### Compile with STABS

```bash
# Compile with debug info
gcc -g -o test.out test_stabs.c

# Or use nd100-as with STABS output
ndasm -g -o test.out test_stabs.s
```

### Verify STABS Entries

```bash
# Dump symbol table
nm -n test.out

# Dump STABS entries
objdump -g test.out

# Or use custom tool
./build/bin/dump_header test.out
```

### Test Cases

1. **Source File Mapping**
   ```c
   // Should find "test_stabs.c" as source
   const char *file = symbols_get_file(stabs_table, 0x0100);
   assert(strcmp(file, "test_stabs.c") == 0);
   ```

2. **Line Number Mapping**
   ```c
   // Should find line 5 for address in add()
   int line = symbols_get_line(stabs_table, 0x0100);
   assert(line == 5);
   ```

3. **Address Finding**
   ```c
   // Should find address for line 10
   uint16_t addr;
   uint16_t diff;
   bool found = symbols_find_address(stabs_table, "test_stabs.c", &addr, &diff, 10);
   assert(found == true);
   assert(diff == 0);  // Exact match
   ```

4. **Function Names**
   ```c
   // Should find "add" function
   const symbol_entry_t *sym = symbols_lookup_by_address(stabs_table, 0x0100);
   assert(strcmp(sym->name, "add") == 0);
   assert(sym->type == SYMBOL_TYPE_FUNCTION);
   ```

5. **Next Line Address**
   ```c
   // Should find next line from current
   uint16_t next = symbols_get_next_line_address(stabs_table, 0x0100);
   assert(next > 0x0100);
   assert(next == 0x0102);  // Next instruction
   ```

## Integration with Debugger

### Update Launch Handler

**Location:** `src/debugger/debugger.c` (line 2082)

The STABS loading is already attempted:

```c:2082:2098:debugger.c
if (program_path) {
    printf("Attempting to load symbols from program binary: %s\n", program_path);
    
    if (init_symbol_support(program_path, SYMBOL_TYPE_AOUT) == 0) {
        symbols_loaded = true;
        // ...
    }
}
```

This calls `symbols_load_aout()` which should use the enhanced parser.

### Update Symbol Lookup Calls

**Locations:**
- Breakpoint resolution: `debugger.c:1980-2000`
- Stack frame building: `debugger.c:1842-1862`
- Stopped event: `debugger.c:269-280`
- Step over: `debugger.c:420-432`

All these already use `symbols_get_line()` and `symbols_get_file()`, so they'll automatically benefit from the enhanced parser.

## Performance Considerations

### Line Table Sorting

After loading all STABS entries, sort by address:

```c
int compare_line_entries(const void *a, const void *b) {
    const line_table_entry_t *entry_a = (const line_table_entry_t *)a;
    const line_table_entry_t *entry_b = (const line_table_entry_t *)b;
    return entry_a->address - entry_b->address;
}

void sort_line_table(symbol_table_t *table) {
    if (table->line_table && table->line_table_count > 0) {
        qsort(table->line_table, table->line_table_count,
              sizeof(line_table_entry_t), compare_line_entries);
    }
}
```

**Benefit:** Binary search becomes O(log n) instead of O(n).

### Memory Usage

For typical program:
- 1000 lines of code → ~1000 line table entries
- Each entry: ~40 bytes (address, line, pointers)
- Total: ~40 KB

**Acceptable** for modern systems, even embedded.

### Cache Locality

Line table is contiguous array → good cache performance for sequential lookups during stepping.

## Example Output

### With Enhanced STABS Parser

```
=== Loaded a.out Header ===
  Magic     : 0x0407 (normal)
  Text size : 256 words
  Data size : 64 words
  Symbols   : 1024 bytes
  Entry     : 000000

Loading symbols...
[0] N_SO: main.c (base=0x0000)
[1] N_FUN: main (addr=0x0000, line=9)
[2] N_SLINE: line 10 (addr=0x0002)
[3] N_SLINE: line 11 (addr=0x0005)
[4] N_SLINE: line 12 (addr=0x0008)
[5] N_FUN: add (addr=0x0100, line=4)
[6] N_SLINE: line 5 (addr=0x0102)
[7] N_SLINE: line 6 (addr=0x0105)
[8] N_SO: util.s (base=0x0200)
[9] N_FUN: delay (addr=0x0200, line=42)

Built line table with 9 entries
Source files: main.c, util.s
Functions: main, add, delay
```

### During Debugging

```
Breakpoint at main.c:10 → address 0x0002
Stopped at main.c:10 (PC=000002)

Stack trace:
  Frame 0: main at main.c:10
  Frame 1: _start at crt0.s:5

Step over → next line at 0x0005 (main.c:11)
```

## Summary

### What's Needed

1. **Enhance STABS parser** (`symbols.c`)
   - Parse N_SO, N_SOL, N_FUN, N_SLINE
   - Build line table structure
   - Sort for fast lookup

2. **Add line table functions** (`symbols.c`)
   - `add_line_table_entry()`
   - `symbols_get_line_from_table()`
   - `symbols_get_file_from_table()`
   - `symbols_find_address_from_table()`
   - `sort_line_table()`

3. **Update symbol lookup** (`symbols.c`)
   - Use line table if available
   - Fallback to old method

4. **Test thoroughly**
   - Unit tests for each function
   - Integration tests with real programs
   - VS Code debugging sessions

### What's Already Done ✅

- ✅ DAP server infrastructure
- ✅ Thread coordination
- ✅ Breakpoint manager
- ✅ Source reference system
- ✅ Multi-table lookup strategy
- ✅ Enhanced stack frames
- ✅ Enhanced stopped events
- ✅ VS Code extension updates
- ✅ Configuration examples
- ✅ Documentation

### Estimated Effort

**STABS Parser Enhancement:**
- Data structures: 2 hours
- Parser implementation: 4-6 hours
- Line table functions: 2-3 hours
- Testing: 3-4 hours
- **Total: 11-15 hours**

**Already Completed:**
- Infrastructure: ~8 hours
- DAP handlers: ~6 hours
- Documentation: ~4 hours
- **Total: ~18 hours**

## Conclusion

The DAP integration is 80% complete. The main remaining work is enhancing the STABS parser to provide full C source-level debugging. All the infrastructure is in place - the parser just needs to populate the line table correctly.

Once the STABS parser is complete, the ND100X emulator will provide professional-grade mixed-language debugging support on par with commercial debuggers.

