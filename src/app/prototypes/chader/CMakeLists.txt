set(TARGET_NAME chader)

set(${TARGET_NAME}_HEADERS
    Chader.h
    FileWatcher.h
    GenericDrawer.h
    Scene.h
)

set(${TARGET_NAME}_SOURCES
    Chader.cpp
    FileWatcher.cpp
    main.cpp
)

find_package(glad REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        glad::glad
        ad::graphics
        ad::renderer
        gsl::gsl
        Boost::filesystem
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER prototypes)
