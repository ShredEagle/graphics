set(TARGET_NAME gltf_viewer)

set(${TARGET_NAME}_HEADERS
    Camera.h
    DataLayout.h
    DebugDrawer.h
    GltfAnimation.h
    GltfRendering.h
    LoadBuffer.h
    Logging.h
    Mesh.h
    Polar.h
    Scene.h
    Shaders.h
    SkeletalAnimation.h
    Url.h
)

set(${TARGET_NAME}_SOURCES
    DebugDrawer.cpp
    GltfAnimation.cpp
    GltfRendering.cpp
    LoadBuffer.cpp
    Logging.cpp
    main.cpp
    Mesh.cpp
    Scene.cpp
    SkeletalAnimation.cpp
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
