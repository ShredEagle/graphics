set(TARGET_NAME font)

set(${TARGET_NAME}_HEADERS
    Scene.h
    shaders.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glfw3 3.3 REQUIRED)
find_package(glad REQUIRED)

find_package(math REQUIRED)

find_package(Handy CONFIG REQUIRED COMPONENTS handy platform)
find_package(Graphics CONFIG REQUIRED COMPONENTS test_commons)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        glfw

        ad::arte
        ad::graphics
        ad::handy
        ad::math
        ad::platform
        ad::renderer
        ad::test_commons
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
