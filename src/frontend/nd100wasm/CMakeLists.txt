# WebAssembly Frontend
project(nd100wasm)

# Find source files explicitly
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/nd100wasm.c
)

# Generate prototypes with mktypes
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/nd100wasm_protos.h
    COMMAND ${CMAKE_SOURCE_DIR}/tools/mkptypes/mkptypes ${CMAKE_CURRENT_SOURCE_DIR}/nd100wasm.c > ${CMAKE_CURRENT_SOURCE_DIR}/nd100wasm_protos.h
    DEPENDS ${SOURCES}
    COMMENT "Generating prototypes for nd100wasm"
    VERBATIM
)

# Create executable
add_executable(nd100wasm ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/nd100wasm_protos.h)

# Include directories
target_include_directories(nd100wasm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/frontend/nd100x   # Include nd100x directory for nd100x_types.h
    ${CMAKE_SOURCE_DIR}/src/ndlib
    ${CMAKE_SOURCE_DIR}/src/cpu
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/devices/panel
    ${CMAKE_SOURCE_DIR}/src/devices/rtc
    ${CMAKE_SOURCE_DIR}/src/devices/terminal
    ${CMAKE_SOURCE_DIR}/src/devices/papertape
    ${CMAKE_SOURCE_DIR}/src/devices/floppy
    ${CMAKE_SOURCE_DIR}/src/devices/smd
    ${CMAKE_SOURCE_DIR}/src/machine
)

# Modify nd100wasm.c to not include missing nd100x_types.h for now
add_custom_command(
    TARGET nd100wasm PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "// Comment out including nd100x_types.h until proper implementation" > ${CMAKE_CURRENT_BINARY_DIR}/compiler_flags.h
    COMMENT "Creating compiler flags for nd100wasm"
    VERBATIM
)

target_compile_definitions(nd100wasm PRIVATE
    ND100WASM_BUILD=1
)

# Link libraries
target_link_libraries(nd100wasm PRIVATE
    ndlib
    cpu
    devices
    machine
)

# Emscripten-specific configurations
if(PLATFORM_WASM)
    # Make sure the output goes to the bin directory
    set_target_properties(nd100wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        SUFFIX ".js"
    )
    
    # Add Emscripten-specific compiler and linker flags
    set(EMSCRIPTEN_LINK_FLAGS 
        "-s WASM=1 \
         -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
         -s EXPORT_NAME='ND100Module' \
         -s MODULARIZE=1 \
         --no-heap-copy \
         -s ALLOW_MEMORY_GROWTH=1"
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS} -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1")
    endif()
    
    # Apply Emscripten flags
    set_target_properties(nd100wasm PROPERTIES
        LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}"
    )
    
    # Check if a shell.html template exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
        set_target_properties(nd100wasm PROPERTIES
            LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS} --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
        )
        
        # Install HTML and JS files to bin directory
        install(FILES 
            "${CMAKE_BINARY_DIR}/bin/nd100wasm.js"
            "${CMAKE_BINARY_DIR}/bin/nd100wasm.wasm"
            "${CMAKE_BINARY_DIR}/bin/nd100wasm.html"
            DESTINATION bin
        )
    else()
        # Install JS and WASM files to bin directory
        install(FILES 
            "${CMAKE_BINARY_DIR}/bin/nd100wasm.js"
            "${CMAKE_BINARY_DIR}/bin/nd100wasm.wasm"
            DESTINATION bin
        )
    endif()
    
    # Copy the index.html from template if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/.vscode/template/index.html")
        install(FILES 
            "${CMAKE_SOURCE_DIR}/.vscode/template/index.html"
            DESTINATION bin
        )
    endif()
endif()

# Install the executable (for non-WASM platforms)
if(NOT PLATFORM_WASM)
    install(TARGETS nd100wasm
        RUNTIME DESTINATION bin
    )
endif() 