set(TARGET_NAME tiler)

set(${TARGET_NAME}_HEADERS
    scene.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(Boost 1.71)
find_package(glfw3 3.3 REQUIRED)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glfw::glfw
        ad::graphics
        ad::resource
        Boost::boost # Target for header-only dependencies
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
