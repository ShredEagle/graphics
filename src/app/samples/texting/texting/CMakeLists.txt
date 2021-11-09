set(TARGET_NAME texting)

set(${TARGET_NAME}_HEADERS
    Scene.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

find_package(glad REQUIRED)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        ad::graphics
        ad::resource
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER samples)
