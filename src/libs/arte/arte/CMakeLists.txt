set(TARGET_NAME arte)

set(${TARGET_NAME}_HEADERS
    Freetype.h
    Image.h
    Logging.h
    SpriteSheet.h

    detail/GltfJson.h
    detail/Json.h
    detail/3rdparty/stb_image.h
    detail/3rdparty/stb_image_include.h
    detail/ImageFormats/Netpbm.h
    detail/ImageFormats/StbImageFormats.h

    gltf/Gltf.h
    gltf/Gltf-decl.h
    gltf/Owned.h
)

set(${TARGET_NAME}_SOURCES
    Image.cpp
    Logging.cpp
    SpriteSheet.cpp

    detail/3rdparty/stb_image.cpp

    gltf/Gltf.cpp
)

add_library(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}
             FILES ${${TARGET_NAME}_HEADERS} ${${TARGET_NAME}_SOURCES}
)

add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

cmc_target_current_include_directory(${TARGET_NAME})

cmc_cpp_all_warnings_as_errors(${TARGET_NAME})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        ad::handy
        ad::platform

        ad::math

        freetype
        nlohmann_json::nlohmann_json
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
