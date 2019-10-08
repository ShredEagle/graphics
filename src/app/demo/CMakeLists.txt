project(demo)

set(${PROJECT_NAME}_HEADERS
    Render.h
)

set(${PROJECT_NAME}_SOURCES
    main.cpp
    Render.cpp
)

find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
) 

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glad::glad
        glfw
)
