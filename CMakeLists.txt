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
# WIN32 is set by CMake on all Windows builds (MSVC, MinGW, w64devkit, MSYS2)
# so use it in addition to CMAKE_SYSTEM_NAME for reliability.
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
    # Force-disable the debugger on Windows until libdap and libsymbols have
    # their own Windows ports — without those backends the debugger module
    # has no implementation to call into. Removing this override will work
    # the moment both submodules compile on Windows.
    if(DEBUGGER_ENABLED)
        message(STATUS "DEBUGGER_ENABLED overridden to OFF on Windows "
                       "(libdap/libsymbols not ported yet)")
    endif()
    set(DEBUGGER_ENABLED OFF CACHE BOOL "Enable debugger support" FORCE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(BUILD_RISCV OR CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
        set(PLATFORM_RISCV TRUE)
        set(DEBUGGER_ENABLED FALSE)
        message(STATUS "Building for RISC-V Linux")
    else()
        set(PLATFORM_LINUX TRUE)
        message(STATUS "Building for Linux")
    endif()
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(PLATFORM_WASM TRUE)
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

# Find required packages for menu functionality.
#
# - WASM / RISC-V: no menu, skip entirely.
# - Linux: CURL + ncurses are required (menu is built, URL image loads work).
# - Windows: CURL is optional; if missing, download_file() compiles as a stub.
#   ncurses is not looked up — the F12 floppy-DB menu is not ported to Windows
#   yet and is compiled out via NEED_MENU in src/frontend/nd100x/CMakeLists.txt.
if(NOT (PLATFORM_WASM OR BUILD_WASM OR PLATFORM_RISCV OR BUILD_RISCV))
    if(PLATFORM_WINDOWS)
        find_package(CURL QUIET)
        if(CURL_FOUND)
            message(STATUS "CURL library found: ${CURL_VERSION_STRING}")
            add_definitions(-DHAVE_CURL)
        else()
            message(STATUS "CURL not found on Windows — download_file() will be a stub "
                           "(install mingw-w64-x86_64-curl under MSYS2, or vendor libcurl).")
        endif()
        message(STATUS "Curses lookup skipped on Windows (F12 menu disabled).")
    else()
        find_package(CURL REQUIRED)
        message(STATUS "CURL library found: ${CURL_FOUND}")
        add_definitions(-DHAVE_CURL)

        find_package(Curses REQUIRED)
        message(STATUS "Curses library found: ${CURSES_FOUND}")
    endif()
endif()

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
    
    # Add libdap if it exists. Skipped on WASM (no sockets/pthreads in the
    # browser) and on Windows (libdap's source uses POSIX-only <sys/socket.h>
    # / <netdb.h> and needs its own Windows port — same story as libsymbols).
    # When skipped, the debugger module compiles its stub implementations.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/libdap/CMakeLists.txt"
       AND NOT PLATFORM_WASM AND NOT PLATFORM_WINDOWS)
        set(BUILD_DAP_TEST_TOOLS ${BUILD_DAP_TOOLS} CACHE BOOL "Build libdap test tools" FORCE)
        add_subdirectory(external/libdap EXCLUDE_FROM_ALL)
        if(TARGET dap_objects)
            target_link_libraries(dap_objects PRIVATE cjson_objects)
        endif()
        message(STATUS "DAP library support enabled (only for Linux/Windows builds)")
    elseif(PLATFORM_WINDOWS)
        message(STATUS "libdap skipped on Windows (port pending — DAP debugger unavailable)")
    endif()
else()    
    message(STATUS "Debugger support disabled")
endif()

# Add libsymbols if it exists - always include this for a.out support.
# Excluded on Windows: libsymbols' source uses POSIX-only constructs
# (strndup, DATA_START_SPLIT_ID derived from system a.out.h) that need a
# separate port. Without it, .aout boot is unavailable on Windows; bp/bpun/
# floppy/smd boot types still work.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/libsymbols/CMakeLists.txt"
   AND NOT PLATFORM_WINDOWS)
    add_subdirectory(external/libsymbols EXCLUDE_FROM_ALL)
    message(STATUS "Symbol library support enabled")
elseif(PLATFORM_WINDOWS)
    message(STATUS "libsymbols skipped on Windows (port pending — .aout boot unavailable)")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build mkptypes tool first before we need it.
message(STATUS "Building mkptypes tool...")
execute_process(
    COMMAND make -C ${CMAKE_SOURCE_DIR}/tools/mkptypes
    RESULT_VARIABLE MKTYPES_RESULT
)
if(NOT MKTYPES_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build mkptypes tool. Error: ${MKTYPES_RESULT}")
endif()

# On Windows, the Makefile emits mkptypes.exe, but every downstream
# CMakeLists references the tool as `${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes`
# (no extension). Copy the binary to the extension-less name so those 30+
# existing references keep working. Windows' CreateProcess will execute a
# valid PE file regardless of whether its name ends in .exe.
if(WIN32)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes.exe
            ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes
        RESULT_VARIABLE MKTYPES_COPY_RESULT
    )
    if(NOT MKTYPES_COPY_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to copy mkptypes.exe -> mkptypes")
    endif()
endif()

# Add subdirectories in the correct order for header generation
add_subdirectory(src/ndlib)
add_subdirectory(src/machine)
add_subdirectory(src/cpu)
add_subdirectory(src/devices)
add_subdirectory(src/debugger)

# Cross-module generated-header dependencies.
#
# cpu.c #includes machine/machine_protos.h, so the cpu library needs the
# machine protos generated first. On Ninja with Windows, this dep plus the
# fact that machine PUBLIC-links cpu used to create a cycle — that's why
# machine's CMakeLists wraps the generator in a custom_target rather than
# listing machine_protos.h as a library source.
if(TARGET cpu AND TARGET machine_protos_gen)
    add_dependencies(cpu machine_protos_gen)
endif()

# Add frontend subdirectories conditionally
if(BUILD_WASM)
    add_subdirectory(src/frontend/nd100wasm)
else()
    add_subdirectory(src/frontend/nd100x)
endif()

# Add test subdirectory (only for native non-Windows builds — the printer
# tests use POSIX mkdtemp which has no MinGW equivalent without further work).
if(NOT (BUILD_WASM OR BUILD_RISCV OR PLATFORM_WINDOWS))
    enable_testing()
    add_subdirectory(tests)
endif()

# Emscripten-specific settings are applied in src/frontend/nd100wasm/CMakeLists.txt
# Do NOT override LINK_FLAGS here - the subdirectory sets EXPORTED_FUNCTIONS etc.

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