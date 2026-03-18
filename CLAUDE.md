# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ND100X is a ND-100/CX minicomputer emulator written in C, forked from the nd100em project. It emulates a complete ND-100 system including CPU, memory management, and peripherals. The project supports multiple build targets including native Linux/Windows builds, WebAssembly for browser deployment, and RISC-V cross-compilation.

## Build Commands

### Standard Build Commands (using Makefile wrapper)
```bash
# Build debug version (default)
make debug

# Build release version
make release

# Build with address sanitizer for debugging
make sanitize

# Build WebAssembly version (standard UI, no debugger)
make wasm

# Build WebAssembly version (glassmorphism UI, with debugger)
make wasm-glass

# Build and serve glassmorphism WASM version on localhost:8000
make wasm-glass-run

# Build RISC-V version (requires specific toolchain)
make riscv

# Build with DAP debugger tools
make dap-tools

# Clean all build directories
make clean

# Build and run with default options
make run

# Run with valgrind for memory debugging
make runv

# Build and run unit tests
make test

# Build RetroTermWeb from submodule
make retroterm-build

# Show all available options
make help
```

### Build Dependencies
- **Core**: cmake, gcc, mkptypes (auto-built)
- **Floppy Menu**: libcurl4-openssl-dev, libncurses5-dev
- **WebAssembly**: Emscripten SDK
- **RISC-V**: riscv64-unknown-linux-musl toolchain

Install on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install libcurl4-openssl-dev libncurses5-dev
```

### Direct CMake Commands
```bash
# Standard native build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# WebAssembly build
mkdir -p build_wasm
emcmake cmake -B build_wasm -S . -DBUILD_WASM=ON
cmake --build build_wasm

# RISC-V build (requires riscv64-unknown-linux-musl toolchain)
mkdir -p build_riscv
cmake -B build_riscv -S . -DBUILD_RISCV=ON -DCMAKE_TOOLCHAIN_FILE=riscv64-toolchain.cmake
cmake --build build_riscv
```

### Running the Emulator
```bash
# Boot from SMD disk (default)
build/bin/nd100x --boot=smd

# Boot from floppy
build/bin/nd100x --boot=floppy --image=FLOPPY.IMG

# Load a.out file with debugger
build/bin/nd100x --boot=aout --image=program.out --debugger --verbose

# Load BPUN files (boot program unprotected)
build/bin/nd100x --boot=bpun --image=images/FILSYS-INV-Q04.BPUN

# Boot with paper tape loaded
build/bin/nd100x --boot=floppy --tape=images/test.bpun

# Boot with telnet server for remote terminal access
build/bin/nd100x --boot=smd --telnet=9000

