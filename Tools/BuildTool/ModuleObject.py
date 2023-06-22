class ModuleParams:
    Name=""

class ModuleOject:
    def __init__(self):
        self.TemplateFile = ""
        self.Params = ModuleParams()
        self.SubModules = []
        self.CMakeDefines = []
        self.CompileOptions = []
        self.LinkerOptions = []
        self.Dependencies = []
        self.BuildFlags = []
        self.PreProcessorDefines = []


    def setModuleParamsFromDictionary(self, params = {}):
        if(params["Name"]):
            self.Params.Name = params["Name"]
        return

    TemplateFile=""
    Params = ModuleParams()
    SubModules=[]
    CMakeDefines=[]
    CompileOptions = []
    LinkerOptions = []
    Dependencies = []
    BuildFlags = []
    PreProcessorDefines = []
    
