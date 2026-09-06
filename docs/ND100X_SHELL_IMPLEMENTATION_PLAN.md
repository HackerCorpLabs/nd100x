# nd100x Interactive Shell Implementation Plan

**Date:** 2026-07-23  
**Scope:** Add `--monitor`/`--shell` mode to nd100x for loading and running BPUN/PROG compiler files  
**Baseline:** nd500x shell implementation (770 lines, 13 commands)  
**Target:** Simplified shell for compiler file loading with auto-name-based file type detection

---

## Executive Summary

nd500x has a full SINTRAN-flavoured shell with 13 commands, file operations, user management, and domain execution. nd100x needs a **simplified version** focused on:

1. **Loading BPUN and PROG files** based on file naming conventions
2. **Running loaded programs** interactively 
3. **File listing and basic file operations** (for ND-100 context)
4. **Terminal type configuration** (if needed for PROG compatibility)

The implementation should reuse nd500x's patterns but scale down to ND-100's simpler architecture (no domain system, no MMU, simpler memory model).

---

## Current State Analysis

### nd500x Shell (Reference Implementation)

**File:** `<nd500x>/src/frontend/nd500x/nd500x_shell.c` (770 lines)

**Architecture:**
- Entry point: `nd500x_shell.c` + `nd500x_shell.h`
- Dispatcher in main nd500x.c detects `--monitor`/`--shell` flag
- Integrates with ndmonlib for file table, terminal state, config
- Uses readline for input (if available)
- Supports telnet transport (separate module `nd500x_telnet.c`)

**Key Dependencies:**
- `ndmonlib` - file table, terminal I/O, path resolution, config
- `readline` (optional) - command history and completion
- `nd500x_dom.c` - domain loading and execution

**Commands (13 total):**
```
HELP              - show command list
LOGIN <user>      - authenticate (simplified from ESC flow)
LOGOUT            - end session
EXIT              - leave emulator
LIST-FILES [*]    - list files with pattern filter (:TYPE, NAME*, etc)
SET-TERMINAL-TYPE - set terminal type from VTM file
GET-TERMINAL-TYPE - show current terminal type
RECOVER-DOMAIN    - load and run a program file
CREATE-FILE       - create empty file
DELETE-FILE       - delete file
RENAME-FILE       - rename file
COPY-FILE         - copy file
LIST-USERS        - list user directories
CREATE-USER       - create user directory
```

**Features:**
- `@` prompt (SINTRAN authentic)
- Command abbreviation (e.g., `LI-FI` for `LIST-FILES`)
- SINTRAN-style file specs (NAME:TYPE)
- Host filesystem integration (dot for colon conversion)
- Error messages from ND manuals
- Script file support (`--script` flag)
- Telnet server support (`--telnet <port>`)
- Config file support (`nd500x.ini`)

---

## nd100x Requirements

### Use Cases

1. **BPUN file loading and execution**
   - User runs: `ND100-loader --monitor`
   - Loads a BPUN (Batch Punched) file
   - Executes it in the CPU
   
2. **PROG file compilation**
   - User runs: `ND100-compiler --monitor`
   - Loads a PROG (program source) file
   - Compiles it using ND-100 compiler in the emulator
   - Produces output (executable, listing, etc)

3. **Interactive file listing**
   - `LIST-FILES` to see available BPUN/PROG files
   - Filter by type (`:BPUN`, `:PROG`, etc)

4. **File operations**
   - Create test files
   - Delete/rename output files
   - Copy between users (if multi-user)

### Architecture Differences from nd500

| Aspect | nd500x | nd100x | Impact |
|--------|--------|--------|--------|
| Domain system | Yes (complex) | No | Simplified run mechanism |
| Memory model | Byte-addressed with MMU | Word-addressed, flat | File loading simpler |
| MON calls | Full (230+) | Partial (if ENABLE_SINTRAN_SUPPORT=ON) | May need to add MON ops for file table |
| File I/O | Via ndmonlib MON calls | Via host filesystem? | Decide: emulate or direct FS? |
| User management | Full SINTRAN users | Simplified or single-user | No CREATE-USER needed? |
| Terminal types | VTM file lookup | Fixed list? | Simpler terminal config |

---

## Proposed Implementation

### Phase 1: Core Infrastructure

#### 1.1 Create nd100x_shell.{c,h}

**Structure:**
```c
// nd100x_shell.h
int nd100x_shell_run(Nd100Machine* machine, 
                     Nd100Cpu* cpu,
                     const char* sintran_root);  /* ND-100 file root */

// nd100x_shell.c (approx 400-500 lines)
// Command dispatcher
// Command implementations (10 core commands)
// Helper functions for file ops, abbreviation matching, etc
```

**Key differences from nd500x:**
- No domain/segment concepts
- Simpler program loading (just load to memory, set PC)
- Flat memory model (no MMU translation needed)
- Will use ndmonlib for file table (same host filesystem wrapper, proven SINTRAN semantics)

