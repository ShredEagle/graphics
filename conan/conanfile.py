from conans import ConanFile, CMake, tools


class TwodConan(ConanFile):
    name = "2d"
    license = "The Unlicense"
    author = "adnn"
    url = "https://github.com/Adnn/2D"
    description = "2D graphics with OpenGL"
    topics = ("opengl", "graphics", "2D")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
        "glad:api_version": "4.1",
        "glad:extensions": "GL_KHR_debug, GL_ARB_texture_storage",
    }

    requires = (
        ("boost/1.71.0@conan/stable"),
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
        cmake.definitions["CMAKE_PROJECT_2d_INCLUDE"] = \
            path.join(self.source_folder, "cmake", "conan", "customconan.cmake")
        cmake.definitions["BUILD_tests"] = self.options.build_tests
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
