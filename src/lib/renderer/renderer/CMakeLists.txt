project(renderer)

set(${PROJECT_NAME}_HEADERS
    commons.h
    Drawing.h
    Error.h
    FrameBuffer.h
    gl_helpers.h
    Image.h
    MappedGL.h
    Range.h
    Shading.h
    Texture.h
    VertexSpecification.h

    stb_image.h
)

set(${PROJECT_NAME}_SOURCES
    Shading.cpp
    VertexSpecification.cpp

    stb_image.c
)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

cmc_target_current_include_directory(${PROJECT_NAME})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

set(_internal_dependencies_public gsl handy)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ad::math
        glad::glad
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
