cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(mcj)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer -fno-common")
# works only with clang
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/install")
include_directories("${CMAKE_PREFIX_PATH}/include")
link_directories("${CMAKE_PREFIX_PATH}/lib")

set(ENABLE_FUZZING OFF CACHE BOOL "Enable building off fuzzer binaries")

if (${ENABLE_FUZZING})
	message(STATUS "Fuzzer builds enabled")
  add_subdirectory(fuzzing)
endif()

enable_testing()
add_subdirectory(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/thirdparty/cmake)

find_package(JeMalloc)

include(ExternalProject) # load module

ExternalProject_Add(libfirm
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
  INSTALL_DIR "${CMAKE_PREFIX_PATH}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -j
    -C "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
    "variant=optimize"
    "top_builddir=${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
  INSTALL_COMMAND make -j
    -C "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
    "variant=optimize"
    "top_builddir=${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
    "PREFIX=/"
    "DESTDIR=${CMAKE_PREFIX_PATH}"
    "install"
)

# copied from libfirm.pc
# can't use that as it's not generated yet when cmake runs
set(libfirm_LIBRARIES "-lfirm -lm")

add_executable(mjc
  src/main.cpp
  src/compiler.cpp
  src/lexer.cpp
  src/parser.cpp
  src/dotvisitor.cpp
  src/ast.cpp
  src/symboltable.cpp
  src/semantic_visitor.cpp
  src/firm_visitor.cpp
)

add_dependencies(mjc libfirm)
target_link_libraries(mjc ${Boost_LIBRARIES} ${JEMALLOC_LIBRARIES} ${libfirm_LIBRARIES})


if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include(CodeCoverage)
  setup_target_for_coverage(coverage "make -C ${PROJECT_SOURCE_DIR}/builddir test" coverage)
  add_dependencies(coverage mjc)
endif()
