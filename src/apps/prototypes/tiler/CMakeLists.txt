set(TARGET_NAME tiler)

set(${TARGET_NAME}_HEADERS
    scene.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 REQUIRED)

find_package(Graphics CONFIG REQUIRED COMPONENTS test_commons)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glfw
        ad::graphics
        ad::test_commons
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
