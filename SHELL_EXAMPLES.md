# ND-100X Interactive Shell - Usage Examples

**Files included:**
- `example-shell-config.ini` — INI configuration template
- `example-shell-commands.sh` — Script with sample commands

---

## Quick Start: 3 Ways to Run

### Method 1: CLI Flags (Simplest)

```bash
cd /home/ronny/repos/nd100x

# Point to your BPUN files
./build/bin/nd100x --monitor --nd100-root=/mnd/d/nd/bpun
```

Then in the shell:
```
@ LIST-FILES
@ LIST-FILES *.bpun
@ RUN-PROGRAM <filename>.bpun
@ SHOW-REGISTERS
@ EXIT
```

### Method 2: INI File (Configured)

```bash
cd /home/ronny/repos/nd100x

# Use the example config (edit nd100_root path if needed)
./build/bin/nd100x --config=example-shell-config.ini
```

**Edit first:**
```bash
# Change the path in the INI file
nano example-shell-config.ini
# Find line: nd100_root = /mnd/d/nd/bpun
# Change to your path if needed
```

### Method 3: Batch Script (Automation)

```bash
cd /home/ronny/repos/nd100x

# Run commands automatically from script
./build/bin/nd100x --monitor \
  --nd100-root=/mnd/d/nd/bpun \
  --script=example-shell-commands.sh
```

**Edit script first:**
```bash
# Uncomment or edit commands in the script
nano example-shell-commands.sh
```

---

## Working with `/mnd/d/nd/bpun`

**Yes, you can absolutely use any path!**

```bash
# Point to your exact directory
./build/bin/nd100x --monitor --nd100-root=/mnd/d/nd/bpun

# In the shell:
@ LIST-FILES
  (shows all files in /mnd/d/nd/bpun)

@ LIST-FILES *.bpun
  (shows only .bpun files)

@ RUN-PROGRAM <filename>.bpun
  (loads from /mnd/d/nd/bpun/<filename>.bpun)
```

**Path notes:**
- ✅ Absolute paths: `/mnt/data/bpun`, `/mnd/d/nd/bpun`
- ✅ Relative paths: `./examples`, `../bpun-files`
- ✅ Home paths: `~/nd100-files`
- ✅ Current dir: `.` (default if omitted)
- ✅ Paths with spaces: `/path/to/My Files/bpun`

---

## Complete Example Workflow

### Step 1: Build (if not already built)

```bash
cd /home/ronny/repos/nd100x
mkdir -p build
cd build
cmake ..
make -j4
```

### Step 2: Launch Shell

```bash
cd /home/ronny/repos/nd100x

# Method A: Direct CLI
./build/bin/nd100x --monitor --nd100-root=/mnd/d/nd/bpun
```

### Step 3: Use the Shell

```
@ HELP
ND-100 Interactive Shell - Available Commands:

  HELP                  Show this help message
      (abbrev: HE)
  LIST-FILES            List BPUN/PROG files (supports glob: *.bpun)
      (abbrev: LI-FI)
  RUN-PROGRAM           Load and run a BPUN file
      (abbrev: RU-PR)
  SHOW-REGISTERS        Display CPU state (PC, registers, STS)
      (abbrev: SH-RE)
  EXIT                  Exit the shell
      (abbrev: EX)

Note: Commands are case-insensitive and support abbreviation.
      File names use host extensions (.bpun, .prog, etc)

@ LIST-FILES
Files in /mnd/d/nd/bpun matching '*':
  kernel.bpun
  hello.bpun
  test.prog
  diag.bpun

@ LI-FI *.bpun
Files in /mnd/d/nd/bpun matching '*.bpun':
  kernel.bpun
  hello.bpun
  diag.bpun

@ RUN-PROGRAM hello.bpun
Loading /mnd/d/nd/bpun/hello.bpun...
Program loaded at entry point: 0o001000
(CPU execution not yet integrated with shell)

@ SH-RE
CPU Registers:
  A:     0o000000
  B:     0o000000
  D:     0o000000
  X:     0o000000
  L:     0o000000
  T:     0o000000
  P:     0o001000
  STS:   0o000000

@ EXIT
Exiting shell.
Shell exited.
```

---

## Example: Custom INI File

**File: `/tmp/my-nd100-shell.ini`**

```ini
[cpu]
type = 100

[boot]
device = smd.0.0

[runtime]
shell = on
nd100_root = /mnd/d/nd/bpun
# Optional: auto-run script on startup
# script = /tmp/commands.sh
memory = 4
```

**Run:**
```bash
./build/bin/nd100x --config=/tmp/my-nd100-shell.ini
```

---

## Example: Batch Automation Script

**File: `/tmp/test-all-bpun.sh`**

```bash
# List what we're testing
LIST-FILES *.bpun

# Test kernel
RUN-PROGRAM kernel.bpun
SHOW-REGISTERS

# Test hello world
RUN-PROGRAM hello.bpun
SHOW-REGISTERS

# Done
EXIT
```

**Run:**
```bash
./build/bin/nd100x --monitor \
  --nd100-root=/mnd/d/nd/bpun \
  --script=/tmp/test-all-bpun.sh
```

---

## Troubleshooting

### Error: "Cannot open directory: /mnd/d/nd/bpun"

**Solutions:**
```bash
# Check path exists and is readable
ls -la /mnd/d/nd/bpun

# If path doesn't exist, try alternative:
./build/bin/nd100x --monitor --nd100-root=./examples

# Create test directory if needed
mkdir -p /mnd/d/nd/bpun
cp some-file.bpun /mnd/d/nd/bpun/
```

### No files show up in LIST-FILES

**Check:**
```bash
# Verify files exist with correct extension
ls /mnd/d/nd/bpun/*.bpun

# Check if shell is looking in right place
@ LIST-FILES
# Should show path in output like:
# Files in /mnd/d/nd/bpun matching '*':
```

### Command not recognized

**Help:**
```bash
@ HELP
# Shows all valid commands and abbreviations

@ LI    # Abbreviated LIST-FILES - works
@ LISTFILES  # Wrong - needs hyphen
@ LIST-FILES  # Correct - full name
```

---

## File Locations (Created Files)

**Example files:**
- `/home/ronny/repos/nd100x/example-shell-config.ini`
- `/home/ronny/repos/nd100x/example-shell-commands.sh`
- `/home/ronny/repos/nd100x/SHELL_EXAMPLES.md` ← You are here

**Quick Start Guide:**
- `/home/ronny/repos/nd100x/docs/ND100X_SHELL_QUICKSTART.md`

**Shell Implementation:**
- `/home/ronny/repos/nd100x/src/frontend/nd100x/nd100x_shell.c` (470 lines)
- `/home/ronny/repos/nd100x/src/frontend/nd100x/nd100x_shell.h` (Public API)

---

## Next Steps

1. **Build:** `make -j4` in build/ directory (if not done)
2. **Test:** Run with your path: `./build/bin/nd100x --monitor --nd100-root=/mnd/d/nd/bpun`
3. **Explore:** Try LIST-FILES, RUN-PROGRAM, SHOW-REGISTERS
4. **Automate:** Create scripts to batch-test your BPUN files
5. **Report:** Any issues or missing features

---

**Ready to go! Your path `/mnd/d/nd/bpun` is fully supported.**
