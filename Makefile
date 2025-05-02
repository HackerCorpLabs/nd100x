# ND100X Makefile - CMake wrapper
# This Makefile provides backward compatibility by calling CMake

# Build directories
BUILD_DIR = build
BUILD_DIR_DEBUG = build
BUILD_DIR_RELEASE = build_release
BUILD_DIR_SANITIZE = build_sanitize
BUILD_DIR_WASM = build_wasm

# Commands with full paths
CMAKE := $(shell command -v cmake 2>/dev/null)
EMCMAKE := $(shell command -v emcmake 2>/dev/null)

# Default target is debug
.PHONY: all
all: debug

# Check for required dependencies
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@command -v cmake >/dev/null 2>&1 || { echo "Error: cmake not found. Please install cmake."; exit 1; }
	@command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Please install gcc."; exit 1; }
	@test -e /usr/include/cjson/cJSON.h || { echo "Warning: cJSON development headers not found. Please install libcjson-dev."; }
	@echo "All core dependencies found."

# Build the mkptypes tool
.PHONY: mkptypes
mkptypes:
	@echo "Building mkptypes tool..."
	$(MAKE) -C tools/mkptypes

# Build targets
.PHONY: debug release sanitize wasm clean install run help

debug: check-deps mkptypes
	@echo "Building debug version..."
	$(CMAKE) -B $(BUILD_DIR_DEBUG) -S . -DCMAKE_BUILD_TYPE=Debug
	$(CMAKE) --build $(BUILD_DIR_DEBUG) -- -j$$(nproc 2>/dev/null || echo 4)

release: check-deps mkptypes
	@echo "Building release version..."
	$(CMAKE) -B $(BUILD_DIR_RELEASE) -S . -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_DIR_RELEASE) -- -j$$(nproc 2>/dev/null || echo 4)

sanitize: check-deps mkptypes
	@echo "Building with sanitizers..."
	$(CMAKE) -B $(BUILD_DIR_SANITIZE) -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
	$(CMAKE) --build $(BUILD_DIR_SANITIZE) -- -j$$(nproc 2>/dev/null || echo 4)

wasm: check-deps mkptypes
	@echo "Building WebAssembly version..."
	@command -v emcmake >/dev/null 2>&1 || { echo "Error: emcmake not found. Please install and activate Emscripten SDK."; exit 1; }
	$(EMCMAKE) $(CMAKE) -B $(BUILD_DIR_WASM) -S . -DBUILD_WASM=ON
	$(CMAKE) --build $(BUILD_DIR_WASM) -- -j$$(nproc 2>/dev/null || echo 4)
	@echo "Copying index.html template to WASM build directory..."
	@mkdir -p $(BUILD_DIR_WASM)/bin
	@cp template/index.html $(BUILD_DIR_WASM)/bin/
	@echo ""
	@echo "WebAssembly build completed successfully!"
	@echo "To test in a browser, serve the build directory via HTTP:"
	@echo "  cd $(BUILD_DIR_WASM)/bin"
	@echo "  python3 -m http.server"
	@echo "Then open http://localhost:8000/index.html in your browser."

dap-tools: check-deps mkptypes
	@echo "Building with DAP tools..."
	$(CMAKE) -B $(BUILD_DIR) -S . -DBUILD_DAP_TOOLS=ON
	$(CMAKE) --build $(BUILD_DIR) -- -j$$(nproc 2>/dev/null || echo 4)

clean:
	@echo "Cleaning build directories..."
	rm -rf $(BUILD_DIR) $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) $(BUILD_DIR_SANITIZE) $(BUILD_DIR_WASM)

install:
	@echo "Installing..."
	$(CMAKE) --install $(BUILD_DIR)

# Default run arguments - you can override these by setting environment variables
# e.g., BOOT_TYPE=floppy make run
BOOT_TYPE ?= smd
IMAGE_FILE ?=
START_ADDR ?= 0
VERBOSE ?= 0
DEBUGGER ?= 0
DISASM ?= 0

run: debug
	@echo "Running nd100x..."
	@if [ -n "$(IMAGE_FILE)" ]; then \
		IMAGE_ARG="--image=$(IMAGE_FILE)"; \
	else \
		IMAGE_ARG=""; \
	fi; \
	VERBOSE_ARG=$$([ "$(VERBOSE)" = "1" ] && echo "--verbose" || echo ""); \
	DEBUGGER_ARG=$$([ "$(DEBUGGER)" = "1" ] && echo "--debugger" || echo ""); \
	DISASM_ARG=$$([ "$(DISASM)" = "1" ] && echo "--disasm" || echo ""); \
	$(BUILD_DIR)/bin/nd100x --boot=$(BOOT_TYPE) $$IMAGE_ARG --start=$(START_ADDR) $$VERBOSE_ARG $$DEBUGGER_ARG $$DISASM_ARG

help:
	@echo "ND100X Makefile Help"
	@echo "-------------------------------------------------------------------------------"
	@echo "Targets:"
	@echo "  all (default) - Same as 'debug'"
	@echo "  debug         - Build debug version"
	@echo "  release       - Build release version"
	@echo "  sanitize      - Build with address sanitizer"
	@echo "  wasm          - Build WebAssembly version"
	@echo "                  (Requires Emscripten SDK, run in browser via HTTP server)"
	@echo "  dap-tools     - Build with DAP tools (dap_debugger and dap_mock_server)"
	@echo "                  (Only available on Linux/Windows with libdap)"
	@echo "  clean         - Remove build directories"
	@echo "  install       - Install the build"
	@echo "  run           - Build and run nd100x (uses defaults below)"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Run options (environment variables):"
	@echo "  BOOT_TYPE=smd|floppy|bpun|aout|bp   Boot type (default: smd)"
	@echo "  IMAGE_FILE=path                     Image file to load"
	@echo "  START_ADDR=0                        Start address (default: 0)"
	@echo "  VERBOSE=1                           Enable verbose output"
	@echo "  DEBUGGER=1                          Enable DAP debugger"
	@echo "  DISASM=1                            Enable disassembly output"
	@echo ""
	@echo "Platform-specific features:"
	@echo "  Linux/Windows: _DEBUGGER_ENABLED_ is automatically set for cpu/debugger"
	@echo "                 This enables Debug Adapter Protocol (DAP) support"
	@echo "  WebAssembly:   Debugger is disabled, optimized for browser performance"
	@echo ""
	@echo "WebAssembly options:"
	@echo "  The wasm target builds a browser-compatible version of the emulator."
	@echo "  After building, serve the $(BUILD_DIR_WASM)/bin directory via HTTP"
	@echo "  and open index.html in a browser."
	@echo ""
	@echo "Example:"
	@echo "  BOOT_TYPE=floppy IMAGE_FILE=FLOPPY.IMG make run"
	@echo ""
	@echo "This Makefile is a wrapper around CMake. If you prefer, you can use CMake directly:"
	@echo "  ./build.sh          - For more build options"
	@echo ""