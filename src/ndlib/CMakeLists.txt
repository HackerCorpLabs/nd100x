# ND Library Module
project(ndlib)

# Find source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# List specific source files instead of using wildcard
set(NDLIB_SOURCE_FILES
    load_aout.c
    load_bpun.c
    ndlib.c
    log.c
)

# Generate prototypes with mktypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND echo "/* AUTO-GENERATED FILE. DO NOT EDIT! */" > ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/load_bpun.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/ndlib.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/log.c >> ${CMAKE_CURRENT_SOURCE_DIR}/ndlib_protos.h
    DEPENDS ${SOURCES}
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

# Platform-specific compile definitions
if(PLATFORM_LINUX)
    target_compile_definitions(ndlib PUBLIC PLATFORM_LINUX)
elseif(PLATFORM_WINDOWS)
    target_compile_definitions(ndlib PUBLIC PLATFORM_WINDOWS)
elseif(PLATFORM_WASM)
    target_compile_definitions(ndlib PUBLIC PLATFORM_WASM)
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