# Background Worker Implementation - Living TODO

> Living document tracking Web Worker + WebSocket + Pop-out Terminals implementation.
> Updated throughout with completion dates, learnings, and deviations.

---

## Phase 0: Proxy Abstraction Layer (no Worker yet)

**Goal**: Wrap all `Module._*` calls behind `window.emu` proxy. Direct pass-through. Zero behavioral change.

### Tasks

- [x] Create `/home/ronny/repos/nd100x/template-glass/js/emu-proxy.js` with `window.emu` proxy (2026-02-12)
- [x] Migrate `emulation.js` (8 call sites) (2026-02-12)
- [x] Migrate `terminal.js` (10+ call sites) (2026-02-12)
- [x] Migrate `toolbar.js` (8 call sites) (2026-02-12)
- [x] Migrate `module-init.js` (5+ call sites) (2026-02-12)
- [x] Migrate `debugger.js` (46 call sites - largest file) (2026-02-12)
- [x] Migrate `disassembly.js` (9 call sites) (2026-02-12)
- [x] Migrate `breakpoints.js` (13 call sites) (2026-02-12)
- [x] Migrate `sintran-symbols.js` (14 call sites) (2026-02-12)
- [x] Migrate `sintran.js` (4 call sites) (2026-02-12)
- [x] Migrate `pagetables.js` (8 call sites) (2026-02-12)
- [x] Migrate `floppy-browser.js` (7 call sites) (2026-02-12)
- [x] Update `index.html` script loading order (add emu-proxy.js) (2026-02-12)
- [x] Build with `make wasm-glass` and verify (2026-02-12)
- [x] Verify no remaining direct `Module._` calls outside `emu-proxy.js` (2026-02-12)
- [x] Runtime test: boot SINTRAN, use debugger, open SINTRAN inspectors (2026-02-12)

### Learnings & Gotchas

- **debugger.js** was the most complex migration (46 call sites) due to dynamic dispatch patterns:
  - `regSetters` dict mapped Module function names to emu method names
  - `stepMethodMap` translates `'_Dbg_StepOne'` style names to emu method names
  - `renderSystemRegs` used `Module[r.fn]` pattern - changed to `emu[r.fn]` with new getter names
- **breakpoints.js** had a try-without-catch bug during migration - fixed by ensuring proper catch block
- **sintran-symbols.js** uses `emu.hasFunction()` and `emu.getHEAPU16Buffer()` for the bulk read fast path
- **floppy-browser.js** mount path simplified - `emu.remountFloppy()` always exists via proxy, no need for `if (Module._RemountFloppy)` guard
- Script load order: `module-init.js` -> `emu-proxy.js` -> `nd100wasm.js` -> all other modules
- `module-init.js` still defines `var Module = {}` (Emscripten config) - this is correct and expected
- Only `emu-proxy.js` contains `Module._` calls after migration - verified via grep

---

## Phase 1: Terminal Output Ring Buffer (C-side prerequisite)

**Goal**: Replace char-by-char `EM_ASM` callback with pollable ring buffer.

### Tasks

- [x] Add ring buffer struct to `nd100wasm.c` (2026-02-12)
- [x] Add `EnableTerminalRingBuffer()` export (2026-02-12)
- [x] Add `PollTerminalOutput()` export - single poll function replaces 4 buffer-access functions (2026-02-12)
- [x] Modify `WasmTerminalOutputHandler` for tri-mode: ring buffer > EM_ASM > callbacks (2026-02-12)
- [x] Update CMakeLists.txt with 2 new exports: `EnableTerminalRingBuffer`, `PollTerminalOutput` (2026-02-12)
- [x] Add `flushTerminalOutput()` to emu-proxy.js (2026-02-12)
- [x] Update emulation.js to call flush after each step (2026-02-12)
- [x] Add flush calls in debugger.js after step/run operations (2026-02-12)
- [x] Build with `make wasm-glass` and verify (2026-02-12)
- [x] Runtime test: boot SINTRAN, verify terminal output works via ring buffer (2026-02-12)
- [x] Verify loading overlay still hides on first output (2026-02-12)

### Learnings & Gotchas

