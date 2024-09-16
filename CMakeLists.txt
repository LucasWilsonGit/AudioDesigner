cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_INSTALL_PREFIX "$CMAKE_CURRENT_SOURCE_DIR/_install")

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wfloat-equal")
endif()

project(AUDIODESIGNER)

add_subdirectory(ext)
add_subdirectory(AudioEngine)
add_subdirectory(src)
