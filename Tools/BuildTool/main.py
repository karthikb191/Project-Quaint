import sys
import re
import os
import ast
import argparse
#import TemplateParser as Parser
from typing import Any
import JsonTemplateParser as Parser
from BuildParams import BuildSettings
from BuildParams import ModuleObject
from BuildParams import ModuleType
from Configs.Args import Args
from GUI.App import BuildTool
import CMakeFileBuilder


#TODO: Read this from a settings file
GlobalSettings = BuildSettings()
RootDirectory = "C:\\Works\\Project-Quaint\\"
RelativeTemplatesDirectory = "Scripts\\BuildTemplates\\"
BuildTemplatesDirectory = RootDirectory + RelativeTemplatesDirectory
ExtensionName = ".json"

BuildTargetDirectory = BuildTemplatesDirectory
#BuildTarget = "Core\\Core" + ExtensionName
BuildTarget = "Bolt\\Bolt" + ExtensionName
#BuildTarget = "Core\\Types" + ExtensionName
#BuildTarget = "Media\\Media" + ExtensionName
#BuildTarget = "Media\\Video" + ExtensionName

BuildDirectory = "C:\\Works\\Project-Quaint\\Build\\"
IntermediateDirectory = BuildDirectory + "Intermediates\\"
OutputDirectory = BuildDirectory + "Output\\"
BinaryDirectory = BuildDirectory + "Bin\\"

bForceRootExecutable = True

RootModule = ModuleObject()

def InitDirectories():
    if not os.path.exists(OutputDirectory):
        os.makedirs(OutputDirectory)
    if not os.path.exists(IntermediateDirectory):
        os.makedirs(IntermediateDirectory)


def InitBuildSettings():
    InitDirectories()
    BuildSettings.RootDirectory = RootDirectory
    BuildSettings.OutputDirectory = OutputDirectory
    BuildSettings.IntermediateDirectory = IntermediateDirectory
    BuildSettings.BinaryDirectory = BinaryDirectory
    BuildSettings.BuildTarget = BuildTarget

def ParseCommonTemplate():
    BuildSettings.CommonSettings = Parser.ReadTemplateFile(os.path.join(BuildTemplatesDirectory, "Common" + ExtensionName))

#TODO: Room for improvement here
def FindModule(module : ModuleObject, moduleToFind : str) -> ModuleObject | None:
    processedModules : list[ModuleObject] = []
    stack : set[ModuleObject] = {module}
    
    resModule = None
    while(len(stack)) > 0:
        currentModule = stack.pop()
        if(currentModule.Params.Name == moduleToFind):
            resModule = currentModule
            break
        
        # for subModule in module.SubModules:
        #     if (subModule not in processedModules):
        #         stack.add(subModule)

        for dependency in module.Dependencies:
            if (dependency not in processedModules):
                stack.add(dependency)

        processedModules.append(currentModule)
    
    if(module.Params.Name == moduleToFind):
        return module

    return resModule

# Checks if module is already marked to be built
def IsModuleResolved(moduleName : str) -> tuple[bool, ModuleObject | None]:
    #TODO: Room for improvement here
    resModule = FindModule(RootModule, moduleName)

    if resModule == None:
        return (False, None)

    if resModule.Resolved == False:
        return (False, resModule)

    return (True, resModule)

def ParseTemplate(templatePath : str, ModuleRef : ModuleObject):
    ParamDictionary : dict[str, Any] | None = Parser.ReadTemplateFile(templatePath)

    ModuleRef.setModuleParams(ParamDictionary)
    ModuleRef.setResolved()

    dirPath = os.path.dirname(templatePath)
    return

#TODO: Convert this to module and use relative imports
def Init():
    res = GatherArguments()
    if(res == False):
        print("Arguments gathering failed", file=sys.stderr)
        return
    
    BuildTool.Run()
    sys.exit(BuildTool.Application.exec())

def GatherArguments() -> (bool):
    
    parser = argparse.ArgumentParser(
        prog="CMake File Generator Tool",
        description="Generates CMake files based on the .json build template files"
    )

    parser.add_argument("-c", "--config", required=True,
                        help="Choose configuration to generate cmake files for. Choices[debug, release]")
    parser.add_argument("-p", "--platform", required=True,
                        help="Provide platform to generate cmake files for. Choices[windows]")
    parser.add_argument("--path" , required=True,
                        help="Path to the Project file .json template")
    try:
        args = parser.parse_args()
        print(f"Args Entered: {args}")
        Args.Init(platform=args.platform, config=args.config, projectFilePath=args.path)
    except argparse.ArgumentError:
        print(f"Error parsing arguments: {argparse.ArgumentError.message}")
        return False

    if(not Args.Initialized):
        return False
    
    return True

if __name__ == "__main__":
    Init();
    pass

def LegacyInit():
    InitBuildSettings()
    ParseCommonTemplate()

    RootModule.Params.ModulePath = RelativeTemplatesDirectory + BuildTarget
    RootModule.Params.Name = BuildTarget

    stack : set[ModuleObject] = {RootModule}
    resolvedModules : dict = {}

    while(len(stack) > 0):
        module : ModuleObject = stack.pop()
        if module.Resolved == True:
            print("Resolved module is added to stack. Shouldn't happen")
            continue

        path = os.path.join(BuildSettings.RootDirectory, module.Params.ModulePath)
        ParseTemplate(path, module)
        resolvedModules[module.Params.Name] = module

        # Add any dependencies that need to be resolved
        for i in range(len(module.Dependencies)):
            dependency = module.Dependencies[i]
            if dependency.Type == ModuleType.MODULE:
                if(dependency.Params.Name in resolvedModules):
                    module.Dependencies[i] = resolvedModules[dependency.Params.Name]
                else:
                    stack.add(module.Dependencies[i])


    if bForceRootExecutable and RootModule.Type != ModuleType.EXECUTABLE:
        RootModule.Type = ModuleType.EXECUTABLE

    builder = CMakeFileBuilder.CMakeBuilder(GlobalSettings, RootModule)
    builder.StartBuild()
    print("Project Generation Complete")