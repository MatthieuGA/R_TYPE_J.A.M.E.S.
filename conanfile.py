"""
Conan recipe for R-Type J.A.M.E.S. project.

This file defines all dependencies for the project when using Conan as the
package manager. It mirrors the dependencies defined in vcpkg.json.
"""

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class RTypeRecipe(ConanFile):
    """
    Conan recipe for R-Type J.A.M.E.S. project dependencies.

    This recipe specifies all required dependencies and generates CMake files
    for integration with the build system.
    """

    name = "r-type"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    
    # Set C++20 standard
    def init(self):
        pass

    # Dependencies matching vcpkg.json
    def requirements(self):
        """Define project dependencies."""
        # SFML 2.6.x for graphics, audio, networking
        self.requires("sfml/2.6.1")
        # Boost components for networking and lock-free data structures
        self.requires("boost/1.83.0")
        # JSON library for configuration parsing
        self.requires("nlohmann_json/3.11.3")
        # fmt library (required by OpenAL-soft used by SFML Audio)
        self.requires("fmt/10.2.1")

    def build_requirements(self):
        """Define build-time dependencies."""
        # GoogleTest is fetched via FetchContent in CMake, not needed here

    def generate(self):
        """Generate CMake toolchain and dependency files."""
        # Generate CMake toolchain file for Conan
        tc = CMakeToolchain(self)
        tc.user_presets = False  # Don't generate CMakeUserPresets.json
        tc.variables["CMAKE_CXX_STANDARD"] = "20"
        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "TRUE"
        tc.generate()

        # Generate CMake find_package files for dependencies
        deps = CMakeDeps(self)
        deps.generate()

    def configure(self):
        """Configure options for dependencies."""
        # Enable all SFML components we need
        self.options["sfml/*"].audio = True
        self.options["sfml/*"].graphics = True
        self.options["sfml/*"].network = True
        self.options["sfml/*"].window = True

        # Configure Boost - we only need header-only asio + system + lockfree
        self.options["boost/*"].without_python = True
        self.options["boost/*"].without_test = True
        self.options["boost/*"].without_log = True
        self.options["boost/*"].without_locale = True
        self.options["boost/*"].without_mpi = True
        self.options["boost/*"].without_graph = True
        self.options["boost/*"].without_graph_parallel = True
        self.options["boost/*"].without_wave = True
