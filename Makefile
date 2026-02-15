# ND100X Makefile - CMake wrapper
# This Makefile provides backward compatibility by calling CMake

# Build directories
BUILD_DIR = build
BUILD_DIR_DEBUG = build
BUILD_DIR_RELEASE = build_release
BUILD_DIR_SANITIZE = build_sanitize
BUILD_DIR_WASM = build_wasm
BUILD_DIR_WASM_GLASS = build_wasm_glass
BUILD_DIR_RISCV = build_riscv

# Commands with full paths
CMAKE = cmake
EMCMAKE = emcmake

# By default, enable debugger in Linux/Windows builds
DEBUGGER_ENABLED ?= ON

# Default target is debug
.PHONY: all
all: debug

# Check for required dependencies
.PHONY: check-deps
check-deps:
	@echo "Checking dependencies..."
	@command -v cmake >/dev/null 2>&1 || { echo "Error: cmake not found. Please install cmake."; exit 1; }
	@command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Please install gcc."; exit 1; }
	@echo "All core dependencies found."

# Check for RISC-V specific dependencies
.PHONY: check-riscv-deps
check-riscv-deps:
	@echo "Checking RISC-V dependencies..."
	@COMPILER_PATH="/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc"; \
	if [ -x "$$COMPILER_PATH" ]; then \
		echo "RISC-V compiler found at $$COMPILER_PATH"; \
		echo "Compiler version: $$("$$COMPILER_PATH" --version | head -n1)"; \
	else \
		echo "Error: RISC-V compiler not found at expected location: $$COMPILER_PATH"; \
		exit 1; \
	fi

# Build the mkptypes tool
.PHONY: mkptypes
mkptypes:
	@echo "Building mkptypes tool..."
	$(MAKE) -C tools/mkptypes

# Build targets
# Compile TypeScript terminal sources (if tsc available, else use pre-compiled JS)
.PHONY: ts-compile
ts-compile:
	@if npx tsc --version >/dev/null 2>&1; then \
		echo "Compiling TypeScript terminal modules..."; \
		npx tsc -p template-glass/ts/tsconfig.json; \
	else \
		echo "TypeScript not available, using pre-compiled JS files."; \
	fi

.PHONY: debug release sanitize wasm wasm-run wasm-glass wasm-glass-run riscv clean install run help gateway-install gateway gateway-test wasm-glass-gateway

debug: check-deps mkptypes
	@echo "Building debug version..."
	@mkdir -p $(BUILD_DIR_DEBUG)
	cd $(BUILD_DIR_DEBUG) && $(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug -DDEBUGGER_ENABLED=$(DEBUGGER_ENABLED)
	cd $(BUILD_DIR_DEBUG) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)

release: check-deps mkptypes
	@echo "Building release version..."
	@mkdir -p $(BUILD_DIR_RELEASE)
	cd $(BUILD_DIR_RELEASE) && $(CMAKE) .. -DCMAKE_BUILD_TYPE=Release -DDEBUGGER_ENABLED=$(DEBUGGER_ENABLED)
	cd $(BUILD_DIR_RELEASE) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)

sanitize: check-deps mkptypes
	@echo "Building with sanitizers..."
	@mkdir -p $(BUILD_DIR_SANITIZE)
	cd $(BUILD_DIR_SANITIZE) && $(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer" -DDEBUGGER_ENABLED=$(DEBUGGER_ENABLED)
	cd $(BUILD_DIR_SANITIZE) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)

wasm: check-deps mkptypes
	@echo "Building WebAssembly version..."
	@command -v emcmake >/dev/null 2>&1 || { echo "Error: emcmake not found. Please install and activate Emscripten SDK."; exit 1; }
	@mkdir -p $(BUILD_DIR_WASM)
	cd $(BUILD_DIR_WASM) && $(EMCMAKE) $(CMAKE) .. -DBUILD_WASM=ON -DDEBUGGER_ENABLED=OFF
	cd $(BUILD_DIR_WASM) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)
	@echo "Copying index.html template to WASM build directory..."
	@mkdir -p $(BUILD_DIR_WASM)/bin
	@cp template/index.html $(BUILD_DIR_WASM)/bin/
	@echo ""
	@echo "WebAssembly build completed successfully!"
	@echo "To test in a browser, serve the build directory via HTTP:"
	@echo "  cd $(BUILD_DIR_WASM)/bin"
	@echo "  python3 -m http.server"
	@echo "Then open http://localhost:8000/index.html in your browser."

