cmake_minimum_required(VERSION 2.6)

# add_definitions( -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS )

project(mcj)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer -fno-common")
# works only with clang
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common")

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  endif()
  add_subdirectory(fuzzing)
endif()

enable_testing()
add_subdirectory(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(mjc src/main.cpp src/compiler.cpp)

target_link_libraries(mjc ${Boost_LIBRARIES})
