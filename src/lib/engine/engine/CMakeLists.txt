set(TARGET_NAME engine)

set(${TARGET_NAME}_HEADERS
    Application.h
    CameraUtilities.h
    commons.h
    Engine.h
    shaders.h
    Sprite.h
    Spriting.h
    Tiling.h
    Timer.h
    TrivialLineStrip.h
    TrivialShaping.h
    DrawLine.h
    Vertex.h

    dataformat/tiles.h
)

set(${TARGET_NAME}_SOURCES
    Engine.cpp
    Spriting.cpp
    Tiling.cpp
    TrivialLineStrip.cpp
    TrivialShaping.cpp
    DrawLine.cpp

    dataformat/tiles.cpp
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
        ad::renderer

        ad::math

        glad::glad
        glfw
    PRIVATE
        ad::handy
        ad::resource

        nlohmann_json::nlohmann_json)


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
