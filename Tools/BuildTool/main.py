import sys
import re
import os
import ast
import TemplateParser as Parser
from BuildParams import GlobalBuildSettings
from BuildParams import ModuleOject


#TODO: Read this from a settings file
RootDirectory = "D:\\Works\\Project-Quaint\\"
BuildTemplatesDirectory = RootDirectory + "Scripts\\BuildTemplates\\"
ExtensionName = ".buildTmpl"

BuildTargetDirectory = BuildTemplatesDirectory + "Core\\"
BuildTarget = "Core"

GlobalSettings = GlobalBuildSettings()
RootModule = ModuleOject()

def InitializeBuild():
    GlobalSettings.BuildTarget = BuildTarget
    CommonDictionary = Parser.ReadTemplateFile(os.path.join(BuildTemplatesDirectory, "Common" + ExtensionName))
    ParamDictionary = Parser.ReadTemplateFile(os.path.join(BuildTargetDirectory, BuildTarget + ExtensionName))
    module = ModuleOject()
    module.setModuleParams(ParamDictionary)
    ScanForSubmodules(BuildTargetDirectory, RootModule, (BuildTarget + ExtensionName))
    return

def ScanForSubmodules(Directory, ParentModule : ModuleOject, Excludes = []):
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
        module = ModuleOject()
        module.setModuleParams(ParamDictionary)
        if module is not None:
            ParentModule.SubModules.append(module)
    
    for templateFolder in dirNames:
        module = ReadTemplateDirectory(os.path.join(Directory, templateFolder))
        if module is not None:
            ParentModule.SubModules.append(module)
    return

def ReadTemplateDirectory(DirectoryPath):
    #Check for a template file in the current directory. Fail if it's not present
    templateFile = os.path.basename(DirectoryPath) + ExtensionName
    templateFilePath = os.path.join(DirectoryPath, templateFile)
    if not os.path.exists(templateFilePath):
        print("Every Sub folder should contain a template with with same name as folder")
        return None
    
    ParamDictionary = Parser.ReadTemplateFile(templateFilePath)
    module = ModuleOject()
    module.setModuleParams(ParamDictionary)
    ScanForSubmodules(DirectoryPath, module, [templateFile])
    return module

if __name__ == "__main__":
    InitializeBuild()