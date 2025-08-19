cmake_minimum_required(VERSION 3.14)

# Prevent in-source builds to protect the existing Makefile
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR 
        "In-source builds are not allowed!\n"
        "This would overwrite the existing Makefile in the root folder.\n"
        "Please create a build directory and run cmake from there:\n"
        "  mkdir -p build && cd build && cmake ..\n"
        "To remove files created by an attempted in-source build, run:\n"
        "  rm -rf CMakeCache.txt CMakeFiles/\n"
    )
endif()

project(nd100x VERSION 0.1.0 DESCRIPTION "ND100X Emulator" LANGUAGES C CXX)

# Options
option(BUILD_DAP_TOOLS "Build libdap test tools" OFF)
option(BUILD_WASM "Build for WebAssembly using Emscripten" OFF)
option(BUILD_RISCV "Build for RISC-V Linux using gcc-riscv64-unknown-linux-musl" OFF)
option(DEBUGGER_ENABLED "Enable debugger support" ON)

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(BUILD_RISCV OR CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
        set(PLATFORM_RISCV TRUE)
        set(DEBUGGER_ENABLED FALSE) 
        message(STATUS "Building for RISC-V Linux")
    else()
        set(PLATFORM_LINUX TRUE)        
        message(STATUS "Building for Linux")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(PLATFORM_WASM TRUE)
    set(DEBUGGER_ENABLED FALSE)
    message(STATUS "Building for WebAssembly")
    
    if(BUILD_WASM OR EMSCRIPTEN)
        message(STATUS "Configuring for WebAssembly (WASM) using Emscripten")
    endif()
else()
    message(WARNING "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Add cJSON as an object library first
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON")
    add_library(cjson_objects OBJECT
        external/cJSON/cJSON.c
    )
    target_include_directories(cjson_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON
    )
    message(STATUS "cJSON library support enabled")
endif()

# Find required packages for menu functionality
find_package(CURL REQUIRED)
find_package(Curses REQUIRED)

message(STATUS "CURL library found: ${CURL_FOUND}")
message(STATUS "Curses library found: ${CURSES_FOUND}")

# Handle RISC-V specific configuration
if(PLATFORM_RISCV)
    message(STATUS "Using RISC-V cross-compiler: ${CMAKE_C_COMPILER}")
    
    # For RISC-V, disable DAP client tools but keep DAP support
    # Setting this to OFF to prevent building dap_debugger and dap_mock_server
    set(BUILD_DAP_TOOLS OFF CACHE BOOL "Disable DAP client tools for RISC-V" FORCE)
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "Disable cJSON tests for RISC-V" FORCE)
    
    # Force static libraries for RISC-V builds
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
    
    # Add special linker flags for RISC-V
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
    
    # Add include path for cJSON for RISC-V builds
    include_directories(${CMAKE_SOURCE_DIR}/external/cJSON)
    add_definitions(-DCJSON_INCLUDE_PATH="${CMAKE_SOURCE_DIR}/external/cJSON")
endif()

# Handle debugger configuration
if(DEBUGGER_ENABLED)
    add_definitions(-DWITH_DEBUGGER )
    message(STATUS "Enabling debugger support with WITH_DEBUGGER flag")
    
    # Create a directory structure for cjson headers
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/cjson)
    
    # Copy cJSON.h to the expected location
    configure_file(
        ${CMAKE_SOURCE_DIR}/external/cJSON/cJSON.h
        ${CMAKE_BINARY_DIR}/include/cjson/cJSON.h
        COPYONLY
    )
    
    # Add this to the include path
    include_directories(${CMAKE_BINARY_DIR}/include)
    
    # Add libdap if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/libdap")
        set(BUILD_DAP_TEST_TOOLS ${BUILD_DAP_TOOLS} CACHE BOOL "Build libdap test tools" FORCE)
        add_subdirectory(external/libdap)
        if(TARGET dap_objects)
            target_link_libraries(dap_objects PRIVATE cjson_objects)
        endif()
        message(STATUS "DAP library support enabled (only for Linux/Windows builds)")
    endif()
else()    
    message(STATUS "Debugger support disabled")
endif()

# Add libsymbols if it exists - always include this for a.out support
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/libsymbols")
    add_subdirectory(external/libsymbols)
    message(STATUS "Symbol library support enabled")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build mktypes tool first before we need it
message(STATUS "Building mktypes tool...")
execute_process(
    COMMAND make -C ${CMAKE_SOURCE_DIR}/tools/mkptypes
    RESULT_VARIABLE MKTYPES_RESULT
)
if(NOT MKTYPES_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build mktypes tool. Error: ${MKTYPES_RESULT}")
endif()

# Add subdirectories in the correct order for header generation
add_subdirectory(src/ndlib)
add_subdirectory(src/machine)
add_subdirectory(src/cpu)
add_subdirectory(src/devices)
add_subdirectory(src/debugger)

# Add frontend subdirectories conditionally
if(BUILD_WASM)
    add_subdirectory(src/frontend/nd100wasm)
else()
    add_subdirectory(src/frontend/nd100x)
endif()

# Apply Emscripten-specific settings when compiling for WASM
if(PLATFORM_WASM OR BUILD_WASM)
    set(EMSCRIPTEN_LINK_FLAGS
        "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','stackTrace','noExitRuntime','preloadPlugins','STACK_SIZE','FS','addFunction'] -s ALLOW_TABLE_GROWTH=1"
    )
    
    if(TARGET nd100wasm)
        set_target_properties(nd100wasm PROPERTIES LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}")
    endif()
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

# Display RISC-V specific instructions
if(BUILD_RISCV)
    message(STATUS "")
    message(STATUS "======================= RISC-V Build Notes ==========================")
    message(STATUS "Building for RISC-V Linux using gcc-riscv64-unknown-linux-musl")
    message(STATUS "To build the RISC-V target:")
    message(STATUS "    mkdir -p build_riscv && cd build_riscv")
    message(STATUS "    cmake -DBUILD_RISCV=ON ..")
    message(STATUS "    make")
    message(STATUS "====================================================================")
    message(STATUS "")
endif()

# Display build configuration
message(STATUS "")
message(STATUS "======================= Build Configuration Notes =====================")
message(STATUS "Debugger support: ${DEBUGGER_ENABLED}")
if(DEBUGGER_ENABLED)
    message(STATUS "- WITH_DEBUGGER flag is set for CPU and debugger modules")
endif()
message(STATUS "====================================================================")
message(STATUS "") 