#### 1.2 Update nd100x.c

**Changes:**
1. Add `--monitor`/`--shell` flag to option parsing in config.c
2. Add `--script <path>` flag for batch commands
3. Add `--nd100-root <path>` flag to set file root directory
4. Detect monitor mode in main nd100x.c
5. Call nd100x_shell_run() if monitor mode enabled
6. Store config in nd100x.ini (pattern like nd500x)

**Modified files:**
- `src/frontend/nd100x/config.c` - add CLI options
- `src/frontend/nd100x/config.h` - add config fields
- `src/frontend/nd100x/nd100x.c` - add shell invocation
- `src/frontend/nd100x/CMakeLists.txt` - add nd100x_shell source

---

### Phase 2: Command Implementations

#### 2.1 Essential Commands (7)

```
HELP                - show command list
EXIT                - leave emulator
LIST-FILES [<pat>]  - list BPUN/PROG files
RUN-PROGRAM <name>  - load and run a file
LOAD-BPUN <name>    - load BPUN into memory
LOAD-PROG <name>    - load PROG into memory
SHOW-REGISTERS      - display CPU state (for debugging)
```

#### 2.2 Optional Commands (if time permits)

```
CREATE-FILE <name>  - create empty file in root
DELETE-FILE <name>  - delete file
RENAME-FILE <old>,<new> - rename
COPY-FILE <dst>,<src>   - copy file
```

#### 2.3 File Naming Convention

For ND-100, use simple extensions (NOT SINTRAN colons):
- `.bpun` - Batch Punched (executable binary)
- `.prog` - Program source (text for compilation)
- `.out` - Output file
- `.lst` - Listing file

**Mapping:**
- Host: `KERNEL.BPUN` ↔ SINTRAN: `KERNEL:BPUN`
- Host: `HELLO.PROG` ↔ SINTRAN: `HELLO:PROG`

---

### Phase 3: File Loading & Execution

#### 3.1 BPUN Loading

ND-100 BPUN files are punch-card images (paper tape format):
- Binary object code + metadata
- Can be loaded directly into memory
- Set PC to entry point, run

**Implementation:**
```c
int nd100_load_bpun(Nd100Machine* machine, const char* path);
// - Read BPUN file
// - Decode binary/punch format
// - Load into ND-100 memory (word-addressed)
// - Return entry point address
```

#### 3.2 PROG Loading

PROG files are typically text source or pre-assembled binary:
- May need assembler step first
- OR: assume pre-assembled BPUN output from prior compilation

**Approach:**
- For first version: treat PROG as text input to existing ND-100 compiler
- Invoke compiler MON call or external tool
- Redirect output to memory or file

---

### Phase 4: Integration Points

#### 4.1 Readline Support (Optional)

If `HAVE_READLINE` defined:
- Command history
- Tab completion on command names
- Up/down arrow for history (like nd500x shell)

#### 4.2 Script Support

Load commands from file:
```bash
nd100x --monitor --nd100-root ./examples --script test.cmd
```

Script format (simple):
```
# test.cmd
LOGIN SYSTEM
LIST-FILES :BPUN
RUN-PROGRAM HELLO
EXIT
```

#### 4.3 Telnet Support (Deferred)

**Not required for Phase 1** - can be added later like nd500x if needed.

---

## Detailed Task Breakdown

### Task 1: Analyze nd100x file loading

**Subtasks:**
- [ ] Understand current nd100x file format (a.out, binary boot, etc)
- [ ] Identify where programs are loaded into memory
- [ ] Document memory layout (code, data, stack)
- [ ] Understand entry point mechanism

**Deliverable:** Architecture document for nd100x program loading

### Task 2: Create config.c options

**Subtasks:**
- [ ] Add `--monitor` / `--shell` flags to long_options[]
- [ ] Add config fields: `shellMode`, `ndRootDir`, `scriptPath`
- [ ] Add help text in Config_PrintHelp()
- [ ] Parse new flags in Config_ParseCommandLine()

**Deliverable:** Updated src/frontend/nd100x/config.{c,h}

### Task 3: Create nd100x_shell.{c,h}

**Subtasks:**
- [ ] Create header with public API (nd100x_shell_run)
- [ ] Implement command dispatcher
- [ ] Implement HELP command
- [ ] Implement EXIT command
- [ ] Implement LIST-FILES command
- [ ] Implement RUN-PROGRAM command
- [ ] Add @ prompt and REPL loop
- [ ] Add command abbreviation matching

**Deliverable:** nd100x_shell.{c,h} (400-500 lines)

### Task 4: File loading functions

**Subtasks:**
- [ ] Implement nd100_load_bpun() 
- [ ] Implement file path resolution (./nd100root/filename)
- [ ] Add error handling (file not found, corrupt format)
- [ ] Test with example BPUN files

