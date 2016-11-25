cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

include(ExternalProject) # load module

# create instrumented binary for afl
add_executable(mjc_instr $<TARGET_PROPERTY:mjc,SOURCES>)
target_link_libraries(mjc_instr $<TARGET_PROPERTY:mjc,LINK_LIBRARIES>)
set(RELEASE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
set_property(TARGET mjc_instr APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
separate_arguments(RELEASE_FLAGS)
set_property(TARGET mjc_instr APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")

# build afl-gcc binaries
ExternalProject_Add(AFL
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/afl/
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND : # does nothing, necessary since otherwise cmake is used
  BUILD_COMMAND make
  INSTALL_COMMAND :
  TEST_COMMAND :
)

if(EXISTS "/usr/bin/llvm-config")
  message(STATUS "using afl-clang-fast")
  #build afl-clang-fast binaries
  ExternalProject_Add(AFL_CLANG_FAST
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/afl/llvm_mode/
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND : # does nothing, necessary since otherwise cmake is used
    BUILD_COMMAND make
    INSTALL_COMMAND :
    TEST_COMMAND :
  )
  add_dependencies(AFL_CLANG_FAST AFL)
  add_dependencies(mjc_instr AFL_CLANG_FAST)

  set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-clang-fast)
  set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-clang-fast++)
else()
  add_dependencies(mjc_instr AFL)

  set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-gcc)
  set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-g++)
endif()

