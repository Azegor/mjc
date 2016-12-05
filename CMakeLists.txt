cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

cmake_policy(SET CMP0012 NEW)

project(mcj)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/install")
include_directories("${CMAKE_PREFIX_PATH}/include")
link_directories("${CMAKE_PREFIX_PATH}/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(DEBUG_FLAGS "${DEBUG_FLAGS} -Wextra -Werror -pedantic")
set(DEBUG_FLAGS "${DEBUG_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer -fno-common")
# works only with clang
#set(DEBUG_FLAGS "${DEBUG_FLAGS} -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common")

set(RELEASE_FLAGS "${RELEASE_FLAGS} -flto -march=native")

set(ENABLE_PROFILING OFF CACHE BOOL "Build with profiling flags")

if (${ENABLE_PROFILING})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  message(STATUS "Profiling enabled")
endif()

enable_testing()
add_subdirectory(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(Boost COMPONENTS program_options REQUIRED)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/thirdparty/cmake)

find_package(JeMalloc)

include(ExternalProject) # load module

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(ENV{CFLAGS} ${DEBUG_FLAGS}) # for firm
  set(firmVariant "debug")
else()
  set(firmVariant "optimize")
endif()

ExternalProject_Add(libfirm
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
  INSTALL_DIR "${CMAKE_PREFIX_PATH}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -j8
    -C "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
    "variant=${firmVariant}"
    "top_builddir=${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
  INSTALL_COMMAND make -j8
    -C "${CMAKE_CURRENT_SOURCE_DIR}/libfirm"
    "variant=${firmVariant}"
    "top_builddir=${CMAKE_CURRENT_BINARY_DIR}/libfirm/build"
    "PREFIX=/"
    "DESTDIR=${CMAKE_PREFIX_PATH}"
    "install"
)

# copied from libfirm.pc
# can't use that as it's not generated yet when cmake runs
set(libfirm_LIBRARIES "-lfirm -lm")


set(MJC_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/compiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/dotvisitor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/ast.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/symboltable.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/semantic_visitor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/firm_visitor.cpp"
)
add_executable(mjc
  ${MJC_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)
target_compile_definitions(mjc PRIVATE LIBSEARCHDIR="${CMAKE_BINARY_DIR}")

add_library(runtime STATIC src/runtime.c)

add_dependencies(mjc libfirm runtime)
target_link_libraries(mjc ${Boost_LIBRARIES} ${JEMALLOC_LIBRARIES} ${libfirm_LIBRARIES})

set(ENABLE_FUZZING OFF CACHE BOOL "Enable building off fuzzer binaries")

if (${ENABLE_FUZZING})
	message(STATUS "Fuzzer builds enabled")
  add_subdirectory(fuzzing)
endif()


if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set_property(TARGET mjc APPEND_STRING PROPERTY LINK_FLAGS "${DEBUG_FLAGS}")
  separate_arguments(DEBUG_FLAGS)
  set_property(TARGET mjc APPEND PROPERTY COMPILE_OPTIONS "${DEBUG_FLAGS}")

  include(CodeCoverage)
  setup_target_for_coverage(coverage "make -C ${PROJECT_SOURCE_DIR}/builddir test" coverage)
  add_dependencies(coverage mjc)
else()
  set_property(TARGET mjc APPEND_STRING PROPERTY LINK_FLAGS "${RELEASE_FLAGS}")
  separate_arguments(RELEASE_FLAGS)
  set_property(TARGET mjc APPEND PROPERTY COMPILE_OPTIONS "${RELEASE_FLAGS}")
endif()
