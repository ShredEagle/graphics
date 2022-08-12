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
        ("math/b1491cb250@adnn/develop"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"
    keep_imports = True

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake


    def configure(self):
        tools.check_min_cppstd(self, "17")


    def layout(self):
        # Otherwise, root is the folder containing conanfile.py
        self.folders.root = ".."
        # Handles single-config (with subfolders) and multi-config (in a common folder)
        cmake_layout(self)


    def generate(self):
        toolchain = CMakeToolchain(self)
        # cache_variables are written to CMakePresets.json
        toolchain.cache_variables["BUILD_tests"] = str(self.options.build_tests)
        toolchain.generate()


    def imports(self):
        # see: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
        # the imgui package is designed this way: consumer has to import desired backends.
        self.copy("imgui_impl_glfw.cpp",         src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3.cpp",      src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_glfw.h",           src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3.h",        src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))
        self.copy("imgui_impl_opengl3_loader.h", src="./res/bindings", dst=path.join(self.folders.build, "conan_imports/imgui_backends"))


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()


    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "none")
        if self.folders.build_folder:
            self.cpp_info.builddirs.append(self.folders.build_folder)
        else:
            self.cpp_info.builddirs.append(path.join('lib', 'cmake'))