wasm-glass: check-deps mkptypes ts-compile
	@echo "Building WebAssembly version (glassmorphism UI)..."
	@command -v emcmake >/dev/null 2>&1 || { echo "Error: emcmake not found. Please install and activate Emscripten SDK."; exit 1; }
	@mkdir -p $(BUILD_DIR_WASM_GLASS)
	cd $(BUILD_DIR_WASM_GLASS) && $(EMCMAKE) $(CMAKE) .. -DBUILD_WASM=ON -DDEBUGGER_ENABLED=ON
	cd $(BUILD_DIR_WASM_GLASS) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)
	@echo "Copying glassmorphism UI and shared assets to build directory..."
	@mkdir -p $(BUILD_DIR_WASM_GLASS)/bin
	@cp template-glass/index.html $(BUILD_DIR_WASM_GLASS)/bin/index.html
	@cp template-glass/terminal-popout.html $(BUILD_DIR_WASM_GLASS)/bin/terminal-popout.html 2>/dev/null || true
	@cp -r template-glass/css $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp -r template-glass/js $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp -r template-glass/data $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp template/Logo_ND.png template/favicon.ico template/favicon.png $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp -r template/floppies $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp docs/SINTRAN-Commands.html $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@cp SMD0.IMG $(BUILD_DIR_WASM_GLASS)/bin/ 2>/dev/null || true
	@# Cache-bust: append ?v=TIMESTAMP to all local src/href in HTML files
	@BUILD_TS=$$(date +%s); \
	sed -i \
	  -e "s|src=\"js/|src=\"js/|g" \
	  -e "s|src=\"nd100wasm.js\"|src=\"nd100wasm.js?v=$$BUILD_TS\"|g" \
	  -e "s|href=\"css/styles.css\"|href=\"css/styles.css?v=$$BUILD_TS\"|g" \
	  -e "s|href=\"css/themes.css\"|href=\"css/themes.css?v=$$BUILD_TS\"|g" \
	  -e "s|src=\"js/\([^\"]*\)\"|src=\"js/\1?v=$$BUILD_TS\"|g" \
	  $(BUILD_DIR_WASM_GLASS)/bin/index.html; \
	sed -i \
	  -e "s|href=\"css/styles.css\"|href=\"css/styles.css?v=$$BUILD_TS\"|g" \
	  -e "s|href=\"css/themes.css\"|href=\"css/themes.css?v=$$BUILD_TS\"|g" \
	  -e "s|src=\"js/\([^\"]*\)\"|src=\"js/\1?v=$$BUILD_TS\"|g" \
	  $(BUILD_DIR_WASM_GLASS)/bin/terminal-popout.html; \
	echo "Cache-bust: v=$$BUILD_TS"
	@echo ""
	@echo "WebAssembly build (glassmorphism UI) completed successfully!"
	@echo "To test in a browser, serve the build directory via HTTP:"
	@echo "  cd $(BUILD_DIR_WASM_GLASS)/bin"
	@echo "  python3 -m http.server"
	@echo "Then open http://localhost:8000/index.html in your browser."

wasm-run: wasm
	@echo "Starting HTTP server for standard WASM build..."
	@echo "Open http://localhost:8000/index.html in your browser."
	cd $(BUILD_DIR_WASM)/bin && python3 -m http.server

wasm-glass-run: wasm-glass
	@echo "Starting HTTP server for glassmorphism WASM build..."
	@echo "Open http://localhost:8000/index.html in your browser."
	cd $(BUILD_DIR_WASM_GLASS)/bin && python3 -m http.server

riscv: check-riscv-deps mkptypes
	@echo "Building RISC-V Linux version with DAP support..."
	@mkdir -p $(BUILD_DIR_RISCV)
	
	@# Set PATH to include the RISC-V compiler and disable DAP client tools
	cd $(BUILD_DIR_RISCV) && PATH="$$PATH:/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin" \
	$(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_RISCV=ON \
		-DCMAKE_TOOLCHAIN_FILE=../riscv64-toolchain.cmake \
		-DBUILD_DAP_TOOLS=OFF -DENABLE_CJSON_TEST=OFF \
		-DCMAKE_CXX_COMPILER_WORKS=TRUE -DCMAKE_C_COMPILER_WORKS=TRUE
	
	@# Build with PATH set to include the RISC-V compiler
	cd $(BUILD_DIR_RISCV) && PATH="$$PATH:/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin" \
	$(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)
	
	@echo ""
	@if [ -f $(BUILD_DIR_RISCV)/bin/nd100x ]; then \
		echo "RISC-V build completed successfully!"; \
		echo "Executable information:"; \
		file $(BUILD_DIR_RISCV)/bin/nd100x; \
		if file $(BUILD_DIR_RISCV)/bin/nd100x | grep -q "RISC-V"; then \
			echo "✅ Successfully built RISC-V executable with debug symbols!"; \
		else \
			echo "❌ Warning: The executable doesn't appear to be a RISC-V binary."; \
		fi; \
	else \
		echo "❌ Build failed. Check the error messages above."; \
	fi

dap-tools: check-deps mkptypes
	@echo "Building with DAP tools..."
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) .. -DBUILD_DAP_TOOLS=ON -DDEBUGGER_ENABLED=ON
	cd $(BUILD_DIR) && $(CMAKE) --build . -- -j$$(nproc 2>/dev/null || echo 4)

clean:
	@echo "Cleaning build directories..."
	rm -rf $(BUILD_DIR) $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) $(BUILD_DIR_SANITIZE) $(BUILD_DIR_WASM) $(BUILD_DIR_WASM_GLASS) $(BUILD_DIR_RISCV)


update-libdap:
	cd external/libdap && git fetch && git checkout origin/main
	git add external/libdap
	git commit -m "Update libdap submodule to latest commit"

	
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


runv: debug
	@echo "Running with valgrind.."
	valgrind --leak-check=full  $(BUILD_DIR)/bin/nd100x -d -v

gateway-install:
	@echo "Installing gateway dependencies..."
	cd tools/nd100-gateway && npm install

gateway: gateway-install
	@echo "Starting ND-100 Terminal Gateway..."
	node tools/nd100-gateway/gateway.js $(GATEWAY_ARGS)

gateway-test: gateway-install
	@echo "Running gateway tests..."
	node tools/nd100-gateway/test-gateway.js

wasm-glass-gateway: wasm-glass gateway-install
	@echo "Starting glassmorphism WASM build + Terminal Gateway..."
	@echo "Open http://localhost:8000/index.html?worker=1 in your browser."
	@echo ""
	node tools/nd100-gateway/gateway.js &
	cd $(BUILD_DIR_WASM_GLASS)/bin && python3 -m http.server


help:
	@echo "ND100X Makefile Help"
	@echo "-------------------------------------------------------------------------------"
	@echo "Targets:"
	@echo "  all (default) - Same as 'debug'"
	@echo "  debug         - Build debug version"
	@echo "  release       - Build release version"
	@echo "  sanitize      - Build with address sanitizer"
	@echo "  wasm          - Build WebAssembly version (standard UI)"
	@echo "  wasm-run      - Build and serve standard WASM version"
	@echo "  wasm-glass    - Build WebAssembly version (glassmorphism UI)"
	@echo "  wasm-glass-run - Build and serve glassmorphism WASM version"
	@echo "  riscv         - Build RISC-V Linux version with DAP support"
	@echo "                  (Requires compiler at /home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin)"
	@echo "  dap-tools     - Build with DAP tools (dap_debugger and dap_mock_server)"
	@echo "                  (Requires libdap)"
	@echo "  gateway-install - Install gateway server dependencies (npm)"
	@echo "  gateway       - Start the terminal gateway server"
	@echo "  gateway-test  - Run gateway unit tests (14 tests)"
	@echo "  wasm-glass-gateway - Build glass UI + start gateway + HTTP server"
	@echo "  clean         - Remove build directories"
	@echo "  install       - Install the build"
	@echo "  run           - Build and run nd100x (uses defaults below)"
	@echo "  runv          - Build and run with valgrind"
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
	@echo "Build options (environment variables):"
	@echo "  DEBUGGER_ENABLED=ON|OFF             Enable/disable debugger support in build"
	@echo "                                      (Default: ON for most builds, OFF for WASM)"
	@echo ""
	@echo "WebAssembly options:"
	@echo "  The wasm target builds a browser-compatible version of the emulator."
	@echo "  After building, serve the $(BUILD_DIR_WASM)/bin directory via HTTP"
	@echo "  and open index.html in a browser."
	@echo ""
	@echo "Example:"
	@echo "  BOOT_TYPE=floppy IMAGE_FILE=FLOPPY.IMG make run"
	@echo "  DEBUGGER_ENABLED=OFF make release    # Build without debugger support"
	@echo "  make riscv                           # Build for RISC-V Linux"
	@echo ""
	@echo "This Makefile is a wrapper around CMake. If you prefer, you can use CMake directly:"
	@echo "  ./build.sh          - For more build options"
	@echo "  ./build_riscv.sh    - For RISC-V Linux builds"
	@echo ""