cmake_minimum_required(VERSION 3.21)
#This script handles the external projects

find_package(Git REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(EXT_PROJECTS "")

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_PROGRESS TRUE
    GIT_TAG 0.11.21
    TMP_DIR ${CMAKE_SOURCE_DIR}/_build/tmp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio
)
list(APPEND EXT_PROJECTS "miniaudio")

if (WIN32)

    #TODO: Use PATCH_COMMAND instead of my hacked together execute_process
    FetchContent_Declare(
        wepoll
        GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
        GIT_PROGRESS TRUE
        GIT_TAG 7958b3048fa1e85ab5a71f18a07c24e1b1e64f1e #v1.5.8 with some API fixes
        TMP_DIR ${CMAKE_SOURCE_DIR}/_build/tmp
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wepoll
        BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/wepoll-build
    )
    #list(APPEND EXT_PROJECTS "wepoll")

    FetchContent_GetProperties(wepoll)
    if (NOT wepoll_POPULATED AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wepoll")
        message("Fetching wepoll...")
        FetchContent_Populate(wepoll)

        #patch out the abominable wepoll build system with my own
        execute_process(
            COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace --verbose "${CMAKE_CURRENT_SOURCE_DIR}/wepoll-utf8.patch"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/wepoll"
        )

        FetchContent_GetProperties(wepoll)
    endif()

    add_subdirectory(${wepoll_SOURCE_DIR} ${wepoll_BINARY_DIR})
endif()

FetchContent_MakeAvailable(${EXT_PROJECTS})