# 2D
2D graphics with OpenGL

## Development

Build environment setup:

    git clone --recurse-submodules ...
    cd 2D
    mkdir build && cd build
    # glfw 3.3 must be built because of a bug in INTERFACE_INCLUDE_DIR (absolute path instead of config relative)
    # glfw 3.4 should already fix that
    conan install ../conan --build=missing --build=glfw
    cmake ../
    # Actual build command
    cmake --build ./
