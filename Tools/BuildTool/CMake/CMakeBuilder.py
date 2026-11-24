from colorama import Fore
import re
from io import TextIOWrapper
from pathlib import Path
import Configs.Config as Config
import CMake.CMakePrintCommands as cmds
from Models.ProjectTypes import BuildProject, BuildTarget, ETargetBuildType

class CMakeBuilder:
    def __init__(self) -> None:
        self._errors = 0;
        pass

    def RunBuild(self, project : BuildProject):
        print(Fore.BLUE, "-> Starting CMAKE file generation")

        #TODO: Any validation on the target dependencies should be done here.


        print(Fore.WHITE, "... Gathering Sources ...")
        for _, target in project.AvailableTargets.items():
            target.Sources = GatherSources(target)

        self.__BuildCMakeProject(project)
        for _, target in project.AvailableTargets.items():
            self.__BuildCMakeTarget(target)
        
        print(Fore.GREEN, "-> CMAKE file generation completed")
        pass

    def __BuildCMakeProject(self, project: BuildProject):
        path = Path(Config.INTERMEDIATES_DIRECTORY)
        filePath = Path(path, "CMakeLists.txt")
        if not path.exists():
            path.mkdir(parents=True)

        fs = open(filePath, 'w', -1, "utf-8")
        cmds.PrintCMakeMinimum(fs, Config.CMAKE_MINIMUM_VERSION)
        cmds.PrintCMakeProject(fs, project.Name)

        #TODO: Add common compiler flags, etc
        targetDirectory = Path(path, project.Target.Name)
        cmds.PrintSubDirectory(fs, targetDirectory.as_posix(), project.Target.Name)

        fs.close()
        pass

    def __BuildCMakeTarget(self, target: BuildTarget):
        #TODO: Check if target is already built.
        #each target forms it's own sub directory with CMakeLists.txt file

        #Currently, all "Targets" get their own sub directory when building
        intermediatePath = Path(Config.INTERMEDIATES_DIRECTORY, target.Name)
        filePath = intermediatePath.joinpath("CMakeLists.txt")
        if not intermediatePath.exists():
            intermediatePath.mkdir(parents=True)

        fs = open(filePath, 'w', -1, "utf-8")
        
        cmds.PrintCMakeMinimum(fs, Config.CMAKE_MINIMUM_VERSION)
        cmds.PrintNewLine(fs)

        cmds.PrintSetTargetName(fs, target.Name)
        cmds.PrintSources(fs, target.Sources)
        cmds.PrintNewLine(fs)

        match target.BuildType:
            case ETargetBuildType.EXECUTABLE:
                cmds.PrintExecutableTarget(fs)
            case ETargetBuildType.STATIC:
                cmds.PrintStaticLibraryTarget(fs)
            case ETargetBuildType.DYNAMIC:
                cmds.PrintDynamicLibraryTarget(fs)
            case _:
                raise Exception("Invalid operation")
        cmds.PrintNewLine(fs)

        #Include any sub-targets here. Need to define the target before adding include statements
        for _, subTarget in target.SubTargets.items():
            subTargetPath = Path(intermediatePath, "SubTargets", f"{subTarget.Name}.cmake")
            cmds.PrintInclude(fs, subTargetPath.as_posix())
            self.__BuildCMakeSubTarget(target, subTarget)

        #Header setup
        headerPaths = self.__GetHeaderFullPaths(target.Schema.Path, target.Schema.HeaderPaths)
        cmds.PrintTargetIncludeDirectories(fs, headerPaths)

        #Target Linking and dependency setup
        #Link Targets being built
        for _, link in target.Targets.items():
            targetDirPath = Path(Config.INTERMEDIATES_DIRECTORY, link.Name)
            cmds.PrintSubDirectory(fs, targetDirPath.as_posix(), link.Name)
            cmds.PrintLinkTarget(fs, link.Name)
            cmds.PrintNewLine(fs)
            #TODO: Add support for link option printing for specific targets
        
        self.__BuildExternalTargets(fs, target)
        self.__BuildExternalCMakeTargets(fs, target)
        self.__BuildStaticLibraries(fs, target)
        #TODO: Add support for linking static libraries

        #TODO: Add support for linking dynamic libraries

        fs.close()

    def __BuildExternalTargets(self, fs: TextIOWrapper, target: BuildTarget):
        extTargets = target.ExternalTargets
        if extTargets is None: return

        #Fire off function to build target file
        for _, extTarget in extTargets.items():
            intermediatePath = Path(Config.INTERMEDIATES_DIRECTORY, extTarget.Name)
            cmds.PrintSubDirectory(fs, intermediatePath.as_posix(), extTarget.Name)
            cmds.PrintLinkTarget(fs, extTarget.Name)
            self.__BuildCMakeTarget(extTarget)
        
        cmds.PrintNewLine(fs)


    def __BuildStaticLibraries(self, fs: TextIOWrapper, target: BuildTarget):
        staticLibs = target.Schema.DependencyList.StaticLibs
        if staticLibs is None: return

        for lib in staticLibs:
            headerPaths = self.__GetHeaderFullPaths(lib.Path, lib.HeaderPaths)
            libFilePath = Path(Config.PROJECT_DIRECTORY, lib.Path, lib.FilePath)
            cmds.PrintTargetIncludeDirectories(fs, headerPaths)
            cmds.PrintLinkTarget(fs, libFilePath.as_posix())
        
        cmds.PrintNewLine(fs)

    def __BuildExternalCMakeTargets(self, fs: TextIOWrapper, target: BuildTarget):
        extCMakeTargets = target.Schema.DependencyList.ExternalCMakeTargets
        if extCMakeTargets is None: return

        for ext in extCMakeTargets:
            cmakeListsPath = Path(Config.PROJECT_DIRECTORY, ext.Path)
            if(Path(cmakeListsPath).is_file() is False):
                print(Fore.RED, "No CMakeLists.txt file found for external CMake target: " + ext.Name)

            if(cmakeListsPath.is_file()):
                cmds.PrintSubDirectory(fs, cmakeListsPath.parent.as_posix(), ext.Name)
            else:
                cmds.PrintSubDirectory(fs, cmakeListsPath.as_posix(), ext.Name)
            cmds.PrintLinkTarget(fs, ext.Name)
        cmds.PrintNewLine(fs)


    def __BuildCMakeSubTarget(self, parentTarget: BuildTarget, subTarget: BuildTarget):
        intermediatePath = Path(Config.INTERMEDIATES_DIRECTORY, parentTarget.Name, "SubTargets")
        filePath = intermediatePath.joinpath(f"{subTarget.Name}.cmake")
        if not intermediatePath.exists():
            intermediatePath.mkdir(parents=True)

        fs = open(filePath, 'w', -1, "utf-8")
        
        cmds.PrintSources(fs, subTarget.Sources, True)

        headerPaths = self.__GetHeaderFullPaths(subTarget.Schema.Path, subTarget.Schema.HeaderPaths)

        #TODO: differentiate between private and public header directories
        cmds.PrintTargetIncludeDirectories(fs, headerPaths)

        # TODO: Add support for reading sub-targets within sub-targets. Skipping for now as it's a pain to check for recursion.
        # The data should be pre-validated to ensure there are no cycles
        if(len(subTarget.SubTargets) > 0):
            print(Fore.RED, "Sub-Targets within sub target is not yet supported!")

        externalCMakeTargets = subTarget.Schema.DependencyList.ExternalCMakeTargets
        if(externalCMakeTargets is not None and len(externalCMakeTargets) > 0):
            print(Fore.RED, "External CMake targets within sub target is not yet supported!")

        fs.close()

    def __GetHeaderFullPaths(self, targetRelPath: str, relHeaderPaths: list[str]) -> list[str]:
        headerPaths = [Path(Config.PROJECT_DIRECTORY, targetRelPath, header).as_posix() 
                            for header in relHeaderPaths]
        return headerPaths

