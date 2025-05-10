set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Specify the cross compilers with absolute paths
set(CMAKE_C_COMPILER "/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "/home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc")

# Set the target environment path
set(CMAKE_FIND_ROOT_PATH /home/ronny/milkv/host-tools/gcc/riscv64-linux-musl-x86_64)

# Search for programs only in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# RISC-V specific flags with debug symbols and no optimization
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64gc -mabi=lp64d -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=rv64gc -mabi=lp64d -g -O0")

# Force static libraries for all dependencies
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Disable DAP and test tools for RISC-V builds
set(BUILD_DAP_TOOLS OFF CACHE BOOL "Disable DAP tools for RISC-V" FORCE)
set(ENABLE_CJSON_TEST OFF CACHE BOOL "Disable cJSON tests for RISC-V" FORCE)
add_definitions(-DWITHOUT_DAP)

# Don't actually include the cJSON lib here, we'll do it in a custom build step later
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--whole-archive ${CMAKE_BINARY_DIR}/external/cJSON/libcjson.a -Wl,--no-whole-archive") 