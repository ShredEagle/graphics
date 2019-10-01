project(2d)

set(${PROJECT_NAME}_HEADERS
    commons.h
    Engine.h
    Sprite.h
    Spriting.h
    Tiling.h

    dataformat/tiles.h
)

set(${PROJECT_NAME}_SOURCES
    Engine.cpp
    Spriting.cpp
    Tiling.cpp

    dataformat/tiles.cpp
)

find_package(glad CONFIG REQUIRED)
find_package(math CONFIG REQUIRED)
find_package(nlohmann_json CONFIG 3.7.0 REQUIRED)

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
        renderer # TODO: make private?
        ad::math
    PRIVATE 
        nlohmann_json::nlohmann_json
        handy
        resource
)
