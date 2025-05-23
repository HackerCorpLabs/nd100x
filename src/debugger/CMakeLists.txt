# Debugger Module
project(debugger)


# Find source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Explicitly list source files
set(DEBUGGER_SOURCE_FILES
    debugger.c
)

# Add include directories - locate libdap if present
set(LIBDAP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/libdap/libdap/include")
set(NDLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/ndlib")

# Generate prototypes with mktypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/debugger_protos.h
    COMMAND echo "/* AUTO-GENERATED FILE. DO NOT EDIT! */" > ${CMAKE_CURRENT_SOURCE_DIR}/debugger_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/debugger.c >> ${CMAKE_CURRENT_SOURCE_DIR}/debugger_protos.h
    DEPENDS ${SOURCES}
    COMMENT "Generating prototypes for debugger"
    VERBATIM
)

# Create library
add_library(debugger STATIC ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/debugger_protos.h)

# Include directories
target_include_directories(debugger
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/src/ndlib
    PRIVATE ${CMAKE_SOURCE_DIR}/src/cpu
    PRIVATE ${CMAKE_SOURCE_DIR}/src/machine
)

# Link dependencies
target_link_libraries(debugger
    PUBLIC ndlib
    PUBLIC cpu
    PUBLIC machine
    PRIVATE cjson_objects
)

# Only define WITH_DEBUGGER when globally enabled
if(DEBUGGER_ENABLED)
    target_compile_definitions(debugger PRIVATE
        WITH_DEBUGGER
    )
    message(STATUS "Debugger module: WITH_DEBUGGER flag set")

    # Only include DAP and symbols libraries if debugger is enabled 

    # Check if libdap exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/libdap")
        # Include libdap
        target_include_directories(debugger
            PRIVATE ${LIBDAP_INCLUDE_DIR}
        )

        # Link with libdap - change from dap to dap_objects
        target_link_libraries(debugger
            PRIVATE dap_objects
            PRIVATE cjson_objects
        )

        # Add DAP_ENABLED definition
        target_compile_definitions(debugger
            PRIVATE DAP_ENABLED=1
        )

        message(STATUS "Debugger module: DAP support enabled (using libdap)")
    else()
        message(STATUS "Debugger module: DAP support disabled (libdap not found)")
    endif()

    # Check if libsymbols exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/libsymbols")
        # Set libsymbols include directory
        set(LIBSYMBOLS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/libsymbols/include")

        # Include libsymbols
        target_include_directories(debugger
            PRIVATE ${LIBSYMBOLS_INCLUDE_DIR}
        )

        # Link with libsymbols
        target_link_libraries(debugger
            PRIVATE symbols_objects
        )

        message(STATUS "Debugger module: Symbol support enabled (using libsymbols)")
    else()
        message(STATUS "Debugger module: Symbol support disabled (libsymbols not found)")
    endif()

else()
    message(STATUS "Debugger module: WITH_DEBUGGER flag not set, using stub implementations")
endif()

# Set properties
set_target_properties(debugger PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Install targets
install(TARGETS debugger
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Install headers
install(FILES ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/debugger_protos.h
    DESTINATION include/debugger
)
