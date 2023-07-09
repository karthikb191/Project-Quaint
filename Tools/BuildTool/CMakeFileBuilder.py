import os
from io import TextIOWrapper
from BuildParams import ModuleObject
from BuildParams import ModuleType
from BuildParams import BuildSettings

class CMakeBuilder:
    def __init__(self, buildSettings : BuildSettings, rootModule : ModuleObject) -> None:
        self._BuildSettings = buildSettings
        self._RootModule = rootModule
        #Update Build settings to point to the current Root Module's directory
        self._BuildSettings.BinaryDirectory = os.path.join(self._BuildSettings.BinaryDirectory, rootModule.Params.Name)
        self._BuildSettings.IntermediateDirectory = os.path.join(self._BuildSettings.IntermediateDirectory, rootModule.Params.Name)
        self._BuildSettings.OutputDirectory = os.path.join(self._BuildSettings.OutputDirectory, rootModule.Params.Name)
        self._BuildList = []
        pass
    
    def AddNewLines(self, fd : TextIOWrapper, num : int) -> None:
        newLineStr = ""
        while(num != 0):
            newLineStr += '\n'
            num -= 1
        
        fd.write(newLineStr)

    def WriteCMakeVersion(self, fd : TextIOWrapper, version : str) -> None:
        fd.write(f"cmake_minimum_required(VERSION{version})")
        self.AddNewLines(fd, 2)
        pass

    def WriteProjectName(self, fd : TextIOWrapper, name : str) -> None:
        fd.write(f"project({name})")
        self.AddNewLines(fd, 1)
        pass

    def AddProject(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        
        if(module.Params.PathInfo.get("SrcPaths") == None):
            fd.write(f"add_executable({module.Params.Name} INTERFACE)")
            return
                
        Sources = self.CollectSourceFiles(fd, module.Params.PathInfo["SrcPaths"])
        if(len(Sources) == 0):
            fd.write(f"add_executable({module.Params.Name} INTERFACE)")
            return

        if(module.Type == ModuleType.EXECUTABLE):
            fd.write(f"add_executable({module.Params.Name} SOURCES)")
        elif(module.Type == ModuleType.BUILD_STATIC):
            fd.write(f"add_library({module.Params.Name} STATIC SOURCES)")
        elif(module.Type == ModuleType.BUILD_DYNAMIC):
            fd.write(f"add_library({module.Params.Name} SHARED SOURCES)")
        else:
            assert(False), "Trying to build module of invalid type"

        self.AddNewLines(fd, 2)
        pass
        
    def CollectHeaderDirs(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        HeaderPaths = module.Params.PathInfo["HeaderPaths"]
        for path in HeaderPaths:
            fd.write(f"include_directories({path})")
            self.AddNewLines(fd, 1)
            fd.write(f"target_include_directories({module.Params.Name} PUBLIC {path})")
            self.AddNewLines(fd, 2)
        self.AddNewLines(fd, 2)
        pass

    def CollectSourceFilesInDir(self, fd : TextIOWrapper, dirPath : str, excludesPaths : list[str]) -> list[str]:
        Sources = []
        dirList = os.listdir(dirPath)
        for path in dirList:
            if os.path.isfile(path):
                if os.path.splitext(path) in self._BuildSettings.SourceExtensions:
                    Sources.extend(path)
            else:
                if path not in excludesPaths:
                    Sources.extend(self.CollectSourceFilesInDir(path))

        return Sources
        pass

    def CollectSourceFiles(self, fd : TextIOWrapper, srcPaths : list[str], excludePaths : list[str]) -> list[str]:
        sources = []

        for path in srcPaths:
            if not os.path.isdir(path):
                print("Path to source is not a directory. Check your source entries")
                continue
            sources.extend(self.CollectSourceFilesInDir(path), excludePaths)


        sourceString = ',\n'.join(sources)
        fd.write(f"set(SOURCES {sourceString})")
        return sources
        pass

    def AddSubModuleIncludes(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        for depModule in module.SubModules:
            if depModule.Params.Name in self._IncludeList:
                continue
            if(depModule.Params.Name == self._RootModule.Params.Name):
                continue
            
            if (depModule.Type is ModuleType.BUILD_STATIC) or (depModule.Type is ModuleType.BUILD_DYNAMIC):
                self._IncludeList.append(depModule.Params.Name)
                dirPath = os.path.join(self._BuildSettings.IntermediateDirectory, module.Params.IntermediatePath, module.Params.Name)
                fileName = os.path.join(dirPath, "CMakeLists.txt")
                fd.write(f"include({fileName})")
                self.AddNewLines(fd, 1)

        self.AddNewLines(fd, 1)
        pass

    def AddDependencies(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        for depModule in module.SubModules:
            if(depModule.Params.Name == self._RootModule.Params.Name):
                continue
            
            if (depModule.Type is ModuleType.BUILD_STATIC) or (depModule.Type is ModuleType.BUILD_DYNAMIC):
                #include CMakeLists of this Module
                if depModule.Params.Name not in self._BuildList:
                    fd.write(f"add_dependencies({module.Params.Name} {depModule.Params.Name})")
                    self.AddNewLines(fd, 1)
                
                binPath = os.path.join(self._BuildSettings.BinaryDirectory, depModule.Params.Name)
                extension = self._BuildSettings.StaticLibExtension \
                    if depModule.Type == ModuleType.BUILD_STATIC else self._BuildSettings.DynamicLibExtension  
                binPath = os.path.join(binPath, depModule.Params.Name + extension)
                fd.write(f"target_link_libraries({module.Params.Name} {binPath})")
                self.AddNewLines(fd, 2)

            #TODO: Add code to include libs and dlls
        pass

    def OpenCMakeFile(self, module : ModuleObject) -> TextIOWrapper :
        DirPath = os.path.join(self._BuildSettings.IntermediateDirectory, module.Params.IntermediatePath, module.Params.Name)
        FileName = os.path.join(DirPath, "CMakeLists.txt")
        return open(FileName, 'w')
        pass
        
    def CloseCMakeFile(self, fd : TextIOWrapper):
        fd.close()
        pass

    def _Build(self, module : ModuleObject):
        assert(module.Type == ModuleType.BUILD_STATIC or module.Type == ModuleType.BUILD_DYNAMIC\
                or module.Type == ModuleType.EXECUTABLE), "Trying to build an invalid module"
        self._BuildList.append(module.Params.Name)
        
        fd = self.OpenCMakeFile(module)
        self.WriteCMakeVersion(fd, "3.10")
        self.WriteProjectName(fd, module.Params.Name)        

        self.AddProject(fd, module)

        if(module.Params.PathInfo.get("HeaderPaths") != None):
            self.CollectHeaderDirs(fd, module)

        #Include all submodules/dependency headers if they are not already included
        self.AddSubModuleIncludes(fd, module)

        #Generate subModule Make files before adding dependencies
        for depModule in module.SubModules:
            assert(module.Type != ModuleType.EXECUTABLE), "Dependency Module cannot be an executable. Check your structure!"
            if(module.Type == ModuleType.BUILD_STATIC or module.Type == ModuleType.BUILD_DYNAMIC):
                depModule.Params.IntermediatePath += module.Params.Name + "/"
                self.Build(depModule)

        self.AddDependencies(fd, module)
        self.CloseCMakeFile(fd)
        pass

    def StartBuild(self):
        self._Build(self._RootModule)
        pass

    _RootModule = ModuleObject()
    _BuildSettings = BuildSettings()
    _BuildList = []
    _IncludeList = []

