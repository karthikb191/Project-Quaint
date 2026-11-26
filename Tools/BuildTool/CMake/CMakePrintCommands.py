from io import TextIOWrapper
from Models.Models import TargetSettingsSchema

def PrintNewLine(fs : TextIOWrapper):
    fs.write("\n")

def PrintCMakeMinimum(fs : TextIOWrapper, minVersion: str):
    fs.write(f"cmake_minimum_required(VERSION {minVersion})")
    PrintNewLine(fs)

def PrintIncludeGuard(fs : TextIOWrapper, scope : str = "DIRECTORY"):
    fs.write(f"include_guard {scope}")
    PrintNewLine(fs)

def PrintArchiveOutputDirectory(fs: TextIOWrapper, dirPath: str):
    fs.write(f"set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY {dirPath})")
    PrintNewLine(fs)

def PrintLibraryOutputDirectory(fs: TextIOWrapper, dirPath: str):
    fs.write(f"set(CMAKE_LIBRARY_OUTPUT_DIRECTORY {dirPath})")
    PrintNewLine(fs)

def PrintRuntimeOutputDirectory(fs: TextIOWrapper, dirPath: str):
    fs.write(f"set(CMAKE_RUNTIME_OUTPUT_DIRECTORY {dirPath})")
    PrintNewLine(fs)
    
def PrintInclude(fs : TextIOWrapper, includeFile: str ):
    fs.write(f"include({includeFile})")
    PrintNewLine(fs)

def PrintSubDirectory(fs : TextIOWrapper, subDirectory: str, relBinDir : str = ""):
    subDirStr = f"${{CMAKE_CURRENT_BINARY_DIR}}/{relBinDir}" if len(relBinDir) > 0 else ""
    fs.write(f"add_subdirectory({subDirectory} {subDirStr})")
    PrintNewLine(fs)

def PrintCMakeProject(fs : TextIOWrapper, name : str, version : str = "1.0.0"):
    fs.write(
        f"project({name} \
        VERSION {version})")
    PrintNewLine(fs)

def PrintSetTargetName(fs: TextIOWrapper, targetName: str):
    fs.write(f"set(TARGET_NAME {targetName})")
    PrintNewLine(fs)
    fs.write(f"set(SOURCES )")
    PrintNewLine(fs)
    PrintNewLine(fs)

def PrintSources(fs: TextIOWrapper, sourceList : list[str]):
    if(len(sourceList) <= 0) : return

    sourceStr = '\n'.join(sourceList)
    fs.write(f"list(APPEND SOURCES {sourceStr})")
    PrintNewLine(fs)

# Defining Target
def PrintExecutableTarget(fs: TextIOWrapper):
    fs.write(f"add_executable(${{TARGET_NAME}})")
    PrintNewLine(fs)

def PrintStaticLibraryTarget(fs: TextIOWrapper):
    fs.write(f"add_library(${{TARGET_NAME}} STATIC)")
    PrintNewLine(fs)

def PrintDynamicLibraryTarget(fs: TextIOWrapper):
    fs.write(f"add_library(${{TARGET_NAME}} DYNAMIC)")
    PrintNewLine(fs)

def PrintTargetSources(fs: TextIOWrapper, scope: str = "PUBLIC"):
    fs.write(f"target_sources(${{TARGET_NAME}} {scope} ${{SOURCES}})")
    PrintNewLine(fs)

def PrintInterfaceTarget(fs: TextIOWrapper):
    fs.write(f"add_library(${{TARGET_NAME}} INTERFACE)")
    PrintNewLine(fs)

def PrintIncludeDirectories(fs: TextIOWrapper, includePaths: list[str], scope: str = "PUBLIC"):
    dirStr = "\n".join(includePaths)
    fs.write(f"include_directories(${{TARGET_NAME}} {scope} {dirStr})")
    PrintNewLine(fs)
    
def PrintTargetIncludeDirectories(fs: TextIOWrapper, includePaths: list[str], scope: str = "PUBLIC"):
    dirStr = "\n".join(includePaths)
    fs.write(f"target_include_directories(${{TARGET_NAME}} {scope} {dirStr})")
    PrintNewLine(fs)

# Compile Options
def PrintCompileOptions(fs: TextIOWrapper, options: list[str], scope: str = "PUBLIC"):
    if(len(options) <= 0) : return
    optStr = "; ".join(options)
    fs.write(f"target_compile_options(${{TARGET_NAME}} {scope} {optStr})")
    PrintNewLine(fs)

def PrintCompileDefinitions(fs: TextIOWrapper, definitions: list[str], scope: str = "PUBLIC"):
    if(len(definitions) <= 0) : return

    defStr = "; ".join(definitions)
    fs.write(f"target_compile_definitions(${{TARGET_NAME}} {scope} {defStr})")
    PrintNewLine(fs)

def AddFileGroups(fs: TextIOWrapper):
    #TODO: Add imlementation
    pass

# TODO: add support for interface and imported targets. No use case for it ATM


# Link Options
def PrintLinkTarget(fs: TextIOWrapper, otherTarget: str, scope: str = "PUBLIC"):
    fs.write(f"target_link_libraries(${{TARGET_NAME}} {scope} {otherTarget})")
    PrintNewLine(fs)

def PrintLinkOptions(fs: TextIOWrapper, linkOptions: list[str], scope: str = "PUBLIC"):
    if(len(linkOptions) <= 0): return;

    optStr = ", ".join(linkOptions)
    fs.write(f"target_link_options(${{TARGET_NAME}} {scope} {optStr})")
    PrintNewLine(fs)


def PrintTargetPreCompileHeaders(fs: TextIOWrapper, pchFiles : list[str], scope: str = ""):
    if(len(pchFiles) <= 0) : return
    pchFilesStr = "\n".join(pchFiles)
    fs.write(f"target_precompile_headers(${{TARGET_NAME}} {scope} {pchFilesStr})")
    PrintNewLine(fs)