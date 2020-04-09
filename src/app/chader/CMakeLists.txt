project(chader)

set(${PROJECT_NAME}_HEADERS
    Chader.h
    FileWatcher.h
    GenericDrawer.h
    Scene.h
)

set(${PROJECT_NAME}_SOURCES
    Chader.cpp
    FileWatcher.cpp
    main.cpp
)

find_package(glad CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glad::glad
        ad::engine
        ad::renderer
        gsl::gsl
        Boost::filesystem
)
