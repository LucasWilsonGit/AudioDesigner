cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

add_executable(dsp_basic)
set_property(TARGET dsp_basic PROPERTY CXX_STANDARD 20) #C++ 20
target_include_directories(dsp_basic PRIVATE Include)
target_sources(dsp_basic PRIVATE src/main.cpp)


target_link_libraries(dsp_basic AudioEngine)

message(STATUS "PLATFORM_ID: ${PLATFORM_ID} CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

iswindows()
if ("${LASTCALL}" STREQUAL "1" )
    target_link_libraries(dsp_basic ntdll)
endif()