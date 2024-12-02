cmake_minimum_required(VERSION 3.21)
#This script handles the external projects

find_package(Git REQUIRED)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(EXT_PROJECTS "")
set(EXT_PROJECT_SOURCES "")

FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_PROGRESS TRUE
    GIT_TAG 0.11.21
    TMP_DIR ${CMAKE_BINARY_DIR}/tmp/miniaudio
)
#changing the SOURCE_DIR means I have to populate it manually before I can let MakeAvailable do the regular update and configure 
list(APPEND EXT_PROJECTS "miniaudio")


iswindows()
if (${LASTCALL})
    #TODO: Use PATCH_COMMAND instead of my hacked together execute_process
    FetchContent_Declare(
        wepoll
        GIT_REPOSITORY https://github.com/piscisaureus/wepoll.git
        GIT_PROGRESS TRUE
        GIT_TAG 7958b3048fa1e85ab5a71f18a07c24e1b1e64f1e #v1.5.8 with some API fixes
        TMP_DIR ${CMAKE_BINARY_DIR}/tmp/wepoll
        PATCH_COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace --verbose "${CMAKE_CURRENT_SOURCE_DIR}/wepoll-utf8.patch"
        SOURCE_DIR dist/wepoll
    )
    list(APPEND EXT_PROJECTS "wepoll")
endif()

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_STANDARD=20
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)
FetchContent_MakeAvailable(${EXT_PROJECTS})
if (NOT EXISTS "${wepoll_SOURCE_DIR}/patched.ad")
    message("Patching wepoll...")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace --verbose "${CMAKE_CURRENT_SOURCE_DIR}/wepoll-utf8.patch"
        WORKING_DIRECTORY "${wepoll_SOURCE_DIR}"
    )
    file(WRITE "${wepoll_SOURCE_DIR}/patched.ad" "yes")
endif()


list(APPEND EXT_PROJECT_SOURCES "${miniaudio_SOURCE_DIR}")
list(APPEND EXT_PROJECT_SOURCES "${wepoll_SOURCE_DIR}")
set(EXT_PROJECT_SOURCES "${EXT_PROJECT_SOURCES}" PARENT_SCOPE)