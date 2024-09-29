cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(WINDOWS_PLATFORMS "Windows" "MSYS" "CYGWIN")
list(FIND WINDOWS_PLATFORMS "MSYS" audiodesigner_platform_index)
function(iswindows)
    if (NOT "${audiodesigner_platform_index}" STREQUAL "-1" )
        set(LASTCALL 1 PARENT_SCOPE)
        return()
    endif()
    set(LASTCALL 0 PARENT_SCOPE)
endfunction()

enable_testing()
option(WEPOLL_TESTING "Enable testing for the wepoll library" ON)
option(MINIAUDIO_TESTING "Enable testing for the miniaudio library" OFF)
option(NET_TESTING "Enable testing for Net library" ON)

set(CMAKE_INSTALL_PREFIX "$CMAKE_CURRENT_SOURCE_DIR/_install")

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wfloat-equal")
endif()

project(AUDIODESIGNER)

add_subdirectory(ext)
add_subdirectory(AudioEngine)
add_subdirectory(src)
