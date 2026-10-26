from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class PortfolioHealthConan(ConanFile):
    name = "portfolio_health"
    version = "2.0.71"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"
    default_options = {"libcurl/*:with_websockets": True}

    def layout(self):
        cmake_layout(self)

    def generate(self):
        CMakeDeps(self).generate()
        toolchain = CMakeToolchain(self)
        toolchain.user_presets_path = False
        toolchain.generate()

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        self.requires("openssl/3.2.1")
        self.requires("libcurl/8.22.0")