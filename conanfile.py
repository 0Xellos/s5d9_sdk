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
    """SDK for Renesas S7G2"""
    name = "s7g2_sdk"
    version = get_version()
    license = "Innovatrics"
    url = "ssh://git@git.ba.innovatrics.net:7999/algo/s7g2_sdk.git"
    description = "SDK for Renesas S7G2"
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
        self.cpp_info.builddirs = [ "lib/cmake/s7g2_sdk" ]
