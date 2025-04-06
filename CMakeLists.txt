cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_SUPPRESS_REGENERATION FALSE)

include(${CMAKE_CURRENT_SOURCE_DIR}/build_utils.cmake)
detect_msvc(MSVC)
setup_global_compiler_options()
iswindows() #writes ISWINDOWS toplevel variable


enable_testing()
option(TESTING_ENABLED "Toggle all testing" ON)
option(WEPOLL_TESTING "Enable testing for the wepoll library" ON)
option(MINIAUDIO_TESTING "Enable testing for the miniaudio library" OFF)
option(NET_TESTING "Enable testing for Net library" ON)
option(RINGBUFFER_TESTING "Enable testing for ringbuffers" ON)

set(CMAKE_INSTALL_PREFIX "$CMAKE_CURRENT_SOURCE_DIR/_install")


add_subdirectory(ThirdParty) 

if ( CMAKE_COMPILER_IS_GNUCC )
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wfloat-equal -std=c++20)
endif()

if(MSVC)
    add_compile_options(/external:anglebrackets /external:W0 /Wall /WX /wd4324 /wd5027 /wd4514 /wd4625 /wd4626 /wd4711 /wd4710 /wd5045 /std:c++20)  
endif()


project(AUDIODESIGNER)

#wrapper for test registration which does nice cool things :)
function(lib_add_test test_target test_source test_libs_indirect)
    add_executable(${test_target} ${test_source})
    target_link_libraries(${test_target} PUBLIC "${${test_libs_indirect}}") #unpack test libs indirect into link libraries
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

add_subdirectory(ArgParser)
add_subdirectory(AudioEngine)
add_subdirectory(Plugins)
