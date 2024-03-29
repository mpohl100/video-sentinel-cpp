from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class VideoSentinelConan(ConanFile):
    name = "VideoSentinel"
    version = "1.0.0"
    license = "Apache License v2.0"
    author = "Michael Pohl"
    description = "A colored region detection for videos using OpenCV and C++"
    topics = ("video", "sentinel", "opencv")
    settings = "os", "compiler", "build_type", "arch"
    requires = [
        "catch2/3.1.0",
        "clara/1.1.5",
        "opencv/4.5.5"
    ]
    options = {"coverage": [True, False]}
    default_options = {"coverage": False}

    def configure(self):
        print("try doing nothing")

    def requirements(self):
        # Override libpng for OpenCV
        self.requires("libpng/1.6.42", override=True)

    def layout(self):
        cmake_layout(self, src_folder=".")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        coverage = getattr(self.options, "coverage", False)

        if coverage:
            self.output.info("Building with coverage flags...")
            self._build_with_coverage()
        else:
            self.output.info("Building without coverage flags...")
            self._build_without_coverage()

    def _build_with_coverage(self):
        cmake = CMake(self)
        vars = {
            "ENABLE_COVERAGE": "true",
        }
        cmake.configure(vars)
        cmake.build()

    def _build_without_coverage(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
