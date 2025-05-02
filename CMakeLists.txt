cmake_minimum_required(VERSION 3.14)
project(nd100x VERSION 0.1.0 DESCRIPTION "ND100X Emulator" LANGUAGES C CXX)

# Options
option(BUILD_DAP_TOOLS "Build libdap test tools" OFF)
option(BUILD_WASM "Build for WebAssembly using Emscripten" OFF)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(PLATFORM_WASM TRUE)
    message(STATUS "Building for WebAssembly")
    
    # Apply Emscripten-specific settings when compiling for WASM
    if(BUILD_WASM OR EMSCRIPTEN)
        message(STATUS "Configuring for WebAssembly (WASM) using Emscripten")
    endif()
else()
    message(WARNING "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Handle debugger conditionally based on platform
if(PLATFORM_LINUX OR PLATFORM_WINDOWS)
    # Define WITH_DEBUGGER for the entire project
    add_definitions(-DWITH_DEBUGGER)
    message(STATUS "Enabling debugger support with _DEBUGGER_ENABLED_ flag")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src/ndlib)
add_subdirectory(src/cpu)
add_subdirectory(src/devices)
add_subdirectory(src/machine)

# Handle debugger conditionally based on platform
if(PLATFORM_LINUX OR PLATFORM_WINDOWS)
    # Add the debugger directory
    add_subdirectory(src/debugger)
    
    # Add libdap if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/libdap")
        set(BUILD_DAP_TEST_TOOLS ${BUILD_DAP_TOOLS} CACHE BOOL "Build libdap test tools" FORCE)
        add_subdirectory(external/libdap)
        message(STATUS "DAP library support enabled (only for Linux/Windows builds)")
    endif()
endif()

# Add frontend subdirectories
add_subdirectory(src/frontend/nd100x)
add_subdirectory(src/frontend/nd100wasm)

# Build mktypes tool
message(STATUS "Building mktypes tool...")
execute_process(
    COMMAND make -C ${CMAKE_SOURCE_DIR}/tools/mkptypes
    RESULT_VARIABLE MKTYPES_RESULT
)
if(NOT MKTYPES_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build mktypes tool. Error: ${MKTYPES_RESULT}")
endif()

# Display helper messages
if(PLATFORM_WASM OR BUILD_WASM)
    message(STATUS "")
    message(STATUS "======================= WebAssembly Build Notes =======================")
    message(STATUS "To test in a browser, serve the build directory via HTTP server:")
    message(STATUS "    cd ${CMAKE_BINARY_DIR}")
    message(STATUS "    python3 -m http.server")
    message(STATUS "Then open http://localhost:8000 in your browser.")
    message(STATUS "====================================================================")
    message(STATUS "")
endif()

# Display platform-specific features
if(NOT PLATFORM_WASM)
    message(STATUS "")
    message(STATUS "======================= Build Configuration Notes =====================")
    if(PLATFORM_LINUX OR PLATFORM_WINDOWS)
        message(STATUS "Debugger support: ENABLED")
        message(STATUS "- _DEBUGGER_ENABLED_ flag is set for CPU and debugger modules")        
    else()
        message(STATUS "Debugger support: DISABLED (only available on Linux/Windows)")
    endif()
    message(STATUS "====================================================================")
    message(STATUS "")
endif()
