set(TARGET_NAME renderer)

set(${TARGET_NAME}_HEADERS
    commons.h
    Drawing.h
    Error.h
    FrameBuffer.h
    gl_helpers.h
    GL_Loader.h
    MappedGL.h
    Range.h
    Shading.h
    Texture.h
    Uniforms.h
    VertexSpecification.h
    VertexSpecification_deduction.h

    stb_image.h
)

set(${TARGET_NAME}_SOURCES
    Shading.cpp
    VertexSpecification.cpp

    stb_image.c
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}
             FILES ${${TARGET_NAME}_HEADERS} ${${TARGET_NAME}_SOURCES}
)

add_library(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

cmc_target_current_include_directory(${TARGET_NAME})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        ad::arte
        ad::handy
        ad::math

        gsl::gsl
        glad::glad
)


##
## Install
##

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${TARGET_NAME}/${TARGET_NAME}
                         FILES ${${TARGET_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::)
