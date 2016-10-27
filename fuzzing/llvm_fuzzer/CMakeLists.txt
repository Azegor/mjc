cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(fuzzmjc_llvm)

# needs to be clang!
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fsanitize-coverage=edge")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory,undefined,safe-stack -fsanitize-coverage=edge,trace-cmp,indirect-calls,8bit-counters") # all options

# necessary for Fuzzer lib to build
set(LLVM_USE_SANITIZER Address)
set(LLVM_USE_SANITIZE_COVERAGE YES)

# build Fuzzer library
add_subdirectory(Fuzzer)

find_package(Boost COMPONENTS program_options REQUIRED)

# create fuzzer binary
add_executable(fuzzmjc fuzz.cpp ../../src/compiler.cpp ../../src/lexer.cpp)

target_link_libraries(fuzzmjc LLVMFuzzer ${Boost_LIBRARIES})
