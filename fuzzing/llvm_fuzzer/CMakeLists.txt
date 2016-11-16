cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

include(ExternalProject) # load module

project(fuzzmjc_llvm)

# needs to be clang!
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_C_COMPILER /usr/bin/clang-4.0)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++-4.0)

set(CMAKE_BUILD_TYPE "Release")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fsanitize-coverage=edge")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fsanitize-coverage=trace-pc-guard")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory,undefined,safe-stack -fsanitize-coverage=edge,trace-cmp,indirect-calls,8bit-counters") # all options

# necessary for Fuzzer lib to build
set(LLVM_USE_SANITIZER Address)
set(LLVM_USE_SANITIZE_COVERAGE YES)

# build Fuzzer library
# add_subdirectory(Fuzzer)

ExternalProject_Add(LLVMFuzzer
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND : # does nothing, necessary since otherwise cmake is used
  BUILD_COMMAND ./build.sh
  INSTALL_COMMAND :
  TEST_COMMAND :
)

# add_library(LLVMFuzzer STATIC IMPORTED ${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/libFuzzer.a)

find_package(Boost COMPONENTS program_options REQUIRED)

# create fuzzer binary
add_executable(fuzzmjc fuzz.cpp ../../src/compiler.cpp ../../src/lexer.cpp  ../../src/parser.cpp ../../src/dotvisitor.cpp ../../src/ast.cpp ../../src/semantic_visitor.cpp)

add_dependencies(fuzzmjc LLVMFuzzer)


# target_compile_options(fuzzmjc PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address,undefined -fsanitize-coverage=trace-pc-guard>)

# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/)

target_link_libraries(fuzzmjc ${Boost_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/libFuzzer.a)
