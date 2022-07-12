set(TARGET_NAME imgui)

set(${TARGET_NAME}_HEADERS
    ImguiUi.h
)

set(${TARGET_NAME}_SOURCES
    ImguiUi.cpp
)

set(${TARGET_NAME}_IMGUI_BACKENDS
    ${PROJECT_BINARY_DIR}/conan_imports/imgui_backends/imgui_impl_glfw.cpp
    ${PROJECT_BINARY_DIR}/conan_imports/imgui_backends/imgui_impl_opengl3.cpp
    ${PROJECT_BINARY_DIR}/conan_imports/imgui_backends/imgui_impl_glfw.h
    ${PROJECT_BINARY_DIR}/conan_imports/imgui_backends/imgui_impl_opengl3.h
    ${PROJECT_BINARY_DIR}/conan_imports/imgui_backends/imgui_impl_opengl3_loader.h
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}
             FILES ${${TARGET_NAME}_HEADERS} ${${TARGET_NAME}_SOURCES}
)

add_library(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
    ${${TARGET_NAME}_IMGUI_BACKENDS}
)

add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/conan_imports>
)


cmc_target_current_include_directory(${TARGET_NAME})


##
## Dependencies
##

cmc_find_dependencies(FILE CMakeFinds.cmake.in)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::graphics
    PUBLIC
        imgui::imgui
)


##
## Install
##

install(TARGETS ${TARGET_NAME}
        EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
# Install the header files
cmc_install_with_folders(DESTINATION include/${TARGET_NAME}/${TARGET_NAME}
                         FILES ${${TARGET_NAME}_HEADERS})
# Prepare CMake package in both build and install trees
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::)
