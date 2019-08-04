project(renderer)

set(${PROJECT_NAME}_HEADERS
    Handy.h
    Shading.h
    VertexSpecification.h
)

set(${PROJECT_NAME}_SOURCES
    Shading.cpp
)

find_package(glad REQUIRED)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    #INTERFACE
    #    $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glad::glad
)
