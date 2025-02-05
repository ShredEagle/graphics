cmake_minimum_required(VERSION 3.20)

project(Graphics VERSION "${BUILD_VERSION}")

include(cmake/include.cmake)

cmc_include_conan_configuration()

# Enable the grouping target in folders, when available in IDE.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# see: https://cmake.org/cmake/help/latest/module/CTest.html
# To be included in the top CMakeLists.txt, creates BUILD_TESTING option (ON by default)
# Invokes enable_testing() to enable add_test(), unless BUILD_TESTING is OFF.
# note: Conan can set BUILD_TESTING to OFF through tools.build:skip_test configuration.
include(CTest)

# Install the top-level package config, allowing to find all sub-components
include(cmc-install)
cmc_install_root_component_config(${PROJECT_NAME})
cmc_register_source_package(${PROJECT_NAME})

include(cmc-cpp)
cmc_cpp_define_sanitizer_enum()

add_subdirectory(src)
