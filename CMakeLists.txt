# Minimum CMake version required
cmake_minimum_required(VERSION 3.18)
# Project name and version
project(clonerobotics VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(cxxopts)

include_directories(include/)

# build binaries
add_executable(consumer consumer_src/server.cpp)
add_executable(publisher publisher_src/client.cpp publisher_src/AFUnixPublisher.cpp)

target_include_directories(consumer PRIVATE consumer_src/)
target_include_directories(publisher PRIVATE publisher_src/)

target_link_libraries(consumer PRIVATE cxxopts)
target_link_libraries(publisher PRIVATE cxxopts)
