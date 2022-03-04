set(TARGET_NAME gltf_viewer)

set(${TARGET_NAME}_HEADERS
    Logging.h
)

set(${TARGET_NAME}_SOURCES
    main.cpp
    Logging.cpp
)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::arte
        ad::graphics
        ad::resource

        Boost::program_options
)
