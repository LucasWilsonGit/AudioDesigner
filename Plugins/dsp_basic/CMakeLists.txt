cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

add_executable(dsp_basic)
set_property(TARGET dsp_basic PROPERTY CXX_STANDARD 20) #C++ 20
target_include_directories(dsp_basic PRIVATE Include)
target_sources(dsp_basic PRIVATE src/main.cpp)


target_link_libraries(dsp_basic AudioEngine)

message(STATUS "PLATFORM_ID: ${PLATFORM_ID} CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if (ISWINDOWS)
    target_link_libraries(dsp_basic ntdll)
endif()

# Define the source and destination files
set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/conf.cfg)
set(DEST_FILE ${CMAKE_BINARY_DIR}/conf.cfg)

# Add a custom command to copy the file
add_custom_command(
    OUTPUT ${DEST_FILE}
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${DEST_FILE}
    DEPENDS ${SOURCE_FILE}  # Re-run if the source file changes 
    COMMENT "Copying conf.cfg to the binary directory at build time"
)

# Create a custom target that depends on the copy command
add_custom_target(copy_conf ALL DEPENDS ${DEST_FILE})

# Optionally make other targets depend on this (e.g., your executable)
add_dependencies(dsp_basic copy_conf)