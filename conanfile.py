from conans import ConanFile, CMake

class VideoSentinelConan(ConanFile):
    name = "VideoSentinel"
    version = "1.0.0"
    license = "MIT License"
    author = "Michael Pohl"
    description = "A colored region detection for videos using OpenCV and C++"
    topics = ("video", "sentinel", "opencv")
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    requires = [
        "catch2/3.1.0",
        "clara/1.1.5",
        "opencv/4.5.5"
    ]
    options = {"opencv:with_v4l": [True, False]}
    default_options = {"opencv:with_v4l": True}

    def layout(self):
        self.folders.build = "cmake_build"
        self.folders.source = "src"

    def configure(self):
        if self.options["opencv"].with_v4l:
            self.options["opencv"].with_v4l = True

    def requirements(self):
        if self.options["opencv"].with_v4l:
            # Override libpng for OpenCV
            self.requires["opencv"].ref.requires["libpng"].version = "1.6.42"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "VideoSentinel"
        self.cpp_info.names["cmake_find_package_multi"] = "VideoSentinel"
