cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_SUPPRESS_REGENERATION FALSE)

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
option(TESTING_ENABLED "Toggle all testing" ON)
option(WEPOLL_TESTING "Enable testing for the wepoll library" ON)
option(MINIAUDIO_TESTING "Enable testing for the miniaudio library" OFF)
option(NET_TESTING "Enable testing for Net library" ON)
option(RINGBUFFER_TESTING "Enable testing for ringbuffers" ON)

set(CMAKE_INSTALL_PREFIX "$CMAKE_CURRENT_SOURCE_DIR/_install")

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wfloat-equal")
endif()

project(AUDIODESIGNER)

#wrapper for test registration which does nice cool things :)
function(lib_add_test test_target test_source test_libs_indirect)
  add_executable(${test_target} ${test_source})
  set_property(TARGET ${test_target} PROPERTY CXX_STANDARD 20) #C++ 20
  target_link_libraries(${test_target} PUBLIC "${${test_libs_indirect}}") #unpack test libs indirect into link libraries
  target_compile_options(${test_target} PRIVATE -Wall -Wextra -O3 -mavx2)
  set_target_properties( ${test_target}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
  add_test(
    NAME "ctest_${test_target}"
    COMMAND ${test_target}
  )

  message(STATUS "Add test ${test_target} with file ${test_source}")

endfunction()

add_subdirectory(ext)
add_subdirectory(AudioEngine)
add_subdirectory(src)
