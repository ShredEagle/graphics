add_subdirectory(lib/handy/handy)
add_subdirectory(lib/platform/platform)
add_subdirectory(lib/GSL-2.0.0/gsl)

add_subdirectory(lib/arte/arte)
add_subdirectory(lib/renderer/renderer)
add_subdirectory(lib/resource/resource)
add_subdirectory(lib/engine/engine)

option(BUILD_tests "Build the demo applications" ON)
if(BUILD_tests)
    add_subdirectory(app/tests/tests)

    # In order of implementation
    add_subdirectory(app/prototypes/demo)
    add_subdirectory(app/prototypes/simple)
    add_subdirectory(app/prototypes/sprite_animation)
    add_subdirectory(app/prototypes/doubledraw)
    add_subdirectory(app/prototypes/ringdrop)
    add_subdirectory(app/prototypes/tiler)
    add_subdirectory(app/prototypes/bloom)
    add_subdirectory(app/prototypes/chader)
    add_subdirectory(app/prototypes/polyline)
    add_subdirectory(app/prototypes/curve)

    add_subdirectory(app/samples/trivial_linestrip/trivial_linestrip)
    add_subdirectory(app/samples/trivial_shaping/trivial_shaping)
    add_subdirectory(app/samples/spriting/spriting)
endif()
