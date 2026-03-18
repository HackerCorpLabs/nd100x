# ND Library Module
project(ndlib)

# Find source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# List specific source files instead of using wildcard
set(NDLIB_SOURCE_FILES
    load_bpun.c
    ndlib.c
    log.c
    download.c
    keyboard.c
    pdfwriter.c
    escp.c
    printjob.c
)

# Add telnet server for platforms with pthreads/sockets (not WASM)
if(NOT (PLATFORM_WASM OR BUILD_WASM))
    list(APPEND NDLIB_SOURCE_FILES telnetserver.c)
endif()

# Generate prototypes with mktypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos_preamble.h > ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/load_bpun.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/ndlib.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/log.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/download.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/keyboard.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/pdfwriter.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/escp.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/printjob.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    DEPENDS ${SOURCES} ${NDLIB_SOURCE_FILES}
    COMMENT "Generating prototypes for NDLIB"
    VERBATIM
)

# Create library
add_library(ndlib STATIC ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h)

# Include directories
target_include_directories(ndlib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set properties
set_target_properties(ndlib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Link pthread for platforms with telnet server (not WASM)
if(NOT (PLATFORM_WASM OR BUILD_WASM))
    target_link_libraries(ndlib PUBLIC pthread)
endif()

# Link CURL for platforms where it's available so headers and linkage are visible
if(NOT (PLATFORM_WASM OR BUILD_WASM OR PLATFORM_RISCV OR BUILD_RISCV))
    # Prefer imported target when available
    if(TARGET CURL::libcurl)
        target_link_libraries(ndlib PUBLIC CURL::libcurl)
    else()
        # Fallback to variables set by FindCURL
        if(DEFINED CURL_INCLUDE_DIRS)
            target_include_directories(ndlib PUBLIC ${CURL_INCLUDE_DIRS})
        endif()
        if(DEFINED CURL_LIBRARIES)
            target_link_libraries(ndlib PUBLIC ${CURL_LIBRARIES})
        endif()
    endif()
endif()

# Platform-specific compile definitions
if(PLATFORM_LINUX)
    target_compile_definitions(ndlib PUBLIC PLATFORM_LINUX)
elseif(PLATFORM_WINDOWS)
    target_compile_definitions(ndlib PUBLIC PLATFORM_WINDOWS)
elseif(PLATFORM_WASM)
    target_compile_definitions(ndlib PUBLIC PLATFORM_WASM)
elseif(PLATFORM_RISCV)
    target_compile_definitions(ndlib PUBLIC PLATFORM_RISCV)
endif()

# Install targets
install(TARGETS ndlib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Install headers
install(FILES ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    DESTINATION include/ndlib
) 