project(platform CXX)

set(${PROJECT_NAME}_HEADERS
    Filesystem.h
)

# NOTE should evolve into a compiled module
add_library(${PROJECT_NAME} INTERFACE)

add_library(ad::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Custom target to have the files show up in the IDE
add_custom_target(${PROJECT_NAME}_IDE
    SOURCES
        ${${PROJECT_NAME}_HEADERS}
)

cmc_target_current_include_directory(${PROJECT_NAME})

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        Boost::filesystem)


##
## Install
##

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${PROJECT_NAME}/${PROJECT_NAME}
                         FILES ${${PROJECT_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${PROJECT_NAME} ${PROJECT_NAME}Targets
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::)
