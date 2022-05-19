set(TARGET_NAME ringdrop)

set(${TARGET_NAME}_HEADERS
    Ring.h
    scene.h
    shaders.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)

find_package(Handy CONFIG REQUIRED COMPONENTS resource)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        glfw
        ad::renderer
        ad::resource
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
