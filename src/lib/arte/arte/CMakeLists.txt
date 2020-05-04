project(arte)

set(${PROJECT_NAME}_HEADERS
    Image.h

    details/ImageFormats/Netpbm.h
)

set(${PROJECT_NAME}_SOURCES
    Image.cpp
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

add_library(ad::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

cmc_target_current_include_directory(${PROJECT_NAME})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

set(_internal_dependencies_public handy platform)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ad::math
        ${_internal_dependencies_public})


##
## Install
##

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${PROJECT_NAME}/${PROJECT_NAME}
                         FILES ${${PROJECT_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${PROJECT_NAME} ${PROJECT_NAME}Targets
                          DEPENDS_COMPONENTS ${_internal_dependencies_public}
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::)
