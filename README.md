# Graphics

Graphics library for programming with C++ and OpenGL.

This library aims to simplify the development of higher-level rendering engines and graphical applications.
It provides a non-invasive, generic, and modular foundation layer, consisting of the following components:
* `libs/renderer`: Abstractions, RAII wrappers, and utilities for the OpenGL API.
* `libs/arte`: File format handling for images and GLTF models.
* `libs/imgui`: RAII wrappers and utilities for integrating the Dear ImGui library.

The project initially served as a development sandbox,
with early experiments and prototypes available in `apps` and `libs/graphics`.

## Build (with Conan 2)

* Ensure [the pre-requisites](https://github.com/ShredEagle/public_build_info?tab=readme-ov-file#requirements) are met.
* Clone:
    ```bash
    git clone --recurse-submodule https://github.com/ShredEagle/graphics.git
    cd graphics
    ```
* Build:
    ```bash
    conan build ./conan/ --build missing \
      -c tools.system.package_manager:mode=install \
      -c tools.system.package_manager:sudo=True
    ```

## Build System and Dependencies

The project uses CMake for its build scripts, which is sufficient for building the project.
However, CMake does not manage upstream dependencies.

To address this, a Conan recipe is provided on top of the CMake scripts.
It handles dependency management and facilitates the integration inside a dependency graph.
