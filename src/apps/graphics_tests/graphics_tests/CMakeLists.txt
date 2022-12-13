string(TOLOWER ${PROJECT_NAME} _lower_project_name)
set(TARGET_NAME ${_lower_project_name}_tests)

set(${TARGET_NAME}_HEADERS
    catch.hpp
)

set(${TARGET_NAME}_SOURCES
    main.cpp

    Image_tests.cpp
    ShaderSource_tests.cpp
)

add_executable(${TARGET_NAME}
               ${${TARGET_NAME}_HEADERS}
               ${${TARGET_NAME}_SOURCES}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::graphics
        ad::renderer
        ad::test_commons
        )

set_target_properties(${TARGET_NAME} PROPERTIES
                      VERSION "${${PROJECT_NAME}_VERSION}"
)

install(TARGETS ${TARGET_NAME})
