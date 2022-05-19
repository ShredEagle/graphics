set(TARGET_NAME sprite-animation)

set(${TARGET_NAME}_HEADERS
    Scene.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glad REQUIRED)

find_package(Handy CONFIG REQUIRED COMPONENTS resource)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        ad::arte
        ad::graphics
        ad::resource
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER samples)
