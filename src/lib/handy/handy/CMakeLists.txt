project(handy)

set(${PROJECT_NAME}_HEADERS
    array_utils.h
    Guard.h
    random.h
    tuple_utils.h
    vector_utils.h
)

add_library(${PROJECT_NAME} INTERFACE)

add_library(ad::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Custom target to have the files show up in the IDE
add_custom_target(${PROJECT_NAME}_IDE
    SOURCES
        ${${PROJECT_NAME}_HEADERS}
)

cmc_target_current_include_directory(${PROJECT_NAME})


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
                          NAMESPACE ad::)
