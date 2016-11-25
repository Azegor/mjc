cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(MJC_Fuzzer)

include(ExternalProject) # load module

add_subdirectory(afl_fuzzer)
add_subdirectory(llvm_fuzzer)
