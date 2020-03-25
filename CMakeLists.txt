project(dsl)
cmake_minimum_required(VERSION 3.17.0)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -O0 -g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Arch: ${CMAKE_SYSTEM_PROCESSOR}")

add_library(dsl
  dsl/parser.cpp
  dsl/lexer.cpp  
)

add_executable(main
  main.cpp
)

target_link_libraries(main dsl)
