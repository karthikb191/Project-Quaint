from enum import Enum
from typing import Any
import os

class ModuleType(Enum):
    STATIC = 1
    DYNAMIC = 2
    BUILD_STATIC = 3
    BUILD_DYNAMIC = 4
    BUILD_INTERFACE = 5
    MODULE = 6
    EXECUTABLE = 7
    CMAKELISTS = 8

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
    CommonSettings : dict[str, Any] | None = {}


class ModuleParams:
    def __init__(self) -> None:
        self.Name = ""
        self.ModulePath = ""
        self.IntermediatePath = ""
        self.PathInfo : dict[str, list[str]] = {
            "LibPaths" : [],
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
        #self.ParentModule : ModuleObject = None
        self.Params = ModuleParams()
        #self.SubModules : list[ModuleObject] = []
        self.CMakeDefines : dict[str, str] = {}
        self.CompileOptions : list[str] = []
        self.LinkerOptions : list[str] = []
        self.Dependencies : list[ModuleObject] = []
        self.BuildFlags = []
        self.PreProcessorDefines :list[str] = []
        self.LibName : str = ""
        self.Resolved = False

    def setResolved(self):
        self.Resolved = True

    def setModuleParams(self, paramDict : dict[str, Any] | None):
        if(paramDict is None):
            return

        self.Params.Name = paramDict["Name"]
        self.Type = ModuleType[paramDict["Type"]]
        self.Params.ModulePath = BuildSettings.RootDirectory + paramDict["Path"]
        if(self.Type == ModuleType.BUILD_STATIC or self.Type == ModuleType.BUILD_DYNAMIC or self.Type == ModuleType.EXECUTABLE):
            self.TemplateFile = self.Params.Name + ".json"
        else:
            self.TemplateFile = "Dependency"

        self.LibName = paramDict.get("LibName") or ""
        if("LibPaths" in paramDict):
            #Populates actual paths to lib files
            for path in paramDict["LibPaths"]:
                self.Params.PathInfo["LibPaths"].append(os.path.join(self.Params.ModulePath, path, self.LibName))

        if("SrcPaths" in paramDict):
            self.Params.PathInfo["SrcPaths"] = [os.path.join(self.Params.ModulePath, s) for s in  paramDict["SrcPaths"]]
        if("SrcExcludePaths" in paramDict):
            self.Params.PathInfo["SrcExcludePaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["SrcExcludePaths"]]
        if("HeaderPaths" in paramDict):
            self.Params.PathInfo["HeaderPaths"] = [os.path.join(self.Params.ModulePath, s) for s in paramDict["HeaderPaths"]]
        
        if("Dependencies" in paramDict):
            for dependency in paramDict["Dependencies"]:
                # Dependency has already added. Skip
                if(dependency["Name"] in self.Dependencies):
                    continue
                module = ModuleObject()

                #If it's not module type, we immediately inject data
                if ModuleType[dependency["Type"]] is not ModuleType.MODULE:
                    module.setModuleParams(dependency)
                else:
                    module.Type = ModuleType[dependency["Type"]]
                    module.Params.Name = dependency["Name"]
                    module.Params.ModulePath = dependency["Path"]

                self.Dependencies.append(module)
        
        if("CMakeDefines" in paramDict):
            self.CMakeDefines = paramDict["CMakeDefines"]
        if("CompileOptions" in paramDict):
            self.CompileOptions = paramDict["CompileOptions"]
        if("PreProcessorDefines" in paramDict):
            self.PreProcessorDefines = paramDict["PreProcessorDefines"]

        return

    TemplateFile=""
    Type = ModuleType.STATIC
    Params = ModuleParams()
    #ParentModule = None 
    #SubModules = []
    CMakeDefines = {}
    CompileOptions = []
    LinkerOptions = []
    Dependencies = []
    BuildFlags = []
    PreProcessorDefines = []
    
