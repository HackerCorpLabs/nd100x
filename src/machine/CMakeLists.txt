# Machine Module
project(machine)

# Find source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Explicitly list source files
set(MACHINE_SOURCE_FILES
    machine.c
    io.c
)

# Generate prototypes with mktypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h
    COMMAND echo "/* AUTO-GENERATED FILE. DO NOT EDIT! */" > ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/machine.c >> ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/io.c >> ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h
    DEPENDS ${SOURCES}
    COMMENT "Generating prototypes for machine"
    VERBATIM
)

# Create library
add_library(machine STATIC ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h)

# Include directories
target_include_directories(machine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/src/ndlib
    PRIVATE ${CMAKE_SOURCE_DIR}/src/cpu
    PRIVATE ${CMAKE_SOURCE_DIR}/src/devices
)

# Link dependencies
target_link_libraries(machine
    PUBLIC ndlib
    PUBLIC cpu
    PUBLIC devices
)

# Set properties
set_target_properties(machine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Install targets
install(TARGETS machine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Install headers
install(FILES ${HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/machine_protos.h
    DESTINATION include/machine
) 