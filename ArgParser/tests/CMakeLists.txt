cmake_minimum_required(VERSION 3.14)

include(GoogleTest)

set(GTEST_LIST "")
function(add_gtest test_name test_source)

    #token_stream
    add_executable("${test_name}"
        "${test_source}"
    )
    target_compile_options("${test_name}"
    PUBLIC
        /Zd /Zi    
    )
    target_link_libraries("${test_name}"
        GTest::gtest_main
    )
    target_include_directories("${test_name}"
    PRIVATE
        ../include/
        ../../AudioEngine/exportheaders #this is terrible
    )

    gtest_discover_tests("${test_name}")

endfunction()

add_gtest(token_stream token_stream.cpp)
add_gtest(arg_parser arg_parser.cpp)

