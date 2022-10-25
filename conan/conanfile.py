from conans import ConanFile, tools
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy

from os import path


class GraphicsConan(ConanFile):
    name = "graphics"
    license = "MIT"
    author = "adnn"
    url = "https://github.com/Adnn/graphics"
    description = "Graphics rendering generic library, both software and with OpenGL"
    topics = ("opengl", "graphics", "2D", "3D")
    settings = "os", "compiler", "build_type", "arch"
    short_paths = True
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
        "glad:gl_version": "4.1",
        # Note: macos only provides GL_ARB_texture_storage and GL_ARB_internalformat_query
        "glad:extensions": "GL_KHR_debug, GL_ARB_texture_storage, GL_ARB_clear_texture",
    }

    requires = (
        ("freetype/2.12.1"),
        ("glad/0.1.34"),
        ("glfw/3.3.6"),
        ("nlohmann_json/3.9.1"),
        ("spdlog/1.9.2"),
        ("utfcpp/3.2.1"),
        ("imgui/1.87"),

        ("handy/b424f82ee7@adnn/develop"),
        ("math/e4a1b11ae1@adnn/develop"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"
    keep_imports = True


    python_requires="shred_conan_base/0.0.5@adnn/stable"
    python_requires_extend="shred_conan_base.ShredBaseConanFile"


    def imports(self):
        # see: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
        # the imgui package is designed this way: consumer has to import desired backends.
        self.copy("imgui_impl_glfw.cpp",         src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3.cpp",      src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_glfw.h",           src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3.h",        src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3_loader.h", src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
