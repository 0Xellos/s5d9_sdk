from conans import ConanFile, CMake, tools
import re

def get_version():
    try:
        content = tools.load("CMakeLists.txt")
        version = re.search("VERSION (.*) LANGUAGES", content).group(1)
        return version.strip()
    except Exception as _:
        return None

class ConanPackage(ConanFile):
    """SDK for Renesas S5D9"""
    name = "s5d9_sdk"
    version = get_version()
    license = "Innovatrics"
    url = "ssh://git@gitlab.ba.innovatrics.net:7999/algo/s5d9_sdk.git"
    description = "SDK for Renesas S5D9"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_paths"
    short_paths = True
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }

    def cmake_configure(self):
        cmake = CMake(self)
        cmake.configure()
        return(cmake)

    def build(self):
        cmake = self.cmake_configure()
        cmake.build()

    def test(self):
        cmake = self.cmake_configure()
        cmake.test()

    def package(self):
        cmake = self.cmake_configure()
        cmake.install()

    def package_info(self):
        self.cpp_info.builddirs = [ "lib/cmake/s5d9_sdk" ]
