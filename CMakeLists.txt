cmake_minimum_required(VERSION 3.2)
project(seastash)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories(src/)
include_directories(src/include/)
FILE(GLOB_RECURSE CORE_SOURCES src/*.c)
add_executable(seastash ${CORE_SOURCES})
target_link_libraries(seastash mill lua dl m)
