project(handy)

set(${PROJECT_NAME}_HEADERS
    array_utils.h
    Guard.h
    random.h
    tuple_utils.h
    vector_utils.h
)

add_library(${PROJECT_NAME} INTERFACE)

# Custom target to have the files show up in the IDE
add_custom_target(${PROJECT_NAME}_IDE
    SOURCES
        ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
        $<INSTALL_INTERFACE:include/>
)
