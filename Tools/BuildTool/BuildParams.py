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
        self.PathInfo : dict[str, list[str]] = {
            "LibPath" : "",
            "SrcPaths" : [],
            "SrcExcludePaths" : [],
            "HeaderPaths" : []
        }
        
    Name=""
    #These will contain the complete OS path
    ModulePath = ""

class ModuleObject:
    def __init__(self):
        self.TemplateFile = ""
        self.Type = ModuleType.STATIC
        self.Params = ModuleParams()
        self.SubModules : list[ModuleObject] = []
        self.CMakeDefines = []
        self.CompileOptions = []
        self.LinkerOptions = []
        self.Dependencies = []
        self.BuildFlags = []
        self.PreProcessorDefines = []


    def setModuleParams(self, paramDict : dict):
        if "Settings" in paramDict:
            self.Params.Name = paramDict["Settings"]["Name"]
            self.Params.ModulePath = BuildSettings.RootDirectory + paramDict["Settings"]["Path"]
            self.Type = ModuleType[paramDict["Settings"]["Type"]]
        
        if("LibPath" in paramDict):
            self.Params.PathInfo["LibPath"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["LibPath"]]
        if("SrcPaths" in paramDict):
            self.Params.PathInfo["SrcPaths"] = [os.path.join(self.Params.ModulePath, s) for s in  paramDict["SrcPaths"]]
        if("SrcExcludePaths" in paramDict):
            self.Params.PathInfo["SrcExcludePaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["SrcExcludePaths"]]
        if("HeaderPaths" in paramDict):
            self.Params.PathInfo["HeaderPaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["HeaderPaths"]]


        return

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
    
