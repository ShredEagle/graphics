set(TARGET_NAME doubledraw)

set(${TARGET_NAME}_HEADERS
    scene.h
    shaders.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)

find_package(Graphics CONFIG REQUIRED COMPONENTS test_commons)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        glfw
        ad::renderer
        ad::test_commons
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