# Run with various options using Makefile
BOOT_TYPE=floppy IMAGE_FILE=disk.img VERBOSE=1 DEBUGGER=1 make run
```

### Command Line Options
- `-b, --boot=TYPE`: Boot type (bp, bpun, aout, floppy, smd)
- `-i, --image=FILE`: Image file to load
- `-e, --tape=FILE`: Paper tape file to load into reader
- `-P, --printdir=DIR`: Directory for line printer output files (default: ./prints)
- `-D, --tapedir=DIR`: Directory for paper tape punch output files (default: ./tapes)
- `-s, --start=ADDR`: Start address (default: 0)
- `-a, --disasm`: Enable disassembly output
- `-d, --debugger`: Enable DAP debugger
- `-p, --port=PORT`: Set debugger port (default: 4711)
- `-S, --smd-debug`: Enable SMD disk controller debug log (stderr)
- `-t, --trace`: Enable CPU execution trace to stderr
- `-n, --max-instr=N`: Stop after N instructions
- `-B, --breakpoint=ADDR`: Stop at address (octal/hex/decimal)
- `-T, --text-start=ADDR`: Text segment load address for a.out (default: 0)
- `-r, --printer=TYPE`: Printer emulation type: text (default), escp, laser (not yet implemented)
- `-f, --printformat=FMT`: Printer output format: txt (default), pdf
- `-N, --telnet[=PORT]`: Enable telnet server (default port: 9000)
- `-v, --verbose`: Enable verbose output
- `-h, --help`: Show help message

### Block Devices
- **Floppy**: Uses FLOPPY.IMG (other images can be mounted via F12 menu)
- **SMD**: Uses SMD0.IMG, SMD1.IMG, SMD2.IMG, SMD3.IMG

### Character Devices
- **Paper Tape Reader**: I/O 0400-0403, loads BPUN files via `--tape` option
- **Paper Tape Punch**: I/O 0410-0413, output saved to `--tapedir` directory
- **Line Printer**: I/O 0430-0433, output saved to `--printdir` directory

### Floppy Menu and Virtual Screen Switching (Native)
Press **F12** during emulation to open the unified menu, which offers both the Floppy Database Browser (requires internet connection) and the Virtual Screen Selector. Press **Alt+1** through **Alt+6** to switch directly between virtual screens (Console, terminals, Line Printer, Paper Tape Punch). Device screens are output-only.

## Architecture Overview

### Core Module Structure
The project follows a modular architecture where each component is built as a static library:

- **ndlib**: Core utility library providing logging, file loading (BPUN/a.out formats), keyboard input, and download functionality
- **cpu**: CPU emulation with full ND-100 instruction set, memory management (MMS1/MMS2), floating point operations, and breakpoint support
- **machine**: Main emulation loop, system state management, and hardware integration
- **devices**: I/O device implementations (console, floppy, SMD disk, real-time clock, line printer, paper tape reader/punch)
- **debugger**: DAP (Debug Adapter Protocol) integration for debugging support
- **frontend**: User interface layers (nd100x native, nd100wasm for browser)

### Key Dependencies
- **external/cJSON**: JSON parsing library (included as submodule)
- **external/libdap**: Debug Adapter Protocol implementation (Linux/Windows only)
- **external/libsymbols**: Symbol table handling for a.out format support
- **mkptypes**: Automatic function prototype generation tool (built during compilation)

### HDLC Development
The project includes HDLC (High-Level Data Link Control) support on the `hdlc` branch:
- **src/devices/hdlc/**: Complete HDLC controller implementation including COM5025 chip emulation
- **HDLC Features**: CRC calculation, register management, interrupt handling

### CPU Architecture
- **Registers**: 16 registers x 16 runlevels (interrupt priority levels), accessed via `gReg->reg[level][regIndex]`
- **Register macros** (`src/cpu/cpu_types.h`): `gPC`, `gA`, `gD`, `gB`, `gT`, `gX`, `gL` - all reference current PIL
- **CPURunMode enum**: 0=`CPU_UNKNOWN_STATE`, 1=`CPU_RUNNING`, 2=`CPU_BREAKPOINT`, 3=`CPU_PAUSED`, 4=`CPU_STOPPED`, 5=`CPU_SHUTDOWN`
- **STS register**: MSB shared across all levels (`reg_STS`), LSB per-level (`reg[level][_STS]`)
- **MMS types**: MMS1 (4 page tables, 16-bit PTE) vs MMS2 (16 page tables, 32-bit PTE, required for VSX)

### Platform-Specific Features
- **Linux/Windows**: Full debugger support with WITH_DEBUGGER flag, libcurl for floppy database downloads, ncurses for terminal UI
- **WebAssembly (standard)**: `make wasm` - basic UI, debugger OFF, uses MEMFS virtual filesystem, xterm.js terminal
- **WebAssembly (Glass UI)**: `make wasm-glass` - glassmorphism UI, debugger ON (`DEBUGGER_ENABLED=ON`), 74 exported Dbg_* functions, SINTRAN OS inspection tools, draggable windows, theme system
- **RISC-V**: Cross-compiled for RISC-V Linux, debugger disabled, static linking enabled

### WASM Frontend API
- **Init()**: Hardware initialization only (CPU, memory, devices)
- **Boot(type)**: Load boot sector - 0=floppy, 1=smd, 2=bpun
- Init and Boot are separated so the UI can configure settings between hardware init and boot
- 74 exported `Dbg_*` functions provide full debugger access from JavaScript

### Build System Architecture
The project uses CMake with a Makefile wrapper for convenience. Each module has its own CMakeLists.txt with:
- Automatic prototype generation using mkptypes tool
- Platform-conditional compilation flags
- Proper target-based dependency management
- Out-of-source builds with organized output directories

### File Organization
- **src/**: All source code organized by functionality
- **build*/**: Multiple build directories for different targets
- **docs/**: Technical documentation and build guides
- **images/**: Binary disk images and system files (SMD0.IMG, FLOPPY.IMG)
- **asm/**: Assembly code samples and development tools
- **template/**: Basic web template for standard WASM build
- **template-glass/**: Glassmorphism UI (1 HTML, 2 CSS, 23 JS modules, 1 JSON data file) - includes draggable debug windows, SINTRAN OS inspection, theme system, CPU load graph, page table viewer, printer and paper tape windows
- **tests/**: Unit tests for printer, PDF writer, ESC/P parser, and print job manager

## Development Workflow

### Auto-Generated Files - DO NOT EDIT
Files matching `*_protos.h` are auto-generated by mkptypes during build. Manual edits will be overwritten.

### Adding New Features
1. Identify the appropriate module (cpu, devices, machine, etc.)
2. Update the module's CMakeLists.txt if adding new source files
3. Follow existing code patterns and naming conventions
4. Test on multiple platforms if the feature affects core functionality

### Debugging
- Use `make sanitize` for memory debugging with AddressSanitizer
- Enable verbose logging with `--verbose` flag
- DAP debugger available on Linux/Windows builds with `--debugger` flag
- WebAssembly builds can use browser dev tools for debugging

### Cross-Platform Considerations
- WASM builds cannot access filesystem directly - use MEMFS
- RISC-V builds require static linking and cross-compiler toolchain
- DAP debugger (socket-based) available on Linux/Windows; WASM Glass UI has its own JS-based debugger via exported Dbg_* functions
- Floppy menu functionality requires network access (disabled on WASM/RISC-V)

### Documentation Files
- **SINTRAN-Commands.html**: Auto-generated help documentation file - DO NOT modify manually
- **dap-analysis.md**: Comprehensive analysis of DAP debugger implementation - excellent reference for understanding debugger architecture
- **GLASS.md**: Comprehensive architecture reference for the glassmorphism browser UI (template-glass/) - module reference, window management, boot sequence, emulation loop, SINTRAN inspectors, theme system, and contributor guide
- **MERMAID_COLOR_STANDARDS.md**: WCAG 2.1 Level AA color standards for all Mermaid diagrams in this project - includes primary/light palettes, semantic assignments, classDef copy-paste blocks, and colorblind accessibility guidelines
- HTML structure may contain duplicate IDs which should be handled in JavaScript, not fixed in the HTML

### Git Submodules
Update submodules when external dependencies change:
```bash
git submodule update --init --recursive
```

### Test Programs and Images
- **Assembly development**: Use [nd100-as](https://github.com/ragge0/norsk_data/tree/main/nd100-as) for AT&T syntax assembly
- **End programs with**: `opcom` instruction to stop emulator execution
- **Available images**: Located in `images/` directory (BPUN files, floppy images)
- **Test files**: test.bpun and other BPUN files available for testing CPU functionality
- **Floppy database**: https://ndlib.hackercorp.no/

## Testing and Validation

### Unit Tests
The project has unit tests for the printer/PDF subsystem, run via CTest:
```bash
make test    # Build and run all tests
```
Test source files are in `tests/` (test_escp.c, test_pdfwriter.c, test_printjob.c).

### Manual Validation
Additional validation is done through:
- Running BPUN test programs (like test.bpun)
- Booting SINTRAN and verifying system functionality
- Using DAP debugger for step-by-step validation
- Manual testing with various disk images and assembly programs

### Debugging Tools
- **Valgrind support**: Use `make runv` for memory debugging
- **AddressSanitizer**: Use `make sanitize` for build-time memory checking
- **DAP debugger**: Full debugging support with breakpoints, step execution, memory inspection
- **Verbose logging**: Enable with `--verbose` flag for detailed execution tracing