- **HEAPU16 trap**: Original plan used HEAPU16 for JS-side buffer reads. This crashes because Emscripten creates abort-getters for non-exported properties, and even EXPORTED_RUNTIME_METHODS doesn't reliably expose it with MODULARIZE=1. **Never access Module.HEAPU16 directly in hot paths.** Replaced 4 buffer-access exports with single `PollTerminalOutput()` C function - no HEAPU16, no pointer math, no shared constants.
- Ring buffer is 8192 uint16 entries, each packed as `(identCode << 8) | charCode`
- `PollTerminalOutput()` returns next entry or -1 if empty. JS loops until -1.
- Flush calls in 3 places: emulation.js main loop, debugger.js doStep(), debugger.js execFrame()
- WasmTerminalOutputHandler priority: ring buffer > EM_ASM > legacy callbacks

---

## Phase 2: Web Worker Emulation Core

**Goal**: Move WASM instantiation and execution loop to a dedicated Worker.

### Tasks

- [x] Create `/home/ronny/repos/nd100x/template-glass/js/emu-worker.js` (2026-02-12)
- [x] Create `/home/ronny/repos/nd100x/template-glass/js/emu-proxy-worker.js` (2026-02-12)
- [x] Add mode toggle to `index.html` (load emu-proxy.js OR emu-proxy-worker.js) (2026-02-12)
- [x] Update `module-init.js` for Worker mode (disk loading via postMessage) (2026-02-12)
- [x] Update `emulation.js` for Worker mode (Worker runs autonomously, main samples cached state) (2026-02-12)
- [x] Update `toolbar.js` for async init/boot in Worker mode (2026-02-12)
- [x] Update `debugger.js` for async step/run/pause in Worker mode (2026-02-12)
- [x] Update `floppy-browser.js` for Worker mode (FS ops through proxy - no changes needed, proxy handles routing) (2026-02-12)
- [x] Add Worker mode toggle to Config window in `index.html` + handler in `toolbar.js` (2026-02-12)
- [x] Fix background-tab throttling: replaced `setTimeout(0)` with `MessageChannel` scheduling (2026-02-12)
- [x] Fix message flooding: batch multiple `Step()` calls per ~16ms frame, send one consolidated frame (2026-02-12)
- [x] Fix Phase 3 stub console.warn spam: `warnOnce()` helper (2026-02-12)
- [x] Verify boot SINTRAN in Worker mode (2026-02-12)
- [x] Background tab 5+ minutes test (2026-02-12)
- [x] Verify `CLOCK` shows correct time advancement (2026-02-12)
- [x] Toggle back to direct mode - verify everything still works (2026-02-12)

### Architecture

- **Direct mode** (default): `emu-proxy.js` -> `Module._*` -> WASM (synchronous, same as Phase 0/1)
- **Worker mode** (`?worker=1` or localStorage `nd100x-worker=true` or Config toggle):
  - Worker thread: `emu-worker.js` loads WASM via `importScripts`, runs execution loop via `MessageChannel` (immune to background-tab throttling)
  - Main thread: `emu-proxy-worker.js` provides same API as `emu-proxy.js`, returns cached snapshot values
  - Communication: `postMessage` with Transferable ArrayBuffers for disk images
  - Frame batching: Worker runs multiple `Step(10000)` calls per ~16ms window, sends one consolidated frame at ~60fps
- Mode detection: `USE_WORKER` global set in `index.html` before any scripts load
- Config UI toggle in Config window writes `nd100x-worker` localStorage key, prompts reload
- `document.write` used for conditional synchronous script loading

### Message Protocol

Worker -> Main: `ready`, `initialized`, `booted`, `frame`, `breakpoint`, `stopped`, `stepDone`, `runDbgDone`, `log`, `diskLoaded`, `fsResult`, `snapshot`
Main -> Worker: `init`, `boot`, `start`, `stop`, `key`, `carrier`, `step`, `setPaused`, `runDbg`, `addBreakpoint`, `removeBreakpoint`, `clearBreakpoints`, `addWatchpoint`, `removeWatchpoint`, `clearWatchpoints`, `setReg`, `writeMemory`, `snapshot`, `loadDisk`, `fsWrite`, `fsUnlink`, `remountFloppy`, `remountSMD`, `unmountFloppy`, `unmountSMD`

### Learnings & Gotchas

