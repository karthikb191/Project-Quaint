import sys
import re
import os
import ast
import TemplateParser as Parser
from BuildParams import BuildSettings
from BuildParams import ModuleObject
from BuildParams import ModuleType
import CMakeFileBuilder


#TODO: Read this from a settings file
GlobalSettings = BuildSettings()
RootDirectory = "D:\\Works\\Project-Quaint\\"
BuildTemplatesDirectory = RootDirectory + "Scripts\\BuildTemplates\\"
ExtensionName = ".buildTmpl"

BuildTargetDirectory = BuildTemplatesDirectory
BuildTarget = "Core\\Memory\\Memory" + ExtensionName

BuildDirectory = "D:\\Works\\Project-Quaint\\Build\\"
IntermediateDirectory = BuildDirectory + "Intermediates\\"
OutputDirectory = BuildDirectory + "Output\\"
BinaryDirectory = BuildDirectory + "Bin\\"

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
    CommonDictionary = Parser.ReadTemplateFile(os.path.join(BuildTemplatesDirectory, "Common" + ExtensionName))

# Checks if module is already marked to be built
def FindModule(module : ModuleObject, moduleToFind : str) -> ModuleObject | None:
    processedModules : list[ModuleObject] = []
    stack : set[ModuleObject] = {module}
    
    resModule = None
    while(len(stack)) > 0:
        currentModule = stack.pop()
        if(currentModule.Params.Name == moduleToFind):
            resModule = currentModule
            break
        
        for subModule in module.SubModules:
            if (subModule not in processedModules):
                stack.add(subModule)

        for dependency in module.Dependencies:
            if (dependency not in processedModules):
                stack.add(dependency)

        processedModules.append(currentModule)
    
    if(module.Params.Name == moduleToFind):
        return module

    return resModule

def IsModuleResolved(moduleName : str) -> tuple[bool, ModuleObject]:
    resModule = FindModule(RootModule, moduleName)

    if resModule == None:
        return (False, None)

    if resModule.Type == ModuleType.MODULE:
        return (False, resModule)

    return (True, resModule)

def ParseTemplate(templatePath : str, ModuleRef : ModuleObject):
    #TODO: Add some invalid/fail conditions
    ParamDictionary = Parser.ReadTemplateFile(templatePath)

    dirName = os.path.basename(os.path.dirname(templatePath))
    ModuleRef.setModuleParams(ParamDictionary)

    dirPath = os.path.dirname(templatePath)
    if(dirName == ModuleRef.Params.Name):
        ScanForSubmodules(dirPath, ModuleRef, ModuleRef.TemplateFile)
    
    # If there's a dependency and it's a "Module Type" file, Parse the dependency chain
    # Module's params will be overwritten with parsed values
    for i in range(len(ModuleRef.Dependencies)):
        if ModuleRef.Dependencies[i].Type == ModuleType.MODULE:
            (resolved, module) = IsModuleResolved(ModuleRef.Dependencies[i].Params.Name)
            
            if (resolved):
                #if dependency module is already resolved, override current dependency param with resolved one
                ModuleRef.Dependencies[i] = module
                pass
            else:
                #if dependency module is not resolved, Parse template
                dependencyTemplatePath = os.path.join(BuildSettings.RootDirectory, ModuleRef.Dependencies[i].Params.ModulePath)
                ParseTemplate(dependencyTemplatePath, ModuleRef.Dependencies[i])

    return

def ScanForSubmodules(Directory, ParentModule : ModuleObject, Excludes = []):
    iterator = os.walk(Directory)
    (dirPath, dirNames, fileNames) = next(iterator)

    for templateFile in fileNames:
        (root, ext) = os.path.splitext(templateFile)
        if(ext != ExtensionName):
            print(templateFile + " has an invalid extension and cannot be read")
            continue
        
        if templateFile in Excludes:
            continue
    
        ParamDictionary = Parser.ReadTemplateFile(os.path.join(Directory, templateFile))
        module = ModuleObject()
        module.setModuleParams(ParamDictionary)
        if module is not None:
            ParentModule.SubModules.append(module)
            module.ParentModule = ParentModule
    
    for templateFolder in dirNames:
        module = ReadTemplateDirectory(os.path.join(Directory, templateFolder))
        if module is not None:
            ParentModule.SubModules.append(module)
            module.ParentModule = ParentModule
    return

def ReadTemplateDirectory(DirectoryPath):
    #Check for a template file in the current directory. Fail if it's not present
    templateFile = os.path.basename(DirectoryPath) + ExtensionName
    templateFilePath = os.path.join(DirectoryPath, templateFile)
    if not os.path.exists(templateFilePath):
        print("Every Sub folder should contain a template with with same name as folder")
        return None
    
    ParamDictionary = Parser.ReadTemplateFile(templateFilePath)
    module = ModuleObject()
    module.setModuleParams(ParamDictionary)
    ScanForSubmodules(DirectoryPath, module, [templateFile])
    return module

if __name__ == "__main__":
    InitBuildSettings()
    ParseCommonTemplate()
    ParseTemplate(BuildTemplatesDirectory + BuildTarget, RootModule)
    builder = CMakeFileBuilder.CMakeBuilder(GlobalSettings, RootModule)
    builder.StartBuild()