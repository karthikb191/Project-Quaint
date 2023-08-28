import os
import re
from io import TextIOWrapper
from BuildParams import ModuleObject
from BuildParams import ModuleType
from BuildParams import BuildSettings

class CMakeBuilder:
    def __init__(self, buildSettings : BuildSettings, rootModule : ModuleObject) -> None:
        self._Version = "3.10"
        self._BuildSettings = buildSettings
        self._RootModule = rootModule
        #Update Build settings to point to the current Root Module's directory
        #self._BuildSettings.BinaryDirectory = os.path.join(self._BuildSettings.BinaryDirectory, rootModule.Params.Name)
        #self._BuildSettings.IntermediateDirectory = os.path.join(self._BuildSettings.IntermediateDirectory, rootModule.Params.Name)
        #self._BuildSettings.OutputDirectory = os.path.join(self._BuildSettings.OutputDirectory, rootModule.Params.Name)
        self._BuildList = []
        binPath = os.path.join(self._BuildSettings.BinaryDirectory, self._RootModule.Params.Name)
        if not os.path.exists(binPath):
            os.makedirs(binPath)
        pass

    def BuildRootCMakeFile(self) -> None:
        FileName = os.path.join(self._BuildSettings.IntermediateDirectory, "CMakeLists.txt")
        fd = open(FileName, 'w', -1, "utf-8")

        if fd is None:
            return

        fd.write(f"cmake_minimum_required(VERSION {self._Version})\n")
        fd.write(f"project({self._RootModule.Params.Name})\n\n")
        
        BuildDir = os.path.join(self._BuildSettings.BinaryDirectory, self._RootModule.Params.Name)
        BuildDir = BuildDir.replace('\\', '/')
        #TODO: This might need to be handled some more 
        fd.write(f"set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY {BuildDir})\n"\
                    f"set(CMAKE_LIBRARY_OUTPUT_DIRECTORY {BuildDir})\n"\
                    f"set(CMAKE_RUNTIME_OUTPUT_DIRECTORY {BuildDir})\n\n")
        
        if("CMakeDefines" in self._BuildSettings.CommonSettings):
            self.AddCMakeDefinitions(fd, self._BuildSettings.CommonSettings["CMakeDefines"])
        if("CompileOptions" in self._BuildSettings.CommonSettings):
            self.AddCompileFlags(fd, self._BuildSettings.CommonSettings["CompileOptions"])
        if("PreProcessorDefines" in self._BuildSettings.CommonSettings):
            self.AddCompileDefinitions(fd, self._BuildSettings.CommonSettings["PreProcessorDefines"])
        self.AddNewLines(fd, 1)

        RootCMakeDir = os.path.join(self._BuildSettings.IntermediateDirectory, self._RootModule.Params.Name)
        RootCMakePath = os.path.join(RootCMakeDir, "CMakeLists.txt")
        RootCMakePath = RootCMakePath.replace('\\', '/')
        fd.write(f"include({RootCMakePath})")

        fd.close()
        pass
    
    def AddNewLines(self, fd : TextIOWrapper, num : int) -> None:
        newLineStr = ""
        while(num != 0):
            newLineStr += '\n'
            num -= 1
        
        fd.write(newLineStr)

    def WriteCMakeVersion(self, fd : TextIOWrapper, version : str) -> None:
        fd.write(f"cmake_minimum_required(VERSION {version})")
        self.AddNewLines(fd, 2)
        pass

    def WriteProjectName(self, fd : TextIOWrapper, name : str) -> None:
        fd.write(f"project({name})")
        self.AddNewLines(fd, 2)
        pass

    def AddCMakeDefinitions(self, fd : TextIOWrapper, definitions : dict[str, str]) -> None:
        if len(definitions) == 0 : return

        for definition in definitions:
            fd.write(f"set({definition} {definitions[definition]})\n")

        self.AddNewLines(fd, 1)
        pass

    def AddCompileFlags(self, fd : TextIOWrapper, flags : list[str]):
        if len(flags) == 0 : return

        flagString = ' '.join(flags)
        fd.write(f"add_compile_options({flagString})")
        self.AddNewLines(fd, 1)
        pass

    def AddCompileDefinitions(self, fd : TextIOWrapper, definitions : list[str]):
        if len(definitions) == 0 : return

        definitionString = ' '.join(definitions)
        fd.write(f"add_compile_definitions({definitionString})")
        self.AddNewLines(fd, 1)
        pass

    def AddProject(self, fd : TextIOWrapper, module : ModuleObject, isSubModule : bool = False) -> None:
        
        if(module.Params.PathInfo.get("SrcPaths") == None):
            fd.write(f"add_executable(${{PROJECT_NAME}} INTERFACE)")
            return
        
        ExcludeRegex = module.Params.PathInfo["SrcExcludePaths"]
        if (module.Type != ModuleType.EXECUTABLE) and (ExcludeRegex.count(".*[mM]ain.cpp") == 0):
            ExcludeRegex.append(".*[mM]ain.cpp")

        Sources = self.CollectSourceFiles(fd, module.Params.PathInfo["SrcPaths"], ExcludeRegex)
        #TODO: This logic feels off. Review it
        if(len(Sources) == 0) and not isSubModule:
            assert(module.Type != ModuleType.EXECUTABLE), "An Executable should atleast contain main.cpp"
            if(module.Type == ModuleType.EXECUTABLE):
                fd.write(f"add_executable(${{PROJECT_NAME}})")
            elif(module.Type == ModuleType.BUILD_STATIC):
                fd.write(f"add_library(${{PROJECT_NAME}} STATIC)")
            elif(module.Type == ModuleType.BUILD_DYNAMIC):
                fd.write(f"add_library(${{PROJECT_NAME}} SHARED)")
            else:
                assert(False), "Trying to build module of invalid type"
            return
        self.AddNewLines(fd, 2)

        if isSubModule:
            fd.write(f"target_sources(${{PROJECT_NAME}} PUBLIC ${{SOURCES}})")
            self.AddNewLines(fd, 2)
            return

        if(module.Type == ModuleType.EXECUTABLE):
            fd.write(f"add_executable(${{PROJECT_NAME}} ${{SOURCES}})")
        elif(module.Type == ModuleType.BUILD_STATIC):
            fd.write(f"add_library(${{PROJECT_NAME}} STATIC ${{SOURCES}})")
        elif(module.Type == ModuleType.BUILD_DYNAMIC):
            fd.write(f"add_library(${{PROJECT_NAME}} SHARED ${{SOURCES}})")
        else:
            assert(False), "Trying to build module of invalid type"

        self.AddNewLines(fd, 2)
        pass
        
    def CollectHeaderDirs(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        HeaderPaths = module.Params.PathInfo["HeaderPaths"]
        for path in HeaderPaths:
            path = path.replace('\\', '/')
            fd.write(f"include_directories({path})")
            self.AddNewLines(fd, 1)
            fd.write(f"target_include_directories(${{PROJECT_NAME}} PUBLIC {path})\n")
        self.AddNewLines(fd, 2)
        pass

    def CollectSourceFilesInDir(self, dirPath : str, excludeRegex : list[str]) -> list[str]:
        Sources = []
        dirList = os.listdir(dirPath)
        for path in dirList:
            fullPath = os.path.join(dirPath, path)
            fullPath = fullPath.replace('\\', '/')

            mustExclude = False
            for exclExpr in excludeRegex:
                RES = re.search(exclExpr, fullPath)
                if re.search(exclExpr, fullPath) != None:
                    mustExclude = True
                    break
            
            if mustExclude:
                continue

            if os.path.isfile(fullPath):
                if os.path.splitext(fullPath)[1] in self._BuildSettings.SourceExtensions:
                    Sources.append(fullPath)
            else:
                if fullPath not in excludeRegex:
                    Sources.extend(self.CollectSourceFilesInDir(fullPath, excludeRegex))

        return Sources
        pass

    def CollectSourceFiles(self, fd : TextIOWrapper, srcPaths : list[str], excludeRegex : list[str]) -> list[str]:
        sources = []

        for path in srcPaths:
            if not os.path.isdir(path):
                print("Path to source is not a directory. Check your source entries")
                continue
            sources.extend(self.CollectSourceFilesInDir(path, excludeRegex))


        sourceString = '\n'.join(sources)
        if len(sources) > 0 :
            fd.write(f"set(SOURCES {sourceString})")
            self.AddNewLines(fd, 2)
        return sources
        pass

    #TODO: Deprecate this
    def AddSubModuleIncludes(self, fd : TextIOWrapper, module : ModuleObject) -> None:
        for depModule in module.SubModules:
            if depModule.Params.Name in self._IncludeList:
                continue
            if(depModule.Params.Name == self._RootModule.Params.Name):
                continue

            depModule.Params.IntermediatePath = os.path.join(depModule.Params.IntermediatePath, module.Params.Name)
            if (depModule.Type is ModuleType.BUILD_STATIC) or (depModule.Type is ModuleType.BUILD_DYNAMIC):
                self._IncludeList.append(depModule.Params.Name)
                dirPath = os.path.join(self._BuildSettings.IntermediateDirectory, depModule.Params.IntermediatePath, depModule.Params.Name)
                fileName = os.path.join(dirPath, "CMakeLists.txt")

                fileName = fileName.replace('\\', '/')
                fd.write(f"include({fileName})")
                self.AddNewLines(fd, 1)

        self.AddNewLines(fd, 2)
        pass
    
    def AddDependentCMakeIncludes(self, fd : TextIOWrapper, dependencies : list[ModuleObject]) -> None:
        for depModule in dependencies:
            if(depModule.Type == ModuleType.EXECUTABLE) : continue
            
            if \
            (depModule.Params.Name not in self._DependencyList) and \
            (depModule.Params.Name != self._RootModule.Params.Name) and \
            (depModule.Type == ModuleType.BUILD_STATIC or depModule.Type == ModuleType.BUILD_DYNAMIC) :
                self._DependencyList.append(depModule.Params.Name)
                DirPath = os.path.join(self._BuildSettings.IntermediateDirectory, depModule.Params.IntermediatePath, depModule.Params.Name)
                FileName = os.path.join(DirPath, "CMakeLists.txt").replace('\\', '/')
                fd.write(f"include(\"{FileName}\")")
                self.AddNewLines(fd, 1)
        pass

    def AddDependencyModule(self, fd : TextIOWrapper, depModule : ModuleObject) -> None:
        #If any of the dependencies has ROOT module as dependency, skip and dont add it
        if(depModule.Params.Name == self._RootModule.Params.Name):
            return
        
        if (depModule.Type is ModuleType.BUILD_STATIC) or (depModule.Type is ModuleType.BUILD_DYNAMIC):
            #include CMakeLists of this Module
            if depModule.Params.Name not in self._BuildList:
                fd.write(f"add_dependencies(${{PROJECT_NAME}} {depModule.Params.Name})")
                self.AddNewLines(fd, 1)
            
            #binPath = os.path.join(self._BuildSettings.BinaryDirectory, depModule.Params.IntermediatePath, depModule.Params.Name)
            BinPath = os.path.join(self._BuildSettings.BinaryDirectory, self._RootModule.Params.Name)
            if not os.path.exists(BinPath):
                os.makedirs(BinPath)
            
            extension = self._BuildSettings.StaticLibExtension \
                if depModule.Type == ModuleType.BUILD_STATIC else self._BuildSettings.DynamicLibExtension  
            BinPath = os.path.join(BinPath, "${CMAKE_BUILD_TYPE}/" + depModule.Params.Name + extension)
            BinPath = BinPath.replace('\\', '/')
            fd.write(f"target_link_libraries(${{PROJECT_NAME}} {BinPath})")
            self.AddNewLines(fd, 2)
        
        elif (depModule.Type is ModuleType.STATIC):
            
            if(depModule.Params.PathInfo.get("HeaderPaths") != None):
                self.CollectHeaderDirs(fd, depModule)

            if len(depModule.Params.PathInfo["LibPaths"]) == 0:
                print(f"No Libs found in {depModule.Params.Name}. Only headers might be registered.")
                return
            
            #Key is the relative path from module. Value is the list of libraries to link from this path
            for libpath in depModule.Params.PathInfo["LibPaths"]:
                libFullPath = libpath + self._BuildSettings.StaticLibExtension
                libFullPath = libFullPath.replace('\\', '/')
                
                fd.write(f"target_link_libraries(${{PROJECT_NAME}} {libFullPath})\n")

            self.AddNewLines(fd, 2)

            pass
        #TODO: Add code to include and handle dlls
        pass

    def OpenCMakeFile(self, module : ModuleObject) -> TextIOWrapper :
        DirPath = os.path.join(self._BuildSettings.IntermediateDirectory, module.Params.IntermediatePath, module.Params.Name)
        FileName = os.path.join(DirPath, "CMakeLists.txt")
        
        DirPath = DirPath.replace('\\', '/')
        FileName = FileName.replace('\\', '/')
        if not os.path.exists(DirPath):
            os.makedirs(DirPath)
        return open(FileName, 'w', -1, "utf-8")
        
    def CloseCMakeFile(self, fd : TextIOWrapper):
        fd.close()
        pass

    def _Build(self, module : ModuleObject, isSubModule : bool = False):
        assert(module.Type == ModuleType.BUILD_STATIC or module.Type == ModuleType.BUILD_DYNAMIC\
                or module.Type == ModuleType.EXECUTABLE), "Trying to build an invalid module"
        self._BuildList.append(module.Params.Name)
        
        fd = self.OpenCMakeFile(module)

        if not isSubModule:
            self.WriteCMakeVersion(fd, "3.10")

        #Populate cmake variables, compile options and compile definitions here so that they are passed to subsequent modules being built
        self.AddCMakeDefinitions(fd, module.CMakeDefines)
        self.AddCompileFlags(fd, module.CompileOptions)
        self.AddCompileDefinitions(fd, module.PreProcessorDefines)
        self.AddNewLines(fd, 1)
        
        #Include dependent CMakeLists at the very beginning
        self.AddDependentCMakeIncludes(fd, module.Dependencies)

        if not isSubModule:
            self.WriteProjectName(fd, module.Params.Name)

        self.AddProject(fd, module, isSubModule)



        if(module.Params.PathInfo.get("HeaderPaths") != None):
            self.CollectHeaderDirs(fd, module)

        #Add any Submodule include. Submodules don't define "project". Their sources target last defined project
        self.AddDependentCMakeIncludes(fd, module.SubModules)

        #TODO: Deprecate this
        #Include all submodules/dependency headers if they are not already included
        #self.AddSubModuleIncludes(fd, module)

        #Generate subModule Make files before adding dependencies
        for depModule in module.SubModules:
            assert(depModule.Type != ModuleType.EXECUTABLE), "SubModule cannot be an executable. Check your structure!"
            if(depModule.Type == ModuleType.BUILD_STATIC or depModule.Type == ModuleType.BUILD_DYNAMIC):
                self._Build(depModule, True)

        #Dependencies are Modules/External Libs that are explictly specified to be built. Need to be addressed
        #!TODO: This needs to be tested
        for depModule in module.Dependencies:
            if(depModule.Type == ModuleType.EXECUTABLE) : continue

            self.AddDependencyModule(fd, depModule)
            if depModule.Params.Name not in self._BuildList\
            and (depModule.Type == ModuleType.BUILD_STATIC or depModule.Type == ModuleType.BUILD_DYNAMIC) :
                self._Build(depModule)

        self.CloseCMakeFile(fd)
        pass

    def StartBuild(self):
        self.BuildRootCMakeFile()
        self._Build(self._RootModule)
        pass

    _RootModule = ModuleObject()
    _BuildSettings = BuildSettings()
    _BuildList = []
    _IncludeList = []
    _DependencyList = []

