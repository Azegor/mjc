cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(mcj)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer -fno-common")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common") # works only with clang
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_executable(mjc main.cpp)

add_subdirectory(fuzzing)
