#!/bin/bash

# Script to build the ND100X project using CMake

# Exit on error
set -e

# Default build type
BUILD_TYPE="Debug"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
BUILD_WASM=0
BUILD_DAP_TOOLS=0

# Parse command line arguments
while [ $# -gt 0 ]; do
  case $1 in
    --release)
      BUILD_TYPE="Release"
      shift
      ;;
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --install-prefix)
      INSTALL_PREFIX="$2"
      shift 2
      ;;
    --wasm)
      BUILD_WASM=1
      shift
      ;;
    --with-dap-tools)
      BUILD_DAP_TOOLS=1
      shift
      ;;
    --help)
      echo "Usage: $0 [options]"
      echo "Options:"
      echo "  --release          Build in Release mode (default: Debug)"
      echo "  --build-dir DIR    Set build directory (default: build)"
      echo "  --install-prefix P Set installation prefix (default: /usr/local)"
      echo "  --wasm             Build WebAssembly version"
      echo "                     (Note: Debugger support is disabled in WASM builds)"
      echo "  --with-dap-tools   Build DAP test tools (dap_debugger, dap_mock_server)"
      echo "                     (Note: Only available on Linux/Windows builds)"
      echo "  --help             Show this help message"
      echo ""
      echo "Platform-specific features:"
      echo "  - Linux/Windows: Automatically defines _DEBUGGER_ENABLED_ for CPU/debugger modules"
      echo "  - Linux/Windows: Enables DAP (Debug Adapter Protocol) integration"
      echo "  - WebAssembly:   No debugger support, optimized for browser performance"
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

if [ $BUILD_WASM -eq 1 ]; then
  # Check if emcmake is available
  if ! command -v emcmake &> /dev/null; then
    echo "Error: emcmake not found. Please install and activate Emscripten SDK."
    exit 1
  fi
  
  # Create WASM build directory
  WASM_BUILD_DIR="${BUILD_DIR}_wasm"
  mkdir -p "$WASM_BUILD_DIR"
  
  echo "Configuring WebAssembly build in $WASM_BUILD_DIR..."
  emcmake cmake -B "$WASM_BUILD_DIR" -S . \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_DAP_TOOLS=$BUILD_DAP_TOOLS \
    -DBUILD_WASM=ON
  
  echo "Building WebAssembly targets..."
  cmake --build "$WASM_BUILD_DIR" -- -j$(nproc 2>/dev/null || echo 4)
  
  # Copy index.html template to bin directory
  echo "Copying index.html template to WASM build directory..."
  mkdir -p "$WASM_BUILD_DIR/bin"
  cp .vscode/template/index.html "$WASM_BUILD_DIR/bin/"
else
  # Create native build directory
  mkdir -p "$BUILD_DIR"
  
  echo "Configuring native build in $BUILD_DIR..."
  cmake -B "$BUILD_DIR" -S . \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_DAP_TOOLS=$BUILD_DAP_TOOLS
  
  echo "Building native targets..."
  cmake --build "$BUILD_DIR" -- -j$(nproc 2>/dev/null || echo 4)
fi

echo "Build completed successfully!"
if [ $BUILD_WASM -eq 1 ]; then
  echo "To test WASM build in a browser, run:"
  echo "  cd $WASM_BUILD_DIR/bin"
  echo "  python3 -m http.server"
  echo "  # Then open http://localhost:8000/index.html in your browser"
  echo "To install, run: cmake --install $WASM_BUILD_DIR"
else
  echo "To install, run: cmake --install $BUILD_DIR"
  if [ "$(uname -s)" == "Linux" ] || [ "$(uname -s)" == "MINGW"* ] || [ "$(uname -s)" == "MSYS"* ]; then
    echo "Note: This build includes debugger support with _DEBUGGER_ENABLED_ flag"
  fi
fi 