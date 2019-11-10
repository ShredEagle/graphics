project(chader)

set(${PROJECT_NAME}_HEADERS
    Chader.h
    GenericDrawer.h
)

set(${PROJECT_NAME}_SOURCES
    Chader.cpp
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
        engine
        renderer
)
