# Minimum CMake version required
cmake_minimum_required(VERSION 3.18)
# Project name and version
project(clonerobotics VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include/)

#add_subdirectory(server/)
#add_subdirectory(client/)

# build binaries
add_executable(server server_src/server.cpp)
add_executable(client client_src/client.cpp)

target_include_directories(server PRIVATE server_src/)
target_include_directories(client PRIVATE client_src/)
