from io import TextIOWrapper
from ..Models.Models import TargetSettingsSchema

def PrintNewLine(fs : TextIOWrapper):
    fs.write("\n")

def PrintCMakeMinimum(fs : TextIOWrapper, minVersion: str):
    fs.write(f"cmake_minimum_required(VERSION {str})")
    PrintNewLine(fs)

def PrintIncludeGuard(fs : TextIOWrapper, scope : str = "DIRECTORY"):
    fs.write(f"include_guard {scope}")
    PrintNewLine(fs)

def PrintInclude(fs : TextIOWrapper, includeFile: str ):
    fs.write(f"include({includeFile})")
    PrintNewLine(fs)

def PrintSubDirectory(fs : TextIOWrapper, subDirectory: str):
    fs.write(f"add_subdirectory({subDirectory})")
    PrintNewLine(fs)

def PrintCMakeProject(fs : TextIOWrapper, name : str, version : str = "1.0.0"):
    fs.write(
        f"project({name} \
        VERSION {version})")
    PrintNewLine(fs)

def PrintSources(fs: TextIOWrapper, target: str, sourceList : list[str]):
    sourceStr = '\n'.join(sourceList)
    fs.write(f"set(SOURCES \
        {sourceStr})")
    PrintNewLine(fs)

# Defining Target
def PrintExecutableTarget(fs: TextIOWrapper, target: str):
    fs.write(f"add_executable({target} ${{SOURCES}})")
    PrintNewLine(fs)

def PrintStaticLibraryTarget(fs: TextIOWrapper, target: str):
    fs.write(f"add_library({target} STATIC ${{SOURCES}})")
    PrintNewLine(fs)

def PrintDynamicLibraryTarget(fs: TextIOWrapper, target: str):
    fs.write(f"add_library({target} DYNAMIC ${{SOURCES}})")
    PrintNewLine(fs)

def PrintInterfaceTarget(fs: TextIOWrapper, target: str):
    fs.write(f"add_library({target} INTERFACE)")
    PrintNewLine(fs)

def PrintIncludeDirectories(fs: TextIOWrapper, target: str, includePaths: list[str], scope: str = "PUBLIC"):
    dirStr = "\n".join(includePaths)
    fs.write(f"target_include_directories({target} {scope} {dirStr})")
    PrintNewLine(fs)

# Compile Options
def PrintCompileOptions(fs: TextIOWrapper, target: str, options: list[str], scope: str = "PUBLIC"):
    optStr = ", ".join(options)
    fs.write(f"target_compile_options({target} {scope} {optStr})")
    PrintNewLine(fs)

def PrintCompileDefinitions(fs: TextIOWrapper, target: str, definitions: list[str], scope: str = "PUBLIC"):
    defStr = ", ".join(definitions)
    fs.write(f"target_compile_definitions({target} {scope} {defStr})")
    PrintNewLine(fs)

def AddFileGroups(fs: TextIOWrapper):
    #TODO: Add imlementation
    pass

# TODO: add support for interface and imported targets. No use case for it ATM


# Link Options
def PrintLinkTarget(fs: TextIOWrapper, target: str, otherTarget: str):
    fs.write(f"target_link_libraries({target} {otherTarget})")
    PrintNewLine(fs)

def PrintLinkOptions(fs: TextIOWrapper, target: str, linkOptions: list[str], scope: str = "PUBLIC"):
    optStr = ", ".join(linkOptions)
    fs.write(f"target_link_options({target} {scope} {optStr})")
    PrintNewLine(fs)


def PrintTargetPreCompileHeaders(fs: TextIOWrapper, target: str, pchFiles : list[str], scope: str = ""):
    pchFilesStr = "\n".join(pchFiles)
    fs.write(f"target_precompile_headers({target} {scope} {pchFilesStr})")
    PrintNewLine(fs)