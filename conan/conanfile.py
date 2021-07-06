from conans import ConanFile, CMake, tools

from os import path


class GraphicsConan(ConanFile):
    name = "graphics"
    license = "The Unlicense"
    author = "adnn"
    url = "https://github.com/Adnn/graphics"
    description = "Graphics, software and with OpenGL"
    topics = ("opengl", "graphics", "2D", "3D")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
        "boost:layout": "versioned", #Should be system on non-Windows
        "glad:gl_version": "4.1",
        "glad:extensions": "GL_KHR_debug, GL_ARB_texture_storage",
    }

    requires = (
        ("boost/1.76.0"),
        ("glad/0.1.34"),
        ("glfw/3.3.4"),
        ("nlohmann_json/3.9.1"),
        ("math/local"),
    )

    build_requires = ("cmake/3.20.4",)

    build_policy = "missing"
    generators = "cmake_paths", "cmake", "cmake_find_package"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_PROJECT_Graphics_INCLUDE"] = \
            path.join(self.source_folder, "cmake", "conan", "customconan.cmake")
        cmake.definitions["BUILD_tests"] = self.options.build_tests
        cmake.definitions["Boost_USE_STATIC_LIBS"] = not self.options["boost"].shared
        cmake.configure()
        return cmake


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()


    def package_info(self):
        pass
