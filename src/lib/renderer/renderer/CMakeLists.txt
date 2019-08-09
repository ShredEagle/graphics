project(renderer)

set(${PROJECT_NAME}_HEADERS
    Error.h
    Shading.h
    Texture.h
    VertexSpecification.h

    stb_image.h
)

set(${PROJECT_NAME}_SOURCES
    Shading.cpp

    stb_image.c
)

find_package(glad REQUIRED)
find_package(math REQUIRED)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    INTERFACE
        $<INSTALL_INTERFACE:include/>
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        handy
        ad::math
        glad::glad
)
