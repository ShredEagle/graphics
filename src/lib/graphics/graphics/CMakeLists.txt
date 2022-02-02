set(TARGET_NAME graphics)

set(${TARGET_NAME}_HEADERS
    AppInterface.h
    ApplicationGlfw.h
    CameraUtilities.h
    Curving.h
    Curving-shaders.h
    commons.h
    shaders.h
    Sprite.h
    SpriteAnimator.h
    SpriteLoading.h
    Spriting.h
    Texting.h
    Tiling.h
    Timer.h
    TrivialLineStrip.h
    TrivialPolygon.h
    TrivialShaping.h
    DrawLine.h
    Vertex.h

    adapters/ParallaxScroller.h

    detail/Logging.h
    detail/UnitQuad.h
    detail/GlyphUtilities.h

    effects/Fade.h
    effects/Fade-shaders.h
    effects/GaussianBlur.h
    effects/GaussianBlur-shaders.h
)

set(${TARGET_NAME}_SOURCES
    Curving.cpp
    AppInterface.cpp
    SpriteAnimator.cpp
    SpriteLoading.cpp
    Spriting.cpp
    Texting.cpp
    Tiling.cpp
    TrivialLineStrip.cpp
    TrivialPolygon.cpp
    TrivialShaping.cpp
    DrawLine.cpp

    adapters/ParallaxScroller.cpp

    detail/Logging.cpp
    detail/UnitQuad.cpp
    detail/GlyphUtilities.cpp

    effects/Fade.cpp
    effects/GaussianBlur.cpp
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
        ad::renderer

        ad::math

        glad::glad
        glfw
        utf8cpp # part of header of Texting.h
    PRIVATE
        ad::handy
        ad::resource

        spdlog::spdlog
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
