project(AUDIOENGINE)

add_library(AudioEngine STATIC)
set(AudioEngine_DIR "$CMAKE_CURRENT_SOURCE_DIR" PARENT_SCOPE)

set_property(TARGET AudioEngine PROPERTY CXX_STANDARD 20) #C++ 20

target_include_directories(AudioEngine PUBLIC include "${CMAKE_CURRENT_SOURCE_DIR}/../ext")

target_link_libraries(AudioEngine PUBLIC wepoll)

add_subdirectory(src)

# Define a function to generalize the test setup process
function(add_tests_from_folder TEST_NAME_PREFIX TEST_FOLDER LIBS_VAR_NAME)
    # Gather all source files in the specified test folder
    file(GLOB_RECURSE TEST_SOURCES "${TEST_FOLDER}/*.cpp")
    
    # Loop through each source file and create a test target for it
    foreach(test_source IN LISTS TEST_SOURCES)
        # Get the test name from the filename without extension
        get_filename_component(test_name ${test_source} NAME_WE)
        
        # Call your custom add test function with the constructed test name, source file, and libraries
        lib_add_test("${TEST_NAME_PREFIX}_${test_name}" "${test_source}" ${LIBS_VAR_NAME})
    endforeach()
endfunction()


if (TESTING_ENABLED)

    if (NET_TESTING)
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

    if (RINGBUFFER_TESTING)

        #test net stuff here
        set(RB_TEST_LIBS AudioEngine)

        file(GLOB_RECURSE RB_TEST_SOURCES "tests/ring_buffer/*.cpp")

        foreach(test_source IN LISTS RB_TEST_SOURCES)
            get_filename_component(rb_test_name ${test_source} NAME_WE)

            lib_add_test("ring_buffer_${rb_test_name}" "${test_source}" RB_TEST_LIBS)
        endforeach()

    endif()


    
    set(ALLOC_TEST_LIBS AudioEngine)
    file(GLOB_RECURSE ALLOC_TEST_SOURCES "tests/block_allocator/*.cpp")
    foreach(test_source IN LISTS ALLOC_TEST_SOURCES)
        get_filename_component(rb_test_name ${test_source} NAME_WE)

        lib_add_test("ring_buffer_${rb_test_name}" "${test_source}" ALLOC_TEST_LIBS)
    endforeach()

    set(SPARSE_TEST_LIBS AudioEngine)
    file(GLOB_RECURSE SPARSE_TEST_SOURCES "tests/sparse_collection/*.cpp")
    foreach(test_source IN LISTS SPARSE_TEST_SOURCES)
        get_filename_component(sparse_test_name ${test_source} NAME_WE)

        lib_add_test("sparse_collection_${sparse_test_name}" "${test_source}" SPARSE_TEST_LIBS)
    endforeach()

    set(BUFFER_READER_TEST_LIBS AudioEngine)
    file(GLOB_RECURSE BUFFER_READER_TEST_SOURCES "tests/buffer_reader/*.cpp")
    foreach(test_source IN LISTS BUFFER_READER_TEST_SOURCES)
        get_filename_component(buffer_reader_test_name ${test_source} NAME_WE)

        lib_add_test("buffer_reader_${buffer_reader_test_name}" "${test_source}" BUFFER_READER_TEST_LIBS)
    endforeach()

    set(PERF_TEST_LIBS AudioEngine)
    file(GLOB_RECURSE PERF_TEST_SOURCES "tests/perf/*.cpp")
    foreach(test_source IN LISTS PERF_TEST_SOURCES)
        get_filename_component(perf_test_name ${test_source} NAME_WE)

        lib_add_test("perf_${perf_test_name}" "${test_source}" PERF_TEST_LIBS)
    endforeach()
endif()