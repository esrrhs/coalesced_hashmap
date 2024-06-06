cmake_minimum_required(VERSION 2.8)
project(coalesced_hashmap_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DNDEBUG -std=c++11 -O2 -g3 -ggdb -fPIC")

aux_source_directory(./ SRC_LIST)

add_executable(coalesced_hashmap_test ${SRC_LIST})
