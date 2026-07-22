# Printer module unit tests
#
# POSIX-only: the printer tests use mkdtemp, which MinGW does not provide, so
# they are skipped on Windows (see the note in the root CMakeLists.txt).
if(NOT PLATFORM_WINDOWS)
    add_executable(test_printer
        test_printer_main.c
        test_pdfwriter.c
        test_escp.c
        test_printjob.c
    )

    target_link_libraries(test_printer PRIVATE ndlib m)

    target_include_directories(test_printer PRIVATE
        ${CMAKE_SOURCE_DIR}/src/ndlib
    )

    add_test(NAME printer_tests COMMAND test_printer)
endif()

# HDLC module unit tests

add_executable(test_hdlc
    test_hdlc_main.c
    test_tcp_receive_buffer.c
    test_hdlc_crc.c
    test_hdlc_frame.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/tcpReceiveBuffer.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/hdlcFrame.c
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc/hdlc_crc.c
)

target_include_directories(test_hdlc PRIVATE
    ${CMAKE_SOURCE_DIR}/src/devices/hdlc
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/cpu
)

target_link_libraries(test_hdlc PRIVATE m)

add_test(NAME hdlc_tests COMMAND test_hdlc)

# ---------------------------------------------------------------------------
# nd100x --pipe TPE INSTRUCTION regression gate (ND-120), driven by tools/nd100x_expect.py
# The floppy path comes from -DNDX_TPE_FLOPPY=... (or $ND100X_TPE_FLOPPY, or the
# script default). If Python is missing the test is not registered; if the image is
# missing the script exits 2, which CMake treats as SKIPPED (SKIP_RETURN_CODE), so a
# machine without the (non-redistributable) TPE image never sees a red failure.
if(NOT PLATFORM_WASM AND NOT PLATFORM_RISCV)
    find_program(PYTHON_EXECUTABLE NAMES python3 python)
    if(PYTHON_EXECUTABLE)
        set(NDX_TPE_FLOPPY "" CACHE FILEPATH "TPE diagnostic floppy image for the nd100x pipe regression test")
        add_test(
            NAME nd120_tpe_pipe
            COMMAND ${PYTHON_EXECUTABLE}
                    ${CMAKE_SOURCE_DIR}/tools/examples/tpe_instruction_nd120.py
                    ${NDX_TPE_FLOPPY}
        )
        set_tests_properties(nd120_tpe_pipe PROPERTIES
            SKIP_RETURN_CODE 2      # exit 2 = image not found -> SKIPPED, not failed
            TIMEOUT 180             # a full TPE boot + INSTRUCTION run
            LABELS "integration;nd120;pipe"
        )
    else()
        message(STATUS "Python not found - skipping the nd120_tpe_pipe integration test")
    endif()
endif()

# Floppy/disk catalog API tests
#
# floppydb.c is linked in DIRECTLY together with the bundled cJSON parser and a
# stub download_file() provided by test_floppydb.c, so the catalog parse/search
# logic is exercised WITHOUT the network (libcurl) or the machine. This is why the
# test runs on every build, including Windows w64devkit where libcurl is absent.
add_executable(test_floppydb
    test_floppydb.c
    ${CMAKE_SOURCE_DIR}/src/ndlib/floppydb.c
    ${CMAKE_SOURCE_DIR}/external/cJSON/cJSON.c
)

target_include_directories(test_floppydb PRIVATE
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/machine
    ${CMAKE_SOURCE_DIR}/src/cpu
    ${CMAKE_SOURCE_DIR}/external/cJSON
)

target_link_libraries(test_floppydb PRIVATE m)

if(PLATFORM_WINDOWS)
    # machine_types.h -> cpu_types.h pulls in windows.h / timeBeginPeriod().
    target_link_libraries(test_floppydb PRIVATE winmm)
endif()

add_test(NAME floppydb_tests COMMAND test_floppydb)

# DRUM device unit tests
#
# deviceDrum.c is linked in DIRECTLY together with fake Device_* infrastructure
# provided by test_drum.c (fake DMA memory + delayed-IO), so the drum's register
# handlers and transfer engine are exercised without the machine or a disk image.
add_executable(test_drum
    test_drum.c
    ${CMAKE_SOURCE_DIR}/src/devices/drum/deviceDrum.c
)

target_include_directories(test_drum PRIVATE
    ${CMAKE_SOURCE_DIR}/src/devices/drum
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/cpu
)

target_link_libraries(test_drum PRIVATE m)

# deviceDrum.c #includes the generated devices_protos.h; force the devices
# target (which runs mkptypes to emit it) to build first.
add_dependencies(test_drum devices)

add_test(NAME drum_tests COMMAND test_drum)

# CDC system-disc device unit tests
#
# deviceCDC.c is linked in DIRECTLY together with fake Device_* infrastructure
# provided by test_cdc.c (fake DMA memory + delayed-IO), so the disc's register
# handlers, transfer engine and backing-file logic are exercised without the
# machine or the CPU memory subsystem.
add_executable(test_cdc
    test_cdc.c
    ${CMAKE_SOURCE_DIR}/src/devices/cdc/deviceCDC.c
)

target_include_directories(test_cdc PRIVATE
    ${CMAKE_SOURCE_DIR}/src/devices/cdc
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/cpu
)

target_link_libraries(test_cdc PRIVATE m)

# deviceCDC.c #includes the generated devices_protos.h; force the devices
# target (which runs mkptypes to emit it) to build first.
add_dependencies(test_cdc devices)

add_test(NAME cdc_tests COMMAND test_cdc)
# CPU packed-decimal (BCD) instruction tests
#
# bcd.c is linked in DIRECTLY (not via libcpu) together with fake
# MemoryRead/MemoryWrite and a fake register file provided by test_bcd.c, so the
# decimal engine is exercised through its real ndfunc_* entry points without
# pulling in the machine, the devices or a disk image.
add_executable(test_bcd
    test_bcd.c
    ${CMAKE_SOURCE_DIR}/src/cpu/bcd.c
)

target_include_directories(test_bcd PRIVATE
    ${CMAKE_SOURCE_DIR}/src/cpu
    ${CMAKE_SOURCE_DIR}/src/ndlib
)

target_link_libraries(test_bcd PRIVATE m)

if(PLATFORM_WINDOWS)
    # cpu_types.h pulls in windows.h and timeBeginPeriod() for sleep_ms().
    target_link_libraries(test_bcd PRIVATE winmm)
endif()

add_test(NAME bcd_tests COMMAND test_bcd)
