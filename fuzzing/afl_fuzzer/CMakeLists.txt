cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(fuzzmjc_afl)

# change to afl compiler for following targets
set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-gcc)
set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/afl/afl-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include(ExternalProject) # load module

# build afl binaries
ExternalProject_Add(AFL
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/afl/
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND : # does nothing, necessary since otherwise cmake is used
  BUILD_COMMAND make
  INSTALL_COMMAND :
  TEST_COMMAND :
)

find_package(Boost COMPONENTS program_options REQUIRED)

# create instrumented binary for afl
add_executable(mjc_instr ../../src/main.cpp ../../src/compiler.cpp ../../src/lexer.cpp)

target_link_libraries(mjc_instr ${Boost_LIBRARIES})

add_dependencies(mjc_instr AFL)
