cmake_minimum_required(VERSION 3.16)
project(PackageTest CXX)

find_package(Graphics REQUIRED COMPONENTS arte graphics renderer)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE ad::arte ad::graphics ad::renderer)
