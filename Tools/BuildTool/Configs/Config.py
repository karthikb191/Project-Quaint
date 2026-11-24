from pathlib import Path
import Configs.Args as Args

#TODO: Valid platforms and compilers can be fetched from a settings file
class PLATFORMS():
    WIN32 : str = "win32"
    WIN64 : str = "win64"
    LINUX : str = "linux"

class COMPILER():
    MSVC : str = "msvc"
    CLANG : str = "clang"

# TODO: Add support for dynamic config setup
class CONFIG():
    DEBUG : str = "debug"
    RELEASE : str = "release"

#TODO: Inject patch from command line arguments
PROJECT_DIRECTORY = "C:/Works/Project-Quaint/"
BUILT_TEMPLATES_DIRECTORY_REL = "Scripts/BuildTemplates/"
BUILD_TEMPLATES_DIRECTORY = PROJECT_DIRECTORY + BUILT_TEMPLATES_DIRECTORY_REL

BUILD_DIRECTORY = Path(PROJECT_DIRECTORY, "Build/V2")
BINARY_DIRECTORY = BUILD_DIRECTORY.joinpath("Bin")
INTERMEDIATES_DIRECTORY = BUILD_DIRECTORY.joinpath("Intermediates")
OUTPUT_DIRECTORY = BUILD_DIRECTORY.joinpath("Output")

TEMPLATE_FILE_EXTENSION = ".json"
SOURCE_EXTENSIONS = [".c", ".cpp", ".cxx", ".c++", ".cc", ".hpp"]

CMAKE_MINIMUM_VERSION = "3.10"

Args = Args.Args

def GetFullPath(relPath : str) -> str:
    return Path(PROJECT_DIRECTORY, relPath).as_posix()