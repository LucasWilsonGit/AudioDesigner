# build_defs.cmake
#
# Defines some functions and variables to make compiling easier for me. Specifically so far I've had to deal with MSVC not being properly detected
# in a minimum setup because I do not want to have Visual Studio on my system, so I do not have cl.exe 
# MSBuild is all that is ever needed for building VC++ and having the entire CMake toolchain detection based off of cl.exe is ridiculous
# I do not want to install Visual Studio, I debug with gdb or an external debugger that doesn't take multiple seconds to update watch variables etc

function(detect_msvc RESULT_VAR)
    if (MSVC)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
        return()
    endif()

    if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
        return()
    endif()

    find_program(MSVC_COMPILER cl.exe)
    if (MSVC_COMPILER AND NOT "${MSVC_COMPILER}" MATCHES "NOTFOUND")
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
        return()
    endif()
endfunction()



function(setup_global_compiler_options)
    detect_msvc(MSVC)

    if ( CMAKE_COMPILER_IS_GNUCC )
        if ( CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo" )
            set(CMAKE_BUILD_TYPE "Release")
            add_compile_options(-O3)
        endif()

        if (CMAKE_BUILD_TYPE MATCHES "Debug|RelWithDebInfo" )
            add_compile_options(-g)
        endif()
    endif()

    if(MSVC)
        if ( CMAKE_BUILD_TYPE MATCHES "Release" ) 
            set(CMAKE_BUILD_TYPE "Release")
            add_compile_options("/O2")
        endif()

        if (CMAKE_BUILD_TYPE MATCHES "Debug" )
            add_compile_options(/Z7 /MTd)
            add_link_options("/DEBUG")
        endif()

        if ( CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" )
            add_compile_options(/Zi /Zo /MTd)
            add_link_options("/DEBUG")
        endif()
    endif()
endfunction()



set(WINDOWS_PLATFORMS "Windows" "MSYS" "CYGWIN")
list(FIND WINDOWS_PLATFORMS "MSYS" audiodesigner_platform_index)
function(iswindows)
    if (NOT "${audiodesigner_platform_index}" STREQUAL "-1" )
        set(ISWINDOWS 1 PARENT_SCOPE)
        return()
    endif()
    set(ISWINDOWS 0 PARENT_SCOPE)
endfunction()