- **Non-modularized build**: The root CMakeLists.txt overrides MODULARIZE=1. nd100wasm.js creates a global `Module`. Worker pre-defines `var Module = {...}` then `importScripts('../nd100wasm.js')` - Emscripten merges into the pre-defined object. No factory pattern needed.
- **locateFile path**: Worker JS lives in `js/`, WASM files in parent. `locateFile: function(path) { return '../' + path; }` resolves correctly.
- **Terminal info caching**: Worker's `initialized` response includes terminal address/identCode/logicalDevice for all 16 slots. Proxy caches this so `getTerminalAddress(i)` etc work synchronously on main thread.
- **Frame messages**: Worker posts `frame` with `termOutput` array (packed entries), `runMode`, `pil`, `sts`, `pc`, `instrCount`. Proxy dispatches terminal output via `handleTerminalOutputFromC` and updates cached state.
- **Async init/boot**: toolbar.js uses `emu.onInitialized` and `emu.onBooted` callbacks. Direct mode path unchanged (synchronous).
- **Debugger step**: In Worker mode, `doStep()` sets `emu.onStepDone` callback, calls `emu.stepOne()` (returns Promise), callback refreshes view on response.
- **floppy-browser.js**: No code changes needed - `emu.fsWriteFile()` in worker proxy routes through Worker via `fsWrite` message. `emu.remountFloppy()` routes through Worker.
- **Phase 3 stubs**: `readMemory`, `disassemble`, `getLevelInfo`, `ccall`, `getBreakpointList`, `fsReadFile`, `fsStat` return stub values with console.warn in Worker mode. All SINTRAN inspectors deferred to Phase 3.
- **Disk loading**: `loadDiskImage()` in module-init.js calls `emu.workerLoadDisk(path, buf)` in Worker mode, which transfers the ArrayBuffer (zero-copy) to the Worker.
- **setTimeout throttling**: `setTimeout(0)` in Workers IS throttled to 1s+ in background tabs. Replaced with `MessageChannel.postMessage()` which is immune to throttling. Pattern: create channel, port1.onmessage = runLoop, schedule via port2.postMessage(null).
- **Frame batching**: Posting one message per 10K instructions flooded the main thread (thousands/sec). Changed to batch multiple `Step(10000)` calls within ~16ms, send one consolidated frame. Eliminates RAF violation warnings.
- **warnOnce for Phase 3 stubs**: SINTRAN detection polls `readMemory()` every 2s. Phase 3 stub was spamming console.warn. Added `warnOnce()` helper with `_warned` object to log each message only once.
- **Config toggle**: Worker mode toggle in Config window uses same `nd100x-worker` localStorage key. Prompts for page reload since Worker mode can only switch at startup.

---

## Phase 3: Async Debugger & SINTRAN Inspectors

**Goal**: Make debugger operations work in Worker mode via batch commands.

### Tasks

- [x] Add Worker message handlers for all read operations (readMemory, readMemoryBlock, readPhysicalMemory, readPhysicalMemoryBlock, dumpPhysicalMemory, getBreakpointList, getWatchpointInfo, disassemble, getLevelInfo, getPageTableEntryRaw, getPageTableMap, fsRead, fsStat) (2026-02-12)
- [x] Replace all Phase 3 stubs in emu-proxy-worker.js with real postRequest() calls (2026-02-12)
- [x] Add batch getPageTableMap() to both proxies (reads 64 PTEs in one round-trip) (2026-02-12)
- [x] Update `debugger.js` for async memory dump (readMemoryBlock) and levels tab (2026-02-12)
- [x] Update `disassembly.js` for async disassembly render (2026-02-12)
- [x] Update `breakpoints.js` for async breakpoint/watchpoint list (2026-02-12)
- [x] Update `pagetables.js` for batch PTE read via getPageTableMap (2026-02-12)
- [x] Update `sintran-symbols.js` - all read functions return Promises, added sync cache helpers (discoverRtTableSync, rtAddrToNumberSync) (2026-02-12)
- [x] Update `sintran.js` - async SINTRAN detection and syseval (2026-02-12)
- [x] Update `sintran-segments.js` - async readSegmentTable, crossReferenceSegments, refreshSegments (2026-02-12)
- [x] Update `sintran-processes.js` - async readProcessList, buildExecQueueSet (recursive Promise chain), selectProcess reservation chain walk (2026-02-12)
- [x] Update `sintran-queues.js` - async readExecQueue, readTimeQueue, readMonitorQueue (recursive Promise chains for linked list walks) (2026-02-12)
- [x] Update `sintran-devices.js` - async 4-phase discovery with Promise.all for Phase 1, recursive chain walks for Phase 2/3, async buildDynamicLogDevNoMap (2026-02-12)
- [x] Update `sintran-rt-names.js` - resolveProcessName/Description use sync cache (discoverRtTableSync) (2026-02-12)
- [x] Verify all SINTRAN inspectors in Worker mode (2026-02-12)
- [x] Proxy ccall through Worker for Variables/Stack tabs (2026-02-12)
- [x] Add all CPU registers (A,D,B,T,L,X,EA) to Worker frame messages (2026-02-12)
- [x] Add debugger refresh in Worker mode emulation loop (dbgRefreshIfVisible ~4x/sec) (2026-02-12)
- [x] Fix debugger not updating during run: try-catch isolation + RAF-driven refresh (2026-02-12)
- [x] Verify debugger in Worker mode (2026-02-12)
- [x] Verify direct mode unchanged (2026-02-12)

