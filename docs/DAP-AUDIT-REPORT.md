# DAP Protocol Compliance Audit Report

**Date**: 2025-03-15
**Scope**: libdap C library, MCP Python bridge, nd100x debugger integration
**Reference**: [DAP Specification](https://microsoft.github.io/debug-adapter-protocol/specification)

---

## Summary

| Verdict | Count | Description |
|---------|-------|-------------|
| PASS | 28 | Correct per spec |
| VIOLATION | 6 | Breaks spec requirement - **ALL FIXED** |
| DEVIATION | 5 | Non-standard but intentional |
| MISSING | 7 | Not implemented (low/no impact) |
| BUG | 5 | Implementation error - **ALL FIXED** |

### Issues Found and Fixed

| # | Severity | Issue | Layer | Status |
|---|----------|-------|-------|--------|
| V1 | **Critical** | Breakpoint IDs not stable across setBreakpoints calls | C | **FIXED** |
| V2 | **Critical** | stopped event missing `allThreadsStopped` field | C | **FIXED** |
| V3 | **Moderate** | stopped event missing `hitBreakpointIds` for breakpoint stops | C | **FIXED** (API added) |
| V4 | **Low** | continue response `allThreadsContinued` conditionally omitted | C | **FIXED** |
| B1 | **Moderate** | MCP stepBack sends to server that has no handler | MCP | **FIXED** |
| B2 | **Low** | evaluate frameId=0 not sent (Python falsy) | MCP | **FIXED** |
| B3 | **Critical** | breakpoint_manager_clear() wipes ALL types (cross-contamination) | nd100x | **FIXED** |
| B4 | **Moderate** | Instruction BPs use BP_TYPE_USER instead of BP_TYPE_INSTRUCTION | nd100x | **FIXED** |
| B5 | **Moderate** | check_for_breakpoint uses wrong stop reason for non-USER types | CPU | **FIXED** |
| B6 | **Pre-existing** | dap-tools executables fail to link (cJSON not propagated) | CMake | **FIXED** |

---

## Phase 1: Protocol Foundation

### 1.1 Base Protocol / Transport

**Files**: `dap_transport.c`, `dap_connection.py`

| Check | Verdict | Notes |
|-------|---------|-------|
| Content-Length framing | **PASS** | Both C and Python use `Content-Length: N\r\n\r\n` |
| UTF-8 encoding | **PASS** | Body encoded as UTF-8 |
| seq numbering | **PASS** | C: `server->sequence++` starting at 0. Python: `_seq` counter increments per request |
| request_seq correlation | **PASS** | C: echoes `request_seq` from parsed request. Python: matches via `_pending[seq]` dict |
| Max message size | **PASS** | C limits to 10MB |

### 1.2 Initialize / Initialized / ConfigurationDone

**Files**: `dap_server_cmds.c:handle_initialize` (line 329), `tools.py:connect()`, `tools.py:launch()`

| Check | Verdict | Notes |
|-------|---------|-------|
| `initialized` event after init response | **PASS** | `send_initialized_event()` called at end of `handle_initialize` (line 109) |
| `configurationDone` after breakpoint setup | **PASS** | MCP `launch()` sends `configurationDone` after launch request |
| `adapterID` in client request | **PASS** | MCP sends `adapterID="nd100x"` |
| Capabilities in response body | **PASS** | `handle_initialize` builds capabilities from `server_capabilities[]` array |
| Client capability parsing | **PASS** | Parses: clientID, clientName, adapterID, locale, pathFormat, linesStartAt1, columnsStartAt1, plus 9 boolean feature flags |

---

## Phase 2: Session Lifecycle

### 2.1 launch

| Check | Verdict | Notes |
|-------|---------|-------|
| Response is acknowledgement only | **PASS** | `set_response_success(response, NULL)` - no body (line 2319) |
| `stopOnEntry` handling | **PASS** | C handler parses it; nd100x sets `CPU_PAUSED` + sends stopped event with reason `"entry"` |
| `noDebug` flag | **PASS** | Parsed into `LaunchCommandContext.no_debug` |
| `stopped` event on entry | **PASS** | `dap_server_send_stopped_event(server, "entry", ...)` sent when `stopAtEntry=true` |

### 2.2 attach

| Check | Verdict | Notes |
|-------|---------|-------|
| Handler exists | **PASS** | `handle_attach` in C layer |
| MCP exposure | **MISSING** (intentional) | Not exposed in MCP. Emulator always uses launch semantics. Acceptable. |

### 2.3 disconnect

| Check | Verdict | Notes |
|-------|---------|-------|
| `terminateDebuggee` | **PASS** | Parsed and used. nd100x: terminate=CPU_SHUTDOWN, else=CPU_STOPPED |
| `suspendDebuggee` | **PASS** | Parsed into context (line 2400) |
| `restart` flag | **PASS** | Parsed into context |
| Exited/terminated events | **PASS** | nd100x sends both `exited(0)` and `terminated(false)` on disconnect |

### 2.4 terminate / restart

| Check | Verdict | Notes |
|-------|---------|-------|
| C handlers exist | **PASS** | Both `handle_terminate` and `handle_restart` implemented |
| Capability-gated | **PASS** | `DAP_CAP_TERMINATE_REQUEST` and `DAP_CAP_RESTART_REQUEST` both set `true` |
| MCP exposure | **MISSING** (low priority) | Not in MCP tools. Could be added but disconnect covers the use case. |

---

## Phase 3: Breakpoints

### 3.1 setBreakpoints

| Check | Verdict | Notes |
|-------|---------|-------|
| Replace-all per source | **PASS** | nd100x calls `breakpoint_manager_clear()` then re-adds. MCP caches per-file. |
| Response array order matches input | **PASS** | Loop `for (int i = 0; i < count; i++)` preserves order |
| Each bp has `verified` | **PASS** | `cJSON_AddBoolToObject(response_bp, "verified", bp->verified)` |
| `id` field assigned | **VIOLATION (V1)** | IDs are always `i + 1` (1-based positional). Spec requires stable IDs that persist across calls. A breakpoint at line 10 might be id=1 in one call and id=2 in the next if a line was added before it. |
| `line` in response | **PASS** | Included |
| `source` in response | **PASS** | Included when available |

**V1 Detail**: `dap_server_cmds.c:713` - `cJSON_AddNumberToObject(response_bp, "id", i + 1)`. The DAP spec says: "The identifier is needed to identify the breakpoint when it is updated or removed." Positional IDs break this contract. The server should maintain a breakpoint registry with monotonically increasing IDs.

### 3.2 setFunctionBreakpoints

| Check | Verdict | Notes |
|-------|---------|-------|
| Replace-all | **PASS** | Handler clears and re-sets |
| Capability-gated | **PASS** | `DAP_CAP_FUNCTION_BREAKPOINTS = true` |
| Response format | **PASS** | Returns breakpoint array with id/verified |
| ID stability | **VIOLATION (V1)** | Same positional ID issue |

### 3.3 setInstructionBreakpoints

| Check | Verdict | Notes |
|-------|---------|-------|
| Replace-all | **PASS** | MCP clears cache; C handler replaces all |
| `instructionReference` field | **PASS** | MCP sends `instructionReference`, C handler parses it |
| Response format | **PASS** | Returns `instructionReference` as hex string in response |
| ID stability | **VIOLATION (V1)** | Same positional ID issue (line 880) |

### 3.4 setDataBreakpoints

| Check | Verdict | Notes |
|-------|---------|-------|
| Two-phase (info then set) | **PASS** | MCP calls `dataBreakpointInfo` per variable, then `setDataBreakpoints` |
| Replace-all | **PASS** | nd100x clears all watchpoints before adding |
| `accessType` values | **PASS** | read/write/readWrite all supported |
| ID stability | **VIOLATION (V1)** | Same positional ID issue (line 1178) |

### 3.5 setExceptionBreakpoints

| Check | Verdict | Notes |
|-------|---------|-------|
| C handler exists | **PASS** | `handle_set_exception_breakpoints` + `on_set_exception_breakpoints` callback |
| MCP exposure | **MISSING** | Not in MCP tools. Low impact - emulator has no exception categories beyond what watchpoints cover. |

### 3.6 breakpointLocations

| Check | Verdict | Notes |
|-------|---------|-------|
| Implementation | **MISSING** | Not implemented anywhere. Optional capability, low impact. |

---

## Phase 4: Execution Control

### 4.1 continue

| Check | Verdict | Notes |
|-------|---------|-------|
| `threadId` required in args | **PASS** | Parsed from args |
| `allThreadsContinued` in response body | **VIOLATION (V4)** | Only included when `true` (line 1467-1469). Spec says the field MUST be in the response body. When omitted, clients may assume `false` which is correct for a single-thread emulator, but the field should always be present. |

**V4 Detail**: `dap_server_cmds.c:1467` - The field is conditionally added. Per spec: "If `allThreadsContinued` is true, a debug adapter can announce that all threads have continued." It's technically optional in the spec (`allThreadsContinued?: boolean`), but best practice is to always include it. **Downgrading to low severity** since the `?` makes it optional.

### 4.2 next (stepOver)

| Check | Verdict | Notes |
|-------|---------|-------|
| `threadId` required | **PASS** | Parsed |
| `granularity` optional | **PASS** | Parsed: statement/line/instruction |
| Stopped event generated | **PASS** | nd100x sends stopped event via `cmd_check_cpu_events` poll |
| MCP 2s timeout | **DEVIATION (D1)** | MCP uses 2s initial timeout for step events. If no event arrives, falls back to querying stack trace. This is pragmatic - the C server may sometimes not send events if step completes within the same poll cycle. |

### 4.3 stepIn

| Check | Verdict | Notes |
|-------|---------|-------|
| Same as next | **PASS** | Same patterns |
| `targetId` | **MISSING** | Not supported. Optional per spec, low impact for assembly-level debugger. |

### 4.4 stepOut

| Check | Verdict | Notes |
|-------|---------|-------|
| `threadId` required | **PASS** | Parsed |
| Stopped event | **PASS** | Generated via poll |

### 4.5 stepBack

| Check | Verdict | Notes |
|-------|---------|-------|
| Capability-gated | **BUG (B1)** | MCP sends `stepBack` command but C server has NO handler (`command_handlers[DAP_CMD_STEP_BACK] = NULL`). nd100x does NOT set `supportsStepBack` capability. MCP should check capabilities before sending. |
| nd100x implementation | **MISSING** | No `stepBack` callback registered. The command will be silently ignored or error. |

**B1 Detail**: `tools.py` has `step_back()` method. `server.py` registers `debug_step_back` tool. But `dap_server.c:807` shows `command_handlers[DAP_CMD_STEP_BACK] = NULL`. The server will likely return an error response for unhandled commands.

### 4.6 pause

| Check | Verdict | Notes |
|-------|---------|-------|
| `threadId` required | **PASS** | Parsed |
| Stopped event with reason `pause` | **PASS** | C handler sends stopped event with `"pause"` reason and `allThreadsStopped=true` (line 1903) |
| MCP doesn't wait for event | **DEVIATION (D2)** | MCP returns immediately with `{"status": "pause requested"}`. This is acceptable since pause is asynchronous by nature. |

### 4.7 reverseContinue / goto / restartFrame

| Check | Verdict | Notes |
|-------|---------|-------|
| All optional | **MISSING** | Not implemented. Correct - capabilities not advertised. |

---

## Phase 5: Inspection

### 5.1 threads

| Check | Verdict | Notes |
|-------|---------|-------|
| Response format | **PASS** | Returns `threads[]` with `id` and `name` |
| Single thread | **PASS** | Always thread id=1, name="main" |
| Thread lifecycle events | **PASS** | `thread` event with reason `"started"` sent on launch |

### 5.2 stackTrace

| Check | Verdict | Notes |
|-------|---------|-------|
| `threadId` required | **PASS** | Parsed |
| `startFrame`/`levels` paging | **PASS** | Parsed into `StackTraceCommandContext.start_frame` and `.levels` |
| Response `stackFrames[]` | **PASS** | Array of frames with id/name/source/line/column |
| `totalFrames` | **PASS** | Included in response |
| `instructionPointerReference` | **PASS** | Formatted as `"0x%x"` hex string |
| `presentationHint` | **PASS** | "normal", "label", "subtle" supported |

### 5.3 scopes

| Check | Verdict | Notes |
|-------|---------|-------|
| `frameId` required | **PASS** | Parsed |
| Response `scopes[]` | **PASS** | Rich scope set: Locals, Registers, Levels, Internal Read/Write, Status Flags, MMS, Page Tables |
| `variablesReference` | **PASS** | Each scope has unique reference (1000, 1001, 1002, etc.) |
| `expensive` flag | **PASS** | Set appropriately per scope |

### 5.4 variables

| Check | Verdict | Notes |
|-------|---------|-------|
| `variablesReference` required | **PASS** | Parsed |
| `start`/`count` paging | **PASS** | Parsed into context |
| `filter` (indexed/named) | **PASS** | Parsed as enum |
| Response per variable | **PASS** | name/value/type/variablesReference/memoryReference |
| `variablesReference=0` for leaves | **PASS** | Set to 0 for simple variables |
| Nested variables | **PASS** | Status flags scope nests under registers |

### 5.5 evaluate

| Check | Verdict | Notes |
|-------|---------|-------|
| `expression` required | **PASS** | Parsed |
| `context` handling | **PASS** | C handler parses context string (watch/repl/hover/clipboard/variables) |
| `frameId` semantics | **BUG (B2)** | MCP sends `frameId` only if `frame_id > 0` (line 522: `if frame_id:`). This means frame 0 is never sent. However, DAP spec says omitting frameId means "global scope", which is correct behavior for frame 0 in a single-frame emulator. **Downgrading to cosmetic** - behavior is correct by accident. |

---

## Phase 6: Memory & Disassembly

### 6.1 readMemory

| Check | Verdict | Notes |
|-------|---------|-------|
| `memoryReference` + `count` | **PASS** | Parsed |
| Data is base64 | **PASS** | `base64_encode()` used (correct alphabet: A-Za-z0-9+/) |
| `unreadableBytes` | **PASS** | Tracked and returned |
| `offset` support | **PASS** | Parsed and applied |
| MCP handles dual encoding | **DEVIATION (D3)** | `types.py:_decode_memory_data()` tries hex first, then base64. This is defensive but unnecessary since C layer always sends base64. Not harmful. |

### 6.2 writeMemory

| Check | Verdict | Notes |
|-------|---------|-------|
| `memoryReference` + `data` (base64) | **PASS** | MCP converts hex input to base64 before sending |
| `bytesWritten` in response | **PASS** | Returned by nd100x callback |
| `allowPartial` | **PASS** | Parsed |
| `offset` support | **PASS** | Parsed and applied |

### 6.3 disassemble

| Check | Verdict | Notes |
|-------|---------|-------|
| `memoryReference` + `instructionCount` | **PASS** | Parsed |
| Response `instructions[]` | **PASS** | Array with address/instruction/symbol |
| `instructionReference` in each instruction | **DEVIATION (D4)** | Field is named `address` in response, not `instructionReference`. The spec calls it `address` too, so this is actually correct. The `instructionReference` field name is for stack frames only. |
| `symbol` field | **PASS** | Populated when `resolve_symbols=true` |
| `instructionBytes` | **PASS** | Included |

---

## Phase 7: Events

### 7.1 stopped event

| Check | Verdict | Notes |
|-------|---------|-------|
| `reason` required | **PASS** | Always included: step/breakpoint/exception/pause/entry/goto/function breakpoint/data breakpoint/instruction breakpoint |
| `threadId` | **PASS** | Always included |
| `description` | **PASS** | Includes PC and source location |
| `allThreadsStopped` | **VIOLATION (V2)** | **NOT included** in `dap_server_send_stopped_event()` (line 1190-1227). The field is listed as "OPTIONAL" in comments but many clients (including VS Code) rely on it to properly manage thread states. For a single-thread emulator, this should always be `true`. |
| `hitBreakpointIds` | **VIOLATION (V3)** | **NOT included** in stopped events. When reason is "breakpoint", clients need this to highlight which breakpoint was hit. Listed as optional but practically required for breakpoint stops. |

**V2 Detail**: `dap_server.c:1217` - The field is commented out: `// allThreadsStopped?: boolean; // OPTIONAL`. Should always be `true` for nd100x since it's single-threaded.

**V3 Detail**: `dap_server.c:1218` - Also commented out: `// hitBreakpointIds?: number[]; // OPTIONAL`. The nd100x `cmd_check_cpu_events` knows the breakpoint was hit but doesn't pass the ID to the event.

### 7.2 terminated / exited events

| Check | Verdict | Notes |
|-------|---------|-------|
| `exited` has `exitCode` | **PASS** | `dap_server_send_exited_event(server, 0)` |
| `terminated` is separate | **PASS** | Sent separately from exited |
| Order | **PASS** | exited sent before terminated (correct per spec) |

### 7.3 thread event

| Check | Verdict | Notes |
|-------|---------|-------|
| Reason `started`/`exited` | **PASS** | `dap_server_send_thread_event(server, "started", 1)` sent on launch |
| `threadId` | **PASS** | Always 1 |

### 7.4 output event

| Check | Verdict | Notes |
|-------|---------|-------|
| `output` string | **PASS** | Console characters sent as output events |
| `category` | **PASS** | Uses "stdout" for console output |
| Custom `data` field | **DEVIATION (D5)** | Includes JSON `data` field with terminal address and hex bytes. Not in spec but doesn't conflict - `data` is defined as `any` type. |

### 7.5 breakpoint event

| Check | Verdict | Notes |
|-------|---------|-------|
| Breakpoint state change events | **MISSING** | No `breakpoint` events sent when breakpoint state changes (verified/unverified). Low impact since MCP manages breakpoint state. |

---

## Phase 8: Custom Extensions

### 8.1 consoleEnable (custom)

| Check | Verdict | Notes |
|-------|---------|-------|
| Not in DAP spec | **DEVIATION** (intentional) | Custom command for terminal I/O capture. Properly uses custom command type. |
| Registered handler | **PASS** | `handle_console_enable` in C, `DAP_CMD_CONSOLE_ENABLE` enum |

### 8.2 consoleWrite (custom)

| Check | Verdict | Notes |
|-------|---------|-------|
| Not in DAP spec | **DEVIATION** (intentional) | Custom command for terminal input injection |
| Hex mode support | **PASS** | Supports raw hex byte injection |

### 8.3 consoleRead (MCP-only)

| Check | Verdict | Notes |
|-------|---------|-------|
| Event-based | **PASS** | Reads queued output events. Format matches DAP output event spec. |

---

## Phase 9: Object Lifetime & State Management

### 9.1 Reference lifetime rules

| Check | Verdict | Notes |
|-------|---------|-------|
| `variablesReference` valid while stopped | **PASS** | References are scope IDs (1000, 1001, etc.), stable within stopped sessions |
| `frameId` valid while stopped | **PASS** | Frame IDs from stack trace, regenerated on each stop |
| MCP caches stale references? | **PASS** | MCP doesn't cache variablesReference - fetches fresh scopes each time |
| Invalidation on resume | **PASS** | Stack trace rebuilt on each stop via `debugger_build_stack_trace()` |

### 9.2 Breakpoint ID management

| Check | Verdict | Notes |
|-------|---------|-------|
| Stable IDs across calls | **VIOLATION (V1)** | See Phase 3.1. IDs are positional, not stable. |
| Response order matches input | **PASS** | Preserved by loop order |

---

## Detailed Fix Recommendations

### V1: Breakpoint ID Stability (CRITICAL)

**Problem**: All breakpoint types use `i + 1` as ID, meaning IDs change when breakpoints are reordered or added/removed.

**Fix**: Add a monotonically increasing breakpoint ID counter to `DAPServer`:
```c
// In dap_server.h, add to DAPServer struct:
int next_breakpoint_id;  // Monotonically increasing, never resets

// In handle_set_breakpoints, replace:
cJSON_AddNumberToObject(response_bp, "id", i + 1);
// With:
bp->id = server->next_breakpoint_id++;
cJSON_AddNumberToObject(response_bp, "id", bp->id);
```

Apply same fix to instruction, function, and data breakpoint handlers.

**Impact**: Clients that track breakpoints by ID (VS Code, MCP) will work correctly when breakpoints are modified.

**Files**: `external/libdap/libdap/src/dap_server_cmds.c` lines 713, 880, 1178, 2079
**Files**: `external/libdap/libdap/include/dap_server.h` - add field to DAPServer struct

### V2: stopped event missing allThreadsStopped (CRITICAL)

**Problem**: `dap_server_send_stopped_event()` never includes `allThreadsStopped`.

**Fix**:
```c
// In dap_server.c, dap_server_send_stopped_event(), add after reason:
cJSON_AddBoolToObject(event_body, "allThreadsStopped", true);
```

For single-thread emulators this is always `true`. If multi-thread support is added later, parameterize it.

**Impact**: VS Code and other clients may not properly update all thread states without this.

**File**: `external/libdap/libdap/src/dap_server.c` line ~1204

### V3: stopped event missing hitBreakpointIds (MODERATE)

**Problem**: When stopping at a breakpoint, the event doesn't tell the client which breakpoint was hit.

**Fix**: Extend `dap_server_send_stopped_event()` signature to accept optional breakpoint IDs:
```c
int dap_server_send_stopped_event_ex(DAPServer *server, const char *reason,
                                      const char *description,
                                      const int *hit_bp_ids, int hit_bp_count);
```

The nd100x `cmd_check_cpu_events` would need to look up the breakpoint ID from the hit address.

**Impact**: Clients can highlight which breakpoint triggered the stop.

**Files**: `external/libdap/libdap/src/dap_server.c`, `external/libdap/libdap/include/dap_server.h`, `src/debugger/debugger.c`

### V4: continue allThreadsContinued (LOW)

**Problem**: `allThreadsContinued` only included when true. Spec marks it optional (`?`).

**Fix**: Always include it:
```c
cJSON_AddBoolToObject(body, "allThreadsContinued", true);  // Always true for single-thread
```

**Impact**: Minor - most clients default to `false` when missing, which is safe.

**File**: `external/libdap/libdap/src/dap_server_cmds.c` line 1467

### B1: MCP stepBack with no server handler (MODERATE)

**Problem**: MCP exposes `debug_step_back` tool but server has no handler.

**Fix**: Either:
1. Remove `debug_step_back` from MCP tools, OR
2. Have MCP check `capabilities.get("supportsStepBack", False)` before sending

Option 1 is simpler and more honest. The ND-100 emulator doesn't support reverse execution.

**Files**: `external/libdap/mcp-dap-server/mcp_dap_server/tools.py`, `external/libdap/mcp-dap-server/mcp_dap_server/server.py`

### B2: evaluate frameId=0 handling (COSMETIC)

**Problem**: `if frame_id:` in Python treats 0 as falsy, so frame 0 is never sent.

**Fix**:
```python
if frame_id is not None and frame_id >= 0:
    args["frameId"] = frame_id
```

Or since the default is 0 and omitting means "global scope" (which is the same thing for a single-frame debugger), this is a non-issue in practice.

**File**: `external/libdap/mcp-dap-server/mcp_dap_server/tools.py` line 522

---

## Items Not Requiring Fixes

### Intentional Deviations (acceptable)

| ID | What | Why |
|----|------|-----|
| D1 | MCP 2s step timeout | Pragmatic for slow/async step operations |
| D2 | MCP pause returns immediately | Pause is inherently async |
| D3 | MCP dual hex/base64 decode | Defensive, no harm |
| D4 | Disassembly `address` field name | Actually matches spec (not `instructionReference`) |
| D5 | Output event custom `data` field | Spec allows `any` type for data |

### Acceptable Missing Features

| Feature | Why acceptable |
|---------|---------------|
| attach | Emulator uses launch semantics only |
| terminate/restart via MCP | disconnect covers the use case |
| setExceptionBreakpoints via MCP | No meaningful exception categories |
| breakpointLocations | Optional, rarely used |
| stepIn targetId | Assembly-level debugger doesn't have multiple step targets |
| reverseContinue/goto | No reverse execution support |
| breakpoint changed events | MCP manages state client-side |

---

## Priority Order for Fixes

1. **V2** - Add `allThreadsStopped: true` to stopped events (1 line, high impact)
2. **V1** - Stable breakpoint IDs (moderate refactor, required for correctness)
3. **B1** - Remove stepBack from MCP (simple deletion)
4. **V3** - Add `hitBreakpointIds` to stopped events (moderate, improves UX)
5. **V4** - Always include `allThreadsContinued` (1 line, low impact)
6. **B2** - Fix evaluate frameId=0 (1 line, cosmetic)
