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
        "glad:api_version": "4.1",
        "glad:extensions": "GL_KHR_debug, GL_ARB_texture_storage",
    }

    requires = (
        ("boost/1.72.0"),
        ("glad/0.1.29@bincrafters/stable"),
        ("glfw/3.3@bincrafters/stable"),
        ("jsonformoderncpp/3.7.0@vthiery/stable"),
        ("math/local"),
    )

    build_requires = ("cmake_installer/[>=3.16]@conan/stable",)

    build_policy = "missing"
    generators = "cmake_paths", "cmake"

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
