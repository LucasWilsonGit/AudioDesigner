project(AUDIOENGINE)

add_library(AudioEngine STATIC)
set(AudioEngine_DIR "$CMAKE_CURRENT_SOURCE_DIR" PARENT_SCOPE)

set_property(TARGET AudioEngine PROPERTY CXX_STANDARD 20) #C++ 20

target_include_directories(AudioEngine PUBLIC include)

target_link_libraries(AudioEngine PUBLIC wepoll)

add_subdirectory(src)

if (TESTING_ENABLED AND NET_TESTING)
    #test net stuff here
    set(NET_TEST_LIBS AudioEngine)
    iswindows()
    if (${LASTCALL})
        set(NET_TEST_LIBS "${NET_TEST_LIBS}" ws2_32)
    endif()

    file(GLOB_RECURSE NET_TEST_SOURCES "tests/Net/*.cpp")

    foreach(test_source IN LISTS NET_TEST_SOURCES)
        get_filename_component(net_test_name ${test_source} NAME_WE)

        lib_add_test("Net_${net_test_name}" "${test_source}" NET_TEST_LIBS)
    endforeach()
endif()