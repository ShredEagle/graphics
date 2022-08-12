set(TARGET_NAME simple)

set(${TARGET_NAME}_HEADERS
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        glfw
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
