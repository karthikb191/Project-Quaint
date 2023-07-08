from enum import Enum

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
    def __init__(self) -> None:
        self.BuildTarget = ""
        self.BuildSystem = BuildSystem.CMAKE
        self.OutputDirectory = ""
        self.IntermediateDirectory = ""
        self.BinaryDirectory = ""
        pass

    BuildTarget = ""
    BuildSystem = BuildSystem.CMAKE
    OutputDirectory = ""
    IntermediateDirectory = ""
    BinaryDirectory = ""


class ModuleParams:
    def __init__(self) -> None:
        self.Name = ""
        self.Location = ""
        self.SourceLocation = []
        self.HeaderLocation = []
        self.OutputDirectory = ""
        pass
    Name=""
    #These will contain the complete OS path
    Location=""         
    SourceLocation=[]
    HeaderLocation=[]
    OutputDirectory=""

class ModuleObject:
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
    
