cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(MJC_Fuzzer)

include(ExternalProject) # load module

ExternalProject_Add(AFL_FUZZER
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/afl_fuzzer/
  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/afl_fuzzer/build
  INSTALL_COMMAND :
  TEST_COMMAND :
)

ExternalProject_Add(LLVM_FUZZER
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/llvm_fuzzer/
  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/llvm_fuzzer/build
  INSTALL_COMMAND :
  TEST_COMMAND :
)

# add_subdirectory(afl_fuzzer)
# add_subdirectory(llvm_fuzzer)
