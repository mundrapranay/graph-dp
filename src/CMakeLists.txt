cmake_minimum_required(VERSION 3.10)
project(DistributedGraphAlgorithm)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
find_package(OpenSSL REQUIRED)

add_subdirectory(abseil-cpp)

set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_STANDARD 17)

add_executable(DistributedGraphAlgorithm KCore.cpp Graph.h LDS.h distributions.h)
target_link_libraries(DistributedGraphAlgorithm ${MPI_CXX_LIBRARIES} OpenSSL::SSL absl::status absl::base absl::synchronization)
