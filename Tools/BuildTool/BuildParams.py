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
        self.ParentModule : ModuleObject = None
        self.Params = ModuleParams()
        self.SubModules : list[ModuleObject] = []
        self.CMakeDefines = []
        self.CompileOptions = []
        self.LinkerOptions = []
        self.Dependencies : list[ModuleObject] = []
        self.BuildFlags = []
        self.PreProcessorDefines = []


    def setModuleParams(self, paramDict : dict):
        if "Settings" in paramDict:
            self.Params.Name = paramDict["Settings"]["Name"]
            self.Params.ModulePath = BuildSettings.RootDirectory + paramDict["Settings"]["Path"]
            self.Type = ModuleType[paramDict["Settings"]["Type"]]
            self.TemplateFile = self.Params.Name + ".buildTmpl"
        
        if("LibPath" in paramDict):
            self.Params.PathInfo["LibPath"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["LibPath"]]
        if("SrcPaths" in paramDict):
            self.Params.PathInfo["SrcPaths"] = [os.path.join(self.Params.ModulePath, s) for s in  paramDict["SrcPaths"]]
        if("SrcExcludePaths" in paramDict):
            self.Params.PathInfo["SrcExcludePaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["SrcExcludePaths"]]
        if("HeaderPaths" in paramDict):
            self.Params.PathInfo["HeaderPaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["HeaderPaths"]]
        if("Dependencies" in paramDict):
            for dependency in paramDict["Dependencies"]:
                Module = ModuleObject()
                Module.Type = ModuleType[dependency["Type"]]
                Module.Params.Name = dependency["Name"]
                Module.Params.ModulePath = dependency["Path"]
                self.Dependencies.append(Module)

        return

    TemplateFile=""
    Type = ModuleType.STATIC
    Params = ModuleParams()
    ParentModule = None 
    SubModules = []
    CMakeDefines=[]
    CompileOptions = []
    LinkerOptions = []
    Dependencies = []
    BuildFlags = []
    PreProcessorDefines = []
    
