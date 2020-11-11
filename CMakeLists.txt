project(dsl)
include(ExternalProject)

cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -pedantic -O0 -g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Arch: ${CMAKE_SYSTEM_PROCESSOR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

#ExternalProject_Add(llvm-project
#  PREFIX external/llvm-project
#  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/llvm-project
#  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/build
#  INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
#  GIT_REPOSITORY https://github.com/llvm/llvm-project.git
#  GIT_TAG master  
#  GIT_PROGRESS 1
#  #PATCH_COMMAND mkdir <BINARY_DIR> && mkdir <INSTALL_DIR>
#  CMAKE_GENERATOR Ninja
#  CMAKE_ARGS
#  ${CMAKE_CURRENT_SOURCE_DIR}/external/llvm-project/llvm
#  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
#  -DLLVM_ENABLE_PROJECTS=mlir
#  -DLLVM_BUILD_EXAMPLES=ON
#  -DLLVM_TARGETS_TO_BUILD="X86;NVPTX;AMDGPU"
#  -DCMAKE_BUILD_TYPE=Release
#  -DLLVM_ENABLE_ASSERTIONS=ON
#  TEST_AFTER_INSTALL 1
#)

# LLVM used to get cache configuration via TTI.
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})

# Google test.
add_subdirectory(external/googletest EXCLUDE_FROM_ALL)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs 
  core nativecodegen ExecutionEngine tablegen Support)

add_library(dsl
  dsl/parser.cpp
  dsl/lexer.cpp 
  dsl/matchers.cpp
  dsl/emitter.cpp 
)

target_link_libraries(dsl ${llvm_libs})

add_executable(main
  main.cpp
)

target_link_libraries(main dsl)

add_subdirectory(test)
