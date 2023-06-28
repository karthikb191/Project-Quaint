from enum import Enum

class ModuleType(Enum):
    STATIC = 1
    DYNAMIC = 2
    EXECUTABLE = 3

class BuildSystem(Enum):
    CMAKE = 1

class GlobalBuildSettings:
    BuildTarget = ""
    BuildSystem = BuildSystem.CMAKE

class ModuleParams:
    def __init__(self) -> None:
        self.Name = ""
        self.Location = ""
        self.OutputSubDirectory = ""
        pass
    Name=""
    Location=""
    OutputSubDirectory=""

class ModuleOject:
    def __init__(self):
        self.TemplateFile = ""
        self.Type = ModuleType.STATIC
        self.Params = ModuleParams()
        self.SubModules = []
        self.CMakeDefines = []
        self.CompileOptions = []
        self.LinkerOptions = []
        self.Dependencies = []
        self.BuildFlags = []
        self.PreProcessorDefines = []
        self.Params = {}


    def setModuleParams(self, params : dict):
        
        return

    Params = {}
    TemplateFile=""
    Type = ModuleType.STATIC
    Params = ModuleParams()
    SubModules=[]
    CMakeDefines=[]
    CompileOptions = []
    LinkerOptions = []
    Dependencies = []
    BuildFlags = []
    PreProcessorDefines = []
    
