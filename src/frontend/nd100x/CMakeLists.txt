# nd100x frontend executable
set(SOURCES 
    config.c 
    nd100x.c
)

# Create the executable
add_executable(nd100x ${SOURCES})

# Include directories
target_include_directories(nd100x PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/cpu
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/machine
)

# Link with required libraries
target_link_libraries(nd100x PRIVATE
    machine
    devices
    cpu
    ndlib
    m
)

# Add debug flags as needed (matching Makefile)
target_compile_definitions(nd100x PRIVATE
    _GNU_SOURCE
    DEBUG
)

# If debugger is enabled, add the include and link with debugger lib
if(PLATFORM_LINUX OR PLATFORM_WINDOWS)
    target_include_directories(nd100x PRIVATE
        ${CMAKE_SOURCE_DIR}/src/debugger
    )
    
    # Link with debugger library
    target_link_libraries(nd100x PRIVATE
        debugger
    )
    
    # If libdap is available, link with it
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/libdap")
        target_include_directories(nd100x PRIVATE
            ${CMAKE_SOURCE_DIR}/external/libdap/libdap/include
        )
        target_link_libraries(nd100x PRIVATE
            dap_objects
        )
    endif()
endif()

# Add installation rule to copy config file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/nd100x.conf
        DESTINATION ${CMAKE_BINARY_DIR}/etc)

# Create the prototypes header using mkptypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/nd100x_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/config.c > ${CMAKE_CURRENT_SOURCE_DIR}/nd100x_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/nd100x.c >> ${CMAKE_CURRENT_SOURCE_DIR}/nd100x_protos.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config.c ${CMAKE_CURRENT_SOURCE_DIR}/nd100x.c
    COMMENT "Generating nd100x_protos.h"
)

# Make sure the prototype header is generated before compiling
add_custom_target(nd100x_protos_gen DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/nd100x_protos.h)
add_dependencies(nd100x nd100x_protos_gen) 