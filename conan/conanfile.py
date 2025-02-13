from conan import ConanFile
from conan.tools.build import can_run, check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy, update_conandata
from conan.tools.scm import Git

import os


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
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "glad/*:gl_version": "4.6", # 4.2 is required for glBindImageTexture()
                                  # 4.6 notably for FRAGMENT_SHADER_INVOCATIONS query
        # Note: macos only provides GL_ARB_texture_storage and GL_ARB_internalformat_query
        "glad/*:extensions": ("GL_KHR_debug,"
            "GL_ARB_texture_storage,"
            "GL_ARB_clear_texture,"
            "GL_ARB_program_interface_query,"
            "GL_ARB_shader_storage_buffer_object,"
            "GL_ARB_base_instance,"
            "GL_ARB_multi_draw_indirect,"
            "GL_ARB_texture_filter_anisotropic," # anisotropic texture filtering
        )
    }

    generators = "CMakeDeps", "CMakeToolchain"
    revision_mode = "scm"

    requires = (
        ("freetype/2.12.1"),
        ("spdlog/1.15.0"),
        ("utfcpp/4.0.1"),
    )

    def requirements(self):
        self.requires("handy/cb47135273@adnn/develop", transitive_headers=True),
        self.requires("math/cf1d07a75e@adnn/develop", transitive_headers=True),

        self.requires("glad/0.1.36", transitive_headers=True),
        self.requires("glfw/3.4", transitive_headers=True)
        self.requires("imgui/1.91.5-docking", transitive_headers=True)
        self.requires("nlohmann_json/3.11.2", transitive_headers=True)


    # There exist automatic alternatives.
    # see: https://docs.conan.io/2.0/reference/conanfile/methods/config_options.html?highlight=auto_shared_fpic
    def config_options(self):
        if self.settings.get_safe("os") == "Windows":
            self.options.rm_safe("fPIC")


    # see: https://github.com/conan-io/conan/issues/7530#issuecomment-1420634751
    def configure(self):
        if self.options.get_safe("shared"):
            self.options.rm_safe("fPIC")


    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, "20")


    # Handled at the profile level for the moment
    #def tool_requires(self):
    #    self.tool_requires("cmake/[>=3.31]")


    def layout(self):
        # The root of the project is one level above
        self.folders.root = ".."
        cmake_layout(self)


    def export(self):
        git = Git(self, self.recipe_folder)
        # Save the url and commit in conandata.yml
        # Unsafe atm since it is missing the repository argument,
        # so we save the coordinates manually
        #git.coordinates_to_conandata()
        url, commit = git.get_url_and_commit(repository=True)
        update_conandata(self, {"scm": {"url": url, "commit": commit}})


    def source(self):
        # we recover the saved url and commit from conandata.yml and use them to get sources
        git = Git(self)
        git.checkout_from_conandata_coordinates()
        git.run("submodule update --init")


    def generate(self):
        # the imgui package is designed this way: consumer has to import desired backends.
        # see: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
        # imports() has been removed from Conan 2 (and the blog post above is updated accordingly)
        # see: https://docs.conan.io/en/1.66/migrating_to_2.0/recipes.html#removed-imports-method
        imgui_package = os.path.join(self.dependencies["imgui"].package_folder, "res", "bindings")
        destination = os.path.join(self.build_folder, "conan_imports", "imgui_bindings")
        copy(self, "imgui_impl_glfw.h",           src=imgui_package, dst=destination)
        copy(self, "imgui_impl_glfw.cpp",         src=imgui_package, dst=destination)
        copy(self, "imgui_impl_opengl3.h",        src=imgui_package, dst=destination)
        copy(self, "imgui_impl_opengl3.cpp",      src=imgui_package, dst=destination)
        copy(self, "imgui_impl_opengl3_loader.h", src=imgui_package, dst=destination)


    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if can_run(self):
            cmake.test()


    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))


    def package_info(self):
        # Let's bite the bullet and repeate ourselve with a complete cpp_info,
        # waiting for sufficent Common Package Specification support in Conan an CMake.
        ## Disable the config package that would otherwise be generated by CMakeDeps
        #self.cpp_info.set_property("cmake_find_mode", "none")
        ## Find CMake-generated package config when consuming the (installed) conan package
        #self.cpp_info.builddirs = [os.path.join("lib", "cmake")]

        self.cpp_info.set_property("cmake_find_mode", "config")

        self.cpp_info.components["arte"].set_property("cmake_target_name", "ad::arte")
        self.cpp_info.components["arte"].includedirs = ["include/arte"]
        self.cpp_info.components["arte"].libs = ["arte"]
        self.cpp_info.components["arte"].requires = [
            "handy::handy",
            "handy::platform",
            "math::math",
            "freetype::freetype",
            "nlohmann_json::nlohmann_json",
            "spdlog::spdlog",
        ]
        #self.cpp_info.components["arte"].set_property("cmake_build_modules", ["lib/cmake/Graphics/arte/arteFindUpstream.cmake"])
        # Does not work on components, major limitation for us
        #self.cpp_info.set_property("cmake_build_modules", ["lib/cmake/Graphics/arte/arteFindUpstream.cmake"])

        self.cpp_info.components["graphics"].set_property("cmake_target_name", "ad::graphics")
        self.cpp_info.components["graphics"].includedirs = ["include/graphics"]
        self.cpp_info.components["graphics"].libs = ["graphics"]
        self.cpp_info.components["graphics"].requires = [
            "arte",
            "renderer",

            "handy::handy",
            "handy::resource",
            "math::math",
            "glad::glad",
            "glfw::glfw",
            "spdlog::spdlog",
            "utfcpp::utfcpp",
        ]

        self.cpp_info.components["imguiui"].set_property("cmake_target_name", "ad::imguiui")
        self.cpp_info.components["imguiui"].includedirs = ["include/imguiui"]
        self.cpp_info.components["imguiui"].libs = ["imguiui"]
        self.cpp_info.components["imguiui"].requires = [
            "graphics",

            "imgui::imgui",
        ]

        self.cpp_info.components["renderer"].set_property("cmake_target_name", "ad::renderer")
        self.cpp_info.components["renderer"].includedirs = ["include/renderer"]
        self.cpp_info.components["renderer"].libs = ["renderer"]
        self.cpp_info.components["renderer"].requires = [
            "arte",

            "handy::handy",
            "math::math",
            "glad::glad",
        ]
