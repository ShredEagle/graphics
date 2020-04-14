cmake_minimum_required(VERSION 3.14)

project(Graphics)

include(cmake/include.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(LINK_OPTIONS "/INCREMENTAL:NO")
endif()

# Install the top-level package config, allowing to find all sub-components
include(cmc-install)
cmc_install_root_component_config()

add_subdirectory(src)
