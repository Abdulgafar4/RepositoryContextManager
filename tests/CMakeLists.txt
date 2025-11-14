macro(add_repoctx_test TEST_NAME)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} 
        GTest::gtest_main
        repoctx_lib
    )
    gtest_discover_tests(${TEST_NAME})
endmacro()

add_repoctx_test(filter_test)
add_repoctx_test(file_stats)
add_repoctx_test(utils_test)
add_repoctx_test(git_info_test)