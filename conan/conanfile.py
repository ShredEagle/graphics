from conans import ConanFile, CMake, tools


class TwodConan(ConanFile):
    name = "2d"
    version = "0.0.1"
    license = "The Unlicense"
    author = "adnn"
    url = "https://github.com/Adnn/2D"
    description = "2D graphics with OpenGL"
    topics = ("opengl", "2D")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    generators = "cmake_paths", "cmake"
    build_policy = "missing"

    requires = (
        ("boost/1.71.0@conan/stable"),
        ("glad/0.1.29@bincrafters/stable"),
        ("glfw/3.3@bincrafters/stable"),
        ("jsonformoderncpp/3.7.0@vthiery/stable"),
    )

    default_options = {
        "shared": False,
        "glad:api_version": "4.1",
        "glad:extensions": "GL_KHR_debug, GL_ARB_texture_storage",
    }
