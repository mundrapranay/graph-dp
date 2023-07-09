cmake_minimum_required(VERSION 3.0)
project(DistributedGraphAlgorithm)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_STANDARD 11)

add_executable(DistributedGraphAlgorithm Kcore.cpp Graph.h LDS.h)
target_link_libraries(DistributedGraphAlgorithm ${MPI_CXX_LIBRARIES})
