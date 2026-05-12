# CMake toolchain file for the Milk-V Duo (CV1800B, T-Head C906) running
# musl userspace. Triplet: riscv64-unknown-linux-musl-.
#
# The root of the extracted "host-tools" tree is parameterised via the
# MILKV_HOST_TOOLS environment variable so CI runners and local devs share
# this file unchanged. Layout expected:
#
#     $MILKV_HOST_TOOLS/gcc/riscv64-linux-musl-x86_64/bin/
#                                 riscv64-unknown-linux-musl-{gcc,g++,...}
#
# Source: https://github.com/milkv-duo/host-tools (also mirrored at
#         https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/07/16/host-tools.tar.gz)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Resolve the host-tools root. Prefer the env var; fall back to the legacy
# hardcoded path so existing local checkouts still work, with a loud warning.
if(DEFINED ENV{MILKV_HOST_TOOLS} AND NOT "$ENV{MILKV_HOST_TOOLS}" STREQUAL "")
    set(_MILKV_ROOT "$ENV{MILKV_HOST_TOOLS}")
else()
    set(_MILKV_ROOT "/home/ronny/milkv/host-tools")
    message(WARNING
        "MILKV_HOST_TOOLS env var not set — falling back to legacy path "
        "${_MILKV_ROOT}. Set MILKV_HOST_TOOLS to the directory containing "
        "gcc/riscv64-linux-musl-x86_64/ for portable builds.")
endif()

set(_MILKV_BIN "${_MILKV_ROOT}/gcc/riscv64-linux-musl-x86_64/bin")

set(CMAKE_C_COMPILER   "${_MILKV_BIN}/riscv64-unknown-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "${_MILKV_BIN}/riscv64-unknown-linux-musl-gcc")

# Target sysroot for find_package / find_library.
set(CMAKE_FIND_ROOT_PATH "${_MILKV_ROOT}/gcc/riscv64-linux-musl-x86_64")

# Search for programs only in the build host directories.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ISA / ABI for the Duo's T-Head C906 core. rv64gc = rv64imafdc (general +
# compressed), lp64d = 64-bit long/pointer with double-precision FP ABI. The
# C906's vector extension is intentionally NOT enabled — mainline musl GCC
# 10.2 doesn't generate stable code for it on the Duo.
#
# Optimisation/debug flags are intentionally NOT pinned here — they come from
# CMAKE_BUILD_TYPE (Debug => -O0 -g, Release => -O3 -DNDEBUG, etc.) so CI
# release builds aren't forced to -O0.
set(CMAKE_C_FLAGS_INIT   "-march=rv64gc -mabi=lp64d")
set(CMAKE_CXX_FLAGS_INIT "-march=rv64gc -mabi=lp64d")

# Force static libraries for all dependencies (cJSON, libsymbols, libdap).
# The Duo's musl rootfs is minimal — static deps avoid shared-lib surprises.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Disable DAP and test tools for RISC-V builds.
set(BUILD_DAP_TOOLS    OFF CACHE BOOL "Disable DAP tools for RISC-V" FORCE)
set(ENABLE_CJSON_TEST  OFF CACHE BOOL "Disable cJSON tests for RISC-V" FORCE)
add_definitions(-DWITHOUT_DAP)