#TODO: These functions may be best to be placed in a utils file
#TODO: Pass parameter that specifies directories already checked against
def GatherSources(target : BuildTarget) -> list[str]:
    sources : list[str] = []
    #TODO: Currently ignores any overrides.

    print(Fore.WHITE, f"(Sourcing): {target.Name}")
    targetDirectory = Path(Config.PROJECT_DIRECTORY).joinpath(target.Schema.Path)

    srcPaths = target.Schema.SourcePaths
    if srcPaths is None: 
        return sources
    
    excludePaths : list[Path] = []
    for path in target.Schema.ExcludePaths :
        excludePaths.append(targetDirectory.joinpath(path))

    # Fetch sources from the root source paths
    for path in srcPaths:
        fullPath = targetDirectory.joinpath(path)
        sources.extend(GetSourcesFromPath(fullPath, excludePaths))

    filesToRemove = []
    mainFilePattern = ".*[Mm]ain.(cpp|cxx|cc)"
    if target.BuildType != ETargetBuildType.EXECUTABLE:
        filesToRemove = [path for path in sources if re.search(mainFilePattern, path)]
        for file in filesToRemove:
            sources.remove(file)

    # SubTarget sources should also be fetched. And, it's the responsibility of the target 
    # TODO: Validate against paths that are already fetched from above!!
    if len(target.SubTargets) > 0:
        print(Fore.WHITE, "..... Gathering SubTarget sources .....")
        for _, subTarget in target.SubTargets.items():
            subTarget.Sources = GatherSources(subTarget)
        print(Fore.WHITE, "..... Gathering SubTarget sources Completed .....")

    return sources

def GetSourcesFromPath(fullPath : Path, excludePaths: list[Path]) -> list[str]:
    sources : list[str] = []

    if(fullPath.is_file()): return sources

    # If the current directory is in the hierarchy of a exclude path, it should be skipped
    for excPath in excludePaths:
        if(fullPath.is_relative_to(excPath)): return sources

    regexExtList = [f"*{ext}" for ext in Config.SOURCE_EXTENSIONS]

    # Gather all the source files in the current directory
    for ext in regexExtList:
        paths = fullPath.glob(ext)
        sources.extend([path.as_posix() for path in paths])

    # Walk into the sub-directories and collect sources from them too
    for path in fullPath.iterdir():
        if(path.is_file()): continue
        sources.extend(GetSourcesFromPath(path, excludePaths))

    return sources