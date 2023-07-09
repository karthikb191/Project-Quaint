from enum import Enum
import os

class ModuleType(Enum):
    STATIC = 1
    DYNAMIC = 2
    BUILD_STATIC = 3
    BUILD_DYNAMIC = 4
    MODULE = 5
    EXECUTABLE = 6

class BuildSystem(Enum):
    CMAKE = 1

class BuildSettings:
    BuildTarget = ""
    BuildSystem = BuildSystem.CMAKE
    RootDirectory = ""
    OutputDirectory = ""
    IntermediateDirectory = ""
    BinaryDirectory = ""
    StaticLibExtension = ".lib"
    DynamicLibExtension = ".dll"
    SourceExtensions = [".c", ".cpp"]


class ModuleParams:
    def __init__(self) -> None:
        self.Name = ""
        self.ModulePath = ""
        self.IntermediatePath = ""
        self.PathInfo = {
            "LibPath" : str,
            "SrcPaths" : [str],
            "SrcExcludePaths" : [str],
            "HeaderPaths" : [str]
        }
        
    Name=""
    #These will contain the complete OS path
    ModulePath = ""

class ModuleObject:
    def __init__(self):
        self.TemplateFile = ""
        self.Type = ModuleType.STATIC
        self.Params = ModuleParams()
        self.SubModules = [ModuleObject]
        self.CMakeDefines = []
        self.CompileOptions = []
        self.LinkerOptions = []
        self.Dependencies = []
        self.BuildFlags = []
        self.PreProcessorDefines = []
        self.Params = {}


    def setModuleParams(self, paramDict : dict):
        if "Settings" in paramDict:
            self.Params.Name = paramDict["Settings"]["Name"]
            self.Params.ModulePath = BuildSettings.RootDirectory + paramDict["Settings"]["Path"]
            self.Type = paramDict["Settings"]["Type"]
        
        self.Params.PathInfo["LibPath"] = [os.path.join(self.Params.ModulePath + str) for str in paramDict["LibPath"]]
        self.Params.PathInfo["SrcPaths"] = [os.path.join(self.Params.ModulePath + str) for str in  paramDict["SrcPaths"]]
        self.Params.PathInfo["SrcExcludePaths"] = [os.path.join(self.Params.ModulePath + str) for str in paramDict["SrcExcludePaths"]]
        self.Params.PathInfo["HeaderPaths"] = [os.path.join(self.Params.ModulePath + str) for str in paramDict["HeaderPaths"]]

        return

    Params = {}
    TemplateFile=""
    Type = ModuleType.STATIC
    Params = ModuleParams()
    SubModules = []
    CMakeDefines=[]
    CompileOptions = []
    LinkerOptions = []
    Dependencies = []
    BuildFlags = []
    PreProcessorDefines = []
    
