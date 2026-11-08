import os


PLATFORMS = {
    "win32" : "win32",
    "win64" : "win64",
    "ios" : "ios"
}

COMPILER = {
    "msvc" : "msvc",
    "clang" : "clang"
}

BUILD_CONFIG = {
    "debug" : "debug",
    "release" : "release"
}

PROJECT_DIRECTORY = "C:\\Works\\Project-Quaint\\"
BUILT_TEMPLATES_DIRECTORY_REL = "Scripts\\BuildTemplates\\"
BUILD_TEMPLATES_DIRECTORY = PROJECT_DIRECTORY + BUILT_TEMPLATES_DIRECTORY_REL
TEMPLATE_FILE_EXTENSION = ".json"

def GetFullPath(relPath : str) -> str:
    return os.path.join(PROJECT_DIRECTORY, relPath)