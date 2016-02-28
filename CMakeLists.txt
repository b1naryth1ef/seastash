cmake_minimum_required(VERSION 3.2)
project(seastash)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

if ($ENV{WITH_PG})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
endif()

include_directories(src/)
include_directories(src/include/)
file(GLOB_RECURSE CORE_SOURCES src/*.c)
add_executable(seastash ${CORE_SOURCES})
target_link_libraries(seastash mill lua dl m rt)
