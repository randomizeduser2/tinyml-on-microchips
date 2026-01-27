from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class mainRecipe(ConanFile):
    name = "main"
    version = "0.1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Kaan Mert Ağyol <kaanmertagyol@gmail.com>"
    url = "https://github.com/randomizeduser2/tinyml-on-microchips"
    description = "TinyML kapsamında Edge impulse üzerinde eğitilen modelin OpenCV ve TensorFlow Lite kullanılarak C++ ile gerçek zamanlı video akışında çalıştırılması"
    topics = ("TinyML", "Edge Impulse", "OpenCV", "TensorFlow Lite", "C++")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    # Bağımlılıklar
    def requirements(self):
        self.requires("fmt/12.1.0")
        self.requires("opencv/4.12.0")
        self.requires("tensorflow-lite/2.15.0")
    
    def configure(self):
        # OpenCV'nin kamera ve GUI için izinlerinin etklinleştirilmesi
        self.options["opencv"].with_v4l = True
        self.options["opencv"].with_gtk = True
        self.options["opencv"].videoio = True

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    

    
