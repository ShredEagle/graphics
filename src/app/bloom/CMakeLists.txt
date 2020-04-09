project(bloom)

set(${PROJECT_NAME}_HEADERS
    Scene.h
    shaders.h
    vertices.h
)

set(${PROJECT_NAME}_SOURCES
    main.cpp
)

find_package(glad CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glad::glad
        ad::engine
        ad::renderer
        ad::resource
)
