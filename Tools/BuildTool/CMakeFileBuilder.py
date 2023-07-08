import os
from io import TextIOWrapper
from BuildParams import ModuleObject
from BuildParams import ModuleType
from BuildParams import BuildSettings

class CMakeBuilder:
    def __init__(self, buildSettings, rootModule) -> None:
        self._BuildSettings = buildSettings
        self._Module = rootModule
        self._BuildList = []
        pass
    
    def AddNewLines(self, fd : TextIOWrapper, num : int) -> None:
        newLineStr = ""
        while(num != 0):
            newLineStr += '\n'
            num -= 1
        
        fd.write(newLineStr)

    def WriteCMakeVersion(self, fd : TextIOWrapper, version : str) -> None:
        fd.write("cmake_minimum_required(VERSION{ver})".format(ver=version))
        self.AddNewLines(fd, 2)
        pass

    def WriteProjectName(self, fd : TextIOWrapper, name : str) -> None:
        fd.write("project({nm})".format(nm=name))
        pass
        
    def CollectHeaderFiles(self, fd : TextIOWrapper, path : list[str]) -> None:

        pass

    def CollectSourceFiles(self, fd : TextIOWrapper, path : list[str]) -> None:
        
        pass

    def AddDependencies(self, fd : TextIOWrapper, dependency : ModuleObject) -> None:
        
        pass

    def OpenCMakeFile(self, module : ModuleObject) -> TextIOWrapper :
        FileName = self._BuildSettings.IntermediateDirectory + module.Params.OutputSubDirectory + module.Params.Name + ".txt"
        return open(FileName, 'w')
        
    def CloseCMakeFile(self, fd : TextIOWrapper):
        fd.close()

    def Build(self, module : ModuleObject):
        fd = self.OpenCMakeFile(module)
        self.WriteCMakeVersion(fd, "3.10")
        self.WriteProjectName(fd, module.Params.Name)
        self.CollectSourceFiles(fd, module.Params.SourceLocation)

        self.CloseCMakeFile(fd)
        pass

    _Module = ModuleObject()
    _BuildSettings = BuildSettings()
    _BuildList = []

