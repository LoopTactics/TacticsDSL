cmake_minimum_required(VERSION 3.16.0)

add_executable("dsltest" dsltest.cpp)
target_link_libraries("dsltest" dsl)
target_link_libraries("dsltest" gtest_main)
target_include_directories("dsltest" PRIVATE ${CMAKE_SOURCE_DIR})

add_custom_target(check COMMAND "dsltest")
