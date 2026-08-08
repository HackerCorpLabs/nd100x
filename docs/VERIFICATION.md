# Verification Recipes

How to verify a change in each subsystem, and the standing safety rules for
doing so. All paths are relative to the repo root.

## Standing rules

1. **Never open the user's original disk images at runtime.** Booting
   SINTRAN writes to the pack (swap + segment data). Copy the image to a
   scratch location first and run against the copy:
   ```bash
   cp SMD0.IMG /tmp/scratch-SMD0.IMG && build/bin/nd100x --boot=smd --smd0=/tmp/scratch-SMD0.IMG
   ```
2. **Confirm which binary you are testing.** There are multiple build
   directories (build/, build_linux/, build_wasm/, ...) and they diverge.
   `<builddir>/bin/nd100x --version` prints version, git hash (+`-dirty`)
   and build time. If the hash does not match the source state you edited,
   the test proves nothing.
3. **Debug sessions use a non-default DAP port** (e.g. `--port=6661`),
   never 4711 - the default port may be held by a live session.
4. **Never kill a process you did not start** to free a port; pick another
   port instead.

## Unit tests

```bash
make test          # builds debug and runs the CTest suite (tests/)
```
Covers the printer/PDF subsystem (test_escp.c, test_pdfwriter.c,
test_printjob.c) plus machine-config and device tests. Note: `*_protos.h`
CMake dependencies are only exercised by clean parallel builds - a passing
incremental build does not prove the CMake dependency graph is right (CI
builds from a clean tree and will catch it, at ~15 min per round).

## CPU / MMS conformance: TPE test programs

The vendor TPE verifiers (INSTRUCTION, PAGING, ...) are the ground truth for
CPU behavior. See docs/TPE-AUTORUN.md for the full harness.

- Automated: `tools/tpe_autorun.py` drives the verifier end-to-end.
- Manual gotcha: sub-tests need `instr` -> `set-para,...` -> `run`. Typing a
  sub-command name directly executes an EMPTY test body and reports nothing.
- Before believing a failure: check binary freshness (`--version`) AND that
  the test floppy image is current. Both have produced phantom bugs.
- Never re-route fault handlers to satisfy a diagnostic's printed
  expected/found values - see docs/ND-DOMAIN-GOTCHAS.md (IIC table section).

## Boot smoke tests

```bash
make boot-smd      # SMD0_IMAGE=... to override
make boot-wd       # WD0_IMAGE=... to override
make boot-floppy   # FLOPPY_IMAGE=... to override
```
The targets check image existence before launching. Run them against scratch
copies (rule 1). A floppy that does not boot is usually a data floppy with no
FLOMON loader, not an emulator bug.

## WASM / glass UI changes

Do not declare a browser-facing fix done on build success alone - verify it
headlessly with puppeteer. Working examples at the repo root:

- verify-disasm-worker.js - worker mode + OPFS mount + segment disassembly
- test-gateway-browser.js - gateway / SharedArrayBuffer disk I/O
- test-hdd-manager-browser.js - HDD manager window

Puppeteer scripts MUST run from the repo root (local puppeteer dependency).
Typical cycle:
```bash
make wasm-glass
node verify-disasm-worker.js
```

## Symbol-table / SINTRAN inspector changes

```bash
node template-glass/js/tests/test_symbol_tables.js
```
Plus a live check on a booted scratch pack - reads taken before SINTRAN
finishes booting return garbage and must not be used as evidence.

## DAP / debugger changes

libdap lives primarily in a separate repo (see CLAUDE.md); change it there,
then bump the submodule. Verify with a real attach on a non-default port:
```bash
build/bin/nd100x --boot=smd --smd0=/tmp/scratch-SMD0.IMG --debugger --port=6661
```
Flow is connect -> attach -> continue. Remember the DAP single-step tax
(~33x): never benchmark timing-sensitive behavior (RTC, throttle) with the
debugger attached.

## Runnable deliverables (Makefile targets, scripts)

Anything whose purpose is to launch or drive the emulator must be executed
once before being reported as working - source-reading is not verification.
Use scratch copies of images so originals are never touched.
