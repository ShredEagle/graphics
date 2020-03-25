cmake_minimum_required(VERSION 3.16)
project(PackageTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>)
find_package(2d REQUIRED COMPONENTS engine renderer)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE ad::engine ad::renderer)