### Learnings & Gotchas

- **Promise.resolve() pattern**: `Promise.resolve(emu.readMemory(addr)).then(...)` works transparently in both modes - wraps sync values in direct mode, passes through Promises in Worker mode. This is the KEY design pattern for Phase 3.
- **Batch operations**: `getPageTableMap(pt)` reads all 64 PTE entries in one Worker round-trip instead of 64 individual calls. `readMemoryBlock(addr, count)` similarly batches reads.
- **Sync cache helpers**: Added `discoverRtTableSync()` and `rtAddrToNumberSync()` that read from the cached RT table info without returning Promises. Essential for render-time name resolution (resolveProcessName, resolveOwner, resolveWaiter) which runs synchronously inside DOM rendering code.
- **Linked list walks**: Converted all while-loop chain walks (exec queue, time queue, monitor queue, reservation chains) to recursive Promise chains. Pattern: `function walk(addr, count, visited) { return sym.readWord(addr).then(function(next) { return walk(next, count+1, visited); }); }`
- **Promise.all for parallel reads**: Phase 1 device discovery batches all known device header reads via `Promise.all(headerPromises)`. SINTRAN syseval batches 12 word reads. Process list reads RT table + RTREF + exec queue set in parallel.
- **Sequential chains for dependent reads**: Reservation chain walks, LOGDBANK group reads, and page-boundary-crossing block reads chain with `.then()` to maintain ordering.
- **ccall proxy**: `Module.ccall()` now proxied through Worker via `postRequest('ccall', ...)`. Worker calls `Module.ccall()` directly and returns the result. `safeJsonCall()` in debugger.js returns a Promise; `renderVariables()` and `renderStackTrace()` use `.then()` for async rendering.
- **Refresh pattern**: All refresh functions now call `sym.invalidateRtCache()`, then `sym.discoverRtTable().then()` (populates sync cache), then the data reader (returns Promise), then the renderer (synchronous with resolved data).

---

## Phase 4: Pop-out Terminals via BroadcastChannel

**Goal**: Detach any terminal into a separate browser window.

### Tasks

- [ ] Create `/home/ronny/repos/nd100x/template-glass/terminal-popout.html`
- [ ] Create `/home/ronny/repos/nd100x/template-glass/js/terminal-bridge.js`
- [ ] Add "Pop out" button to terminal window headers
- [ ] Implement BroadcastChannel protocol (output, settings, input, close)
- [ ] Route terminal output to pop-out windows
- [ ] Route keyboard input from pop-out back to emulator
- [ ] Add pop-out close handler (restore main window terminal)
- [ ] Add `terminal-bridge.js` to `index.html`
- [ ] Verify pop-out receives real-time output
- [ ] Verify keyboard input from pop-out works
- [ ] Verify main tab backgrounded + pop-out visible = output flows

### Learnings & Gotchas

*(Updated during implementation)*

---

## Phase 5: WebSocket Bridge (Future)

**Goal**: External clients connect via WebSocket for remote terminal access.

### Tasks

- [ ] Design WebSocket bridge server (Node.js or Python)
- [ ] Implement Worker-to-WS adapter
- [ ] Implement client library
- [ ] Same protocol as BroadcastChannel, wrapped in WS frames

### Learnings & Gotchas

*(Updated during implementation)*

---

## Problems Encountered

*(Entries added as issues arise)*

---

## Plan Deviations

*(Entries added when the plan changes)*
