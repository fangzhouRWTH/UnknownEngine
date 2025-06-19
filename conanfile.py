import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout

class RenderEngine(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("vulkan-headers/1.4.309.0")
        self.requires("vulkan-loader/1.4.309.0")

        #self.requires("vk-bootstrap/0.7")
        self.requires("glfw/3.4")
        self.requires("spdlog/1.14.1")
        self.requires("fmt/10.2.1")
        self.requires("assimp/5.4.1")
        self.requires("eigen/3.4.0")
        self.requires("stb/cci.20230920")
        self.requires("nlohmann_json/3.11.2")
        self.requires("catch2/3.7.0")
        
        # Add base64 dependency only for Windows
        #if self.settings.os == "Windows":
        #    self.requires("base64/0.4.0")

    #def build_requirements(self):
    #    if self.settings.os != "Windows":  # we need cmake 3.22.6 in other platforms
    #        self.tool_requires("cmake/3.22.6")

    def layout(self):
        # We make the assumption that if the compiler is msvc the
        # CMake generator is multi-config
        cmake_layout(self)
        multi = True if self.settings.get_safe("compiler") == "msvc" else False
        if multi:
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.build_type))

