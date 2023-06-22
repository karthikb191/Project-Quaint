import sys
import re
import os
from GlobalBuildSettings import GlobalBuildSettings
from ModuleObject import ModuleOject


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
    RootModule = ReadTemplateFile(os.path.join(BuildTargetDirectory, BuildTarget + ExtensionName))
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
    
        module = ReadTemplateFile(os.path.join(Directory, templateFile))
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
    
    module = ReadTemplateFile(templateFilePath)
    ScanForSubmodules(DirectoryPath, module, [templateFile])
    return module


def ReadTemplateFile(TemplateFilePath):
    if not os.path.isfile(TemplateFilePath):
        print("Encountered something that's not a file when trying to read template file")
        return None

    module = ModuleOject()
    module.TemplateFile = os.path.basename(TemplateFilePath)
    stream = open(TemplateFilePath, "r")
    contents = stream.read()
    
    ModuleParamItr = re.finditer("(@:)(.*(\n|\r|\r\n))+?(.*:@)", contents)

    if(ModuleParamItr == None):
        print("No Module params found for this Module. This will skip CMakeLists generation")
    
    for Params in ModuleParamItr:
        CleanedStr = re.sub("(@:)|(:@)|[\n\r\s]", "", Params.group())
        MyList = CleanedStr.split("=")
        print(MyList)

    stream.close()
    
    return module

if __name__ == "__main__":
    InitializeBuild()