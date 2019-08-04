project(handy)

set(${PROJECT_NAME}_HEADERS
    Guard.h
)

add_library(${PROJECT_NAME} INTERFACE)

# To have the files show up in the IDE
add_custom_target(${PROJECT_NAME}_IDE
    SOURCES
        ${${PROJECT_NAME}_HEADERS}
        ${${PROJECT_NAME}_DOXYGEN}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:include/>
)
