cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

include(ExternalProject) # load module

# needs to be clang!
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_C_COMPILER /usr/bin/clang-4.0)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++-4.0)

# necessary for Fuzzer lib to build
set(LLVM_USE_SANITIZER Address)
set(LLVM_USE_SANITIZE_COVERAGE YES)

add_subdirectory(Fuzzer)

# create instrumented binary for llvmfuzzer
add_executable(fuzzmjc
  "${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cpp"
  ${MJC_SOURCES}
)
add_dependencies(fuzzmjc LLVMFuzzer)
target_link_libraries(fuzzmjc
  $<TARGET_PROPERTY:mjc,LINK_LIBRARIES>
  $<TARGET_FILE:LLVMFuzzer>
)

set(RELEASE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
set(RELEASE_FLAGS "${RELEASE_FLAGS} -fsanitize=address,undefined -fsanitize-coverage=edge")
# -fsanitize=address,undefined -fsanitize-coverage=trace-pc-guard
# -fsanitize=memory,undefined,safe-stack -fsanitize-coverage=edge,trace-cmp,indirect-calls,8bit-counters

set_property(TARGET LLVMFuzzerNoMainObjects APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
set_property(TARGET LLVMFuzzerNoMain APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
set_property(TARGET LLVMFuzzer APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
set_property(TARGET fuzzmjc APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
separate_arguments(RELEASE_FLAGS)
set_property(TARGET LLVMFuzzerNoMainObjects APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")
set_property(TARGET LLVMFuzzerNoMain APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")
set_property(TARGET LLVMFuzzer APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")
set_property(TARGET fuzzmjc APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")

