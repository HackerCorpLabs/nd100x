# ND100X Project

## Build System Overview

This project uses CMake as its build system. The structure is modular, with core libraries and frontends built separately.

### Project Structure

- `ndlib/` - Core library for common functionality
- `cpu/` - CPU emulation module
- `devices/` - Device emulation modules
- `machine/` - Machine integration module
- `debugger/` - DAP Debugging support (Linux/Windows only)
- `frontend/nd100x/` - Native frontend
- `frontend/nd100wasm/` - WebAssembly frontend

### Building the Project

#### Prerequisites

- CMake 3.14 or higher
- C/C++ compiler (gcc, clang, MSVC)
- For WebAssembly builds: Emscripten SDK 
    - See [How to build WASM document](HOWTO_BUILD_WASM.md)

- cJSON library
  - apt install libcjson-dev

#### Building on Linux/macOS

```bash
# Configure and build in debug mode
./build.sh

# Build in release mode
./build.sh --release

# Build WebAssembly version
./build.sh --wasm

# Specify custom build directory
./build.sh --build-dir my_build

# Specify custom installation prefix
./build.sh --install-prefix ~/local
```

#### Building on Windows

```batch
# Configure and build in debug mode
build.bat

# Build in release mode
build.bat --release

# Build WebAssembly version
build.bat --wasm

# Specify custom build directory
build.bat --build-dir my_build

# Specify custom installation prefix
build.bat --install-prefix "C:\Program Files\ND100X"
```

#### Manual CMake Build

```bash
# Create a build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Install
cmake --install .
```

### CMake Structure

The project uses a modern CMake approach with proper target-based dependency management:

1. Each module is built as a standalone library
2. Dependencies are expressed using `target_link_libraries`
3. Include directories are properly scoped with PUBLIC/PRIVATE visibility
4. Platform-specific code is conditionally included
5. WebAssembly builds use Emscripten-specific configurations

### Debugger Support

The debugger is only available on Linux and Windows builds. It uses the DAP (Debug Adapter Protocol) from `external/libdap`. The preprocessor symbol `WITH_DEBUGGER` is defined when the debugger is available.

### Out-of-Source Builds

The build system follows CMake best practices for out-of-source builds:

- All build artifacts are placed in the build directory
- No hardcoded paths to build directories in the source code
- Library and executable outputs use standard CMake variables
- Installation paths are configurable 

# Building ND100X

This project provides two build systems:

1. Original Makefile-based build system
2. New CMake-based build system

## Makefile-based Build System
## Using Make

A Makefile is included to simplify the process if you prefer using make.



### Prerequisites
- Linux (tested on Ubuntu 22.04) under WSL
- GCC compiler (tested with GCC 11.4.0)
- Make build system
- mkptypes tool (automatically built during compilation)

### Building with Make

Use "make help" to list the options

```bash

$make help

ND100X Makefile Help
-------------------------------------------------------------------------------
Targets:
  all (default) - Same as 'debug'
  debug         - Build debug version
  release       - Build release version
  sanitize      - Build with address sanitizer
  wasm          - Build WebAssembly version
                  (Requires Emscripten SDK, run in browser via HTTP server)
  dap-tools     - Build with DAP tools (dap_debugger and dap_mock_server)
                  (Only available on Linux/Windows with libdap)
  clean         - Remove build directories
  install       - Install the build
  run           - Build and run nd100x (uses defaults below)
  help          - Show this help

Run options (environment variables):
  BOOT_TYPE=smd|floppy|bpun|aout|bp   Boot type (default: smd)
  IMAGE_FILE=path                     Image file to load
  START_ADDR=0                        Start address (default: 0)
  VERBOSE=1                           Enable verbose output
  DEBUGGER=1                          Enable DAP debugger
  DISASM=1                            Enable disassembly output

Platform-specific features:
  Linux/Windows: _DEBUGGER_ENABLED_ is automatically set for cpu/debugger
                 This enables Debug Adapter Protocol (DAP) support
  WebAssembly:   Debugger is disabled, optimized for browser performance

WebAssembly options:
  The wasm target builds a browser-compatible version of the emulator.
  After building, serve the build_wasm/bin directory via HTTP
  and open index.html in a browser.

Example:
  BOOT_TYPE=floppy IMAGE_FILE=FLOPPY.IMG make run

This Makefile is a wrapper around CMake. If you prefer, you can use CMake directly:
  ./build.sh          - For more build options  
```




1. For debug build (default):
```bash
make debug
```

2. For release build:
```bash
make release
```

3. For build with sanitizers:
```bash
make sanitize
```

The build system supports three different build types:
- `debug`: Includes debug symbols and disables optimizations
- `release`: Enables high-level optimizations for best performance
- `sanitize`: Includes Address Sanitizer for debugging memory issues

## CMake-based Build System

### Prerequisites
- CMake 3.14 or higher
- C/C++ compiler (gcc, clang, MSVC)
- For WebAssembly builds: Emscripten SDK

### Building with CMake

1. Create a build directory:
```bash
mkdir -p build
```

2. Configure the project:
```bash
cd build
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

4. Install (optional):
```bash
cmake --install .
# Or to specify an installation prefix:
cmake --install . --prefix /path/to/install
```

### Build Script

For convenience, a build script is provided that handles common build configurations:

```bash
# Configure and build in debug mode
./build.sh

# Build in release mode
./build.sh --release

# Build WebAssembly version
./build.sh --wasm

# Specify custom build directory
./build.sh --build-dir my_build

# Specify custom installation prefix
./build.sh --install-prefix ~/local

# Build with DAP test tools (dap_debugger, dap_mock_server)
./build.sh --with-dap-tools
```

### Building on Windows

On Windows, you can use the provided batch file:

```batch
# Configure and build in debug mode
build.bat

# Build in release mode
build.bat --release

# Specify custom build directory
build.bat --build-dir my_build

# Specify custom installation prefix
build.bat --install-prefix "C:\Program Files\ND100X"

# Build with DAP test tools
build.bat --with-dap-tools
```

### Building for WebAssembly

To build for WebAssembly, you need to have Emscripten SDK installed and activated:

```bash
# Using emcmake directly
mkdir -p build_wasm
emcmake cmake -B build_wasm -S .
cmake --build build_wasm

# Or using the build script
./build.sh --wasm
```

### Building DAP Test Tools

By default, the DAP (Debug Adapter Protocol) test tools (`dap_debugger` and `dap_mock_server`) are not built to reduce compilation time and dependencies. If you need these tools for development or testing, you can enable them:

```bash
# Using CMake directly
cmake -B build -S . -DBUILD_DAP_TOOLS=ON
cmake --build build

# Or using the build script
./build.sh --with-dap-tools
```

## CMake Structure

The project uses a modern CMake approach:

1. Each module is built as a standalone library
2. Dependencies are expressed using `target_link_libraries`
3. Include directories are properly scoped with PUBLIC/PRIVATE visibility
4. Platform-specific code is conditionally included
5. WebAssembly builds use Emscripten-specific configurations

### Debugger Support

The debugger is only available on Linux and Windows builds. It uses the DAP (Debug Adapter Protocol) from `external/libdap`. The preprocessor symbol `WITH_DEBUGGER` is defined when the debugger is available.

### CMake Generated Files

The build system respects the existing code structure and uses the `mkptypes` tool to generate prototype headers, just like the original Makefile system. 