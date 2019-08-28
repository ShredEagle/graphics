cmake_minimum_required(VERSION 3.12)

project(2D)

include(cmake/include.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies via Conan
if(EXISTS "${CMAKE_BINARY_DIR}/conan_paths.cmake")
    include("${CMAKE_BINARY_DIR}/conan_paths.cmake")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(LINK_OPTIONS "/INCREMENTAL:NO")
endif()

add_subdirectory(src)
