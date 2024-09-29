project(AUDIOENGINE)

add_library(AudioEngine STATIC)
set(AudioEngine_DIR "$CMAKE_CURRENT_SOURCE_DIR" PARENT_SCOPE)

set_property(TARGET AudioEngine PROPERTY CXX_STANDARD 20) #C++ 20

target_include_directories(AudioEngine PUBLIC include)

target_link_libraries(AudioEngine PUBLIC wepoll)

add_subdirectory(src)



#wrapper for test registration which does nice cool things :)
function(lib_add_test test_target test_source test_libs_indirect)
  add_executable(${test_target} ${test_source})
  set_property(TARGET ${test_target} PROPERTY CXX_STANDARD 20) #C++ 20
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

endfunction()

if (NET_TESTING)
    #test net stuff here
    set(NET_TEST_LIBS AudioEngine)
    lib_add_test(net_parse_endpoint "${CMAKE_CURRENT_SOURCE_DIR}/tests/Net/parse_endpoint.cpp" NET_TEST_LIBS)
    lib_add_test(net_get_endpoint "${CMAKE_CURRENT_SOURCE_DIR}/tests/Net/get_endpoint.cpp" NET_TEST_LIBS)
    lib_add_test(net_addr_ipv4_str "${CMAKE_CURRENT_SOURCE_DIR}/tests/Net/address_ipv4_str.cpp" NET_TEST_LIBS)
endif()