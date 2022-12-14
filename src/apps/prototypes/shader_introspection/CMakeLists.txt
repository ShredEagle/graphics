set(TARGET_NAME shader_introspection)

set(${TARGET_NAME}_HEADERS
    Introspection.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        graphics
        renderer
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
