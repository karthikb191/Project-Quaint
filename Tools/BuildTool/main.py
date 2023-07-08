import sys
import re
import os
import ast
import TemplateParser as Parser
from BuildParams import BuildSettings
from BuildParams import ModuleObject
import CMakeFileBuilder


#TODO: Read this from a settings file
RootDirectory = "D:\\Works\\Project-Quaint\\"
BuildTemplatesDirectory = RootDirectory + "Scripts\\BuildTemplates\\"
ExtensionName = ".buildTmpl"

BuildTarget = "Core"
BuildTargetDirectory = BuildTemplatesDirectory + BuildTarget + "\\"

BuildDirectory = "D:\\Works\\Project-Quaint\\Build\\"
IntermediateDirectory = BuildDirectory + "Intermediates\\"
OutputDirectory = BuildDirectory + "Output\\"
BinaryDirectory = BuildDirectory + "Bin\\"

GlobalSettings = BuildSettings()
RootModule = ModuleObject()

def InitDirectories():
    if not os.path.exists(OutputDirectory):
        os.makedirs(OutputDirectory)
    if not os.path.exists(IntermediateDirectory):
        os.makedirs(IntermediateDirectory)


def InitBuildSettings():
    InitDirectories()
    GlobalSettings.OutputDirectory = OutputDirectory
    GlobalSettings.IntermediateDirectory = IntermediateDirectory


def ParseTemplates():
    GlobalSettings.BuildTarget = BuildTarget
    CommonDictionary = Parser.ReadTemplateFile(os.path.join(BuildTemplatesDirectory, "Common" + ExtensionName))
    ParamDictionary = Parser.ReadTemplateFile(os.path.join(BuildTargetDirectory, BuildTarget + ExtensionName))
    module = ModuleObject()
    module.setModuleParams(ParamDictionary)
    ScanForSubmodules(BuildTargetDirectory, RootModule, (BuildTarget + ExtensionName))
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
    module = ModuleObject()
    module.setModuleParams(ParamDictionary)
    ScanForSubmodules(DirectoryPath, module, [templateFile])
    return module

if __name__ == "__main__":
    InitBuildSettings()
    ParseTemplates()
    builder = CMakeFileBuilder.CMakeBuilder(GlobalSettings, RootModule)
    CMakeFileBuilder.Build()