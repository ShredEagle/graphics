project(tiler)

set(${PROJECT_NAME}_HEADERS
    scene.h
    Terrain.h
)

set(${PROJECT_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
) 

target_link_libraries(${PROJECT_NAME}
	PRIVATE
        glfw
        2d
        resource
)
