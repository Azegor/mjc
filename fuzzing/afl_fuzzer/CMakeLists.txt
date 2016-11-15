cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(fuzzmjc_afl)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include(ExternalProject) # load module

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
  set(HAVE_CLANG_FAST 1)
#   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
endif()

find_package(Boost COMPONENTS program_options REQUIRED)

# create instrumented binary for afl
add_executable(mjc_instr ../../src/main.cpp ../../src/compiler.cpp ../../src/lexer.cpp ../../src/parser.cpp ../../src/dotvisitor.cpp ../../src/ast.cpp)

target_link_libraries(mjc_instr ${Boost_LIBRARIES})

if (${HAVE_CLANG_FAST})
  add_dependencies(mjc_instr AFL_CLANG_FAST)

  set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-clang-fast)
  set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-clang-fast++)
else()
  add_dependencies(mjc_instr AFL)

  set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-gcc)
  set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-g++)
endif()