**Deliverable:** nd100x_shell.c with loading functions

### Task 5: Integration in main nd100x.c

**Subtasks:**
- [ ] Detect shell mode in config
- [ ] Call nd100x_shell_run() with machine + cpu pointers
- [ ] Handle return value (exit code)
- [ ] Test basic shell launch

**Deliverable:** Working `nd100x --monitor` launch

### Task 6: Testing & Documentation

**Subtasks:**
- [ ] Create test scripts
- [ ] Document shell commands (nd100x_shell_spec.md)
- [ ] Add examples in docs/
- [ ] Test with real BPUN files from examples/

**Deliverable:** Test suite + documentation

---

## Build System Changes

### CMakeLists.txt Updates

```cmake
# src/frontend/nd100x/CMakeLists.txt

add_executable(nd100x 
    nd100x.c
    nd100x_shell.c      # NEW
    nd100x_shell.h      # NEW
    config.c
    config.h
    keyboard.c
    # ... rest of sources
)

# Link readline if available
if(READLINE_FOUND)
    target_link_libraries(nd100x PRIVATE ${READLINE_LIBRARIES})
    target_include_directories(nd100x PRIVATE ${READLINE_INCLUDE_DIRS})
endif()
```

---

## Dependency Analysis

### External Dependencies

| Dependency | nd500x | nd100x | Required? | Notes |
|------------|--------|--------|-----------|-------|
| ndmonlib | ✅ (MON calls) | ✅ (Recommended) | Yes | Already uses host FS; provides SINTRAN semantics |
| readline | Optional | Optional | No | Command history nice-to-have |
| pthread | ✅ | ✅ | No | Already linked for other features |

### Using ndmonlib (RECOMMENDED)

ndmonlib already uses the host filesystem under the hood (`fopen`/`fclose`/`fread`/`fwrite`). It's not separate from it — it's a SINTRAN semantics wrapper:

| Layer | Implementation |
|-------|-----------------|
| **Actual I/O** | C `fopen()/fread()/fwrite()` on host filesystem |
| **Abstraction** | SINTRAN file table, path resolution, error codes, device tracking |
| **Benefit** | Same host FS access as direct approach, but with reusable SINTRAN infrastructure |

**When `ENABLE_SINTRAN_SUPPORT=ON`:**
- ndmonlib is already linked and available
- Use functions: `mon_list_files()`, `mon_open_file()`, `mon_close_file()` from `mon_file_table.c`
- Get own-dir → SYSTEM fallback for free (proven path resolution in `mon_path.c`)
- Gain file metadata (ObjectEntry structure with timestamps, size, etc.)
- Device reservation tracking for consistency
- Minimal shell code needed — mostly dispatcher + readline loop

**Recommendation:** Use ndmonlib. It does not add filesystem overhead (already uses host FS), provides proven SINTRAN semantics, and scales if nd100x needs full MON call support later.

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|-----------|
| BPUN format unclear | Medium | High | Document format early, test with examples |
| File path resolution conflicts | Low | Medium | Clear namespace (host FS in ./nd100root/) |
| Memory layout changes break loading | Low | High | Encapsulate loading in separate module |
| Terminal input/output issues | Low | Medium | Test with both console and script input |
| Readline availability varies | Low | Low | Make optional, fallback to basic input |

---

## Success Criteria

✅ Shell launches with `nd100x --monitor`  
✅ `LIST-FILES` shows BPUN/PROG files in a directory  
✅ `RUN-PROGRAM <name>` loads and executes file  
✅ `EXIT` cleanly returns from shell  
✅ Works with `--script` for batch execution  
✅ Commands support abbreviation (e.g., `LI-FI` = `LIST-FILES`)  
✅ Readline support if available (optional)  
✅ Documented in help text and nd100x_shell_spec.md

---

## Estimated Effort

| Phase | Task | Effort | Notes |
|-------|------|--------|-------|
| 1 | Analyze nd100x loading | 2h | Research |
| 2 | Config options | 1h | CLI parsing |
| 3 | nd100x_shell.c core | 4h | Command dispatcher, basic cmds |
| 4 | File loading functions | 3h | BPUN/PROG decode |
| 5 | Integration & testing | 3h | Wiring + test suite |
| 6 | Documentation | 2h | Spec + examples |
| **Total** | | **15h** | ~1.5 days |

---

## Next Steps (Recommendation)

1. **Start with Task 1** — analyze nd100x program loading
2. **Decide on dependencies** — ndmonlib vs direct FS
3. **Create minimal shell** — Tasks 2-5 with just LIST/RUN commands
4. **Test with examples** — ensure file loading works
5. **Add polish** — readline, abbreviation, help text (Task 6)

Would you like me to proceed with this plan, or modify any aspects first?
