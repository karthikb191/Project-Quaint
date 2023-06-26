import sys
import re
import os
import ast
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


def IdentifyParamTypeAndCleanup(Param : str) -> tuple[str, str] | None:
    if (Param is None) or (len(Param) == 0):
        return (None, "")
    
    EvaluatingString = False
    CleanedParam = ""
    for c in Param:
        if c is "\"" or c is "\'":
            EvaluatingString = not EvaluatingString
        
        if c is " " and not EvaluatingString:
            continue
        
        CleanedParam += c

    Type = ""
    if len(CleanedParam) == 0 :
        return (None, CleanedParam) 
    c = CleanedParam[0]
    if c is '{' : 
        Type = "Dictionary"
        #CleanedParam = CleanedParam[1:len(CleanedParam)-1]
    elif c is '[' or c is '(':
        Type = "List"
        #CleanedParam = CleanedParam[1:len(CleanedParam)-1]
    elif ord(c) >= 48 and ord(c) <= 57:
        Type = "Number"
    else:
        Type = "String"

    return (Type, CleanedParam)

def ParseNumber(Param, Index) -> tuple[int | float, int]:
    assert ord(Param[Index]) >= 48 and ord(Param[Index]) <= 57

    NumRes = Param[Index]
    while(Param[Index + 1] is not ',' 
          and Param[Index + 1] is not ']'
          and Param[Index + 1] is not ')'
          and Param[Index + 1] is not '}'):
        NumRes += Param[Index + 1]
        Index += 1

    if(NumRes.count('.') != 0):
        return (float(NumRes), Index)
    return (int(NumRes), Index)

def ParseString(Param, Index) -> tuple[str, int]:
    assert Param[Index] is '\"' or Param[Index] is '\''
    
    StringRes = ""
    Index += 1

    while(Param[Index] is not '\"' and Param[Index] is not '\''):
        StringRes += Param[Index]
        Index += 1
    
    return (StringRes, Index)

def ParseList(Param, Index) -> tuple[list, int]:
    assert Param[Index] == '['
    
    ListRes = []
    Index += 1
    while Param[Index] != ']':
        if Param[Index] is '\"' or Param[Index] is '\'':
            (Res, Index) = ParseString(Param, Index)
            ListRes.append(Res)
            pass
        elif Param[Index] is '[' or Param[Index] is '(':
            (Res, Index) = ParseList(Param, Index)
            ListRes.append(Res)
        elif Param[Index] is '{':
            (Res, Index) = ParseList(Param, Index)
            ListRes.append(Res)
        elif ord(Param[Index]) >= 48 and ord(Param[Index]) <= 57:
            (Res, Index) = ParseNumber(Param, Index)
            ListRes.append(Res)
        else:
            assert False, "Invalid Symbol Encountered when Parsing List"

        Index += 1
        assert Param[Index] is ',' or Param[Index] is ']'
        if Param[Index] is ',':
            Index += 1
        pass

    return (ListRes, Index)

def ParseDictionary(Param, Index) -> tuple[dict, int]:
    assert Param[Index] == '{'
    
    DictRes = {}
    Index += 1

    while Param[Index] != '}':
        Key = ""
        if Param[Index] is '\"' or Param[Index] is '\'':
            (Key, Index) = ParseString(Param, Index)
            pass

        Index += 1
        assert Param[Index] == ':'
        Index += 1

        if Param[Index] is '\"' or Param[Index] is '\'':
            (Value, Index) = ParseString(Param, Index)
            DictRes[Key] = Value
            pass
        elif Param[Index] is '[' or Param[Index] is '(':
            (Value, Index) = ParseList(Param, Index)
            DictRes[Key] = Value
            pass
        elif Param[Index] is '{':
            (Value, Index) = ParseDictionary(Param, Index)
            DictRes[Key] = Value
            pass
        else:
            assert False, "Invalid Symbol Encountered when Parsing dictionary"

        Index += 1
        assert Param[Index] is ',' or Param[Index] is '}'
        if Param[Index] is ',':
            Index += 1

    return(DictRes, Index)


def ProcessParam(Param) -> dict | list | str | None:
    (Type, ResParam) = IdentifyParamTypeAndCleanup(Param)
    Res = {}
    if Type is "Dictionary":
        #DictItems = ResParam.split(',')
        #ParamDs = ast.literal_eval(ResParam)
        (Res, Index) = ParseDictionary(ResParam, 0)
        #for Item in DictItems:
        #    (Key, Value) = Item.split(':')
        #    ParamDs[Key] = ProcessParam(Value)
            #ParamDs[elem] = ProcessParam(ParamDs[elem])
    elif Type is "List":
        (Res, Index) = ParseList(ResParam, 0)
        #ParamDs = ResParam.split(',')
        #for index, elem in enumerate(ParamDs):
        #    ParamDs[index] = ProcessParam(elem)
    elif Type is "Number":
        (Res, Index) = ParseNumber(ResParam, 0)
    elif Type is "String":
        (Res, Index) = ParseString(ResParam, 0)
    else:
        assert False, "Trying to parse invalid type"

    return Res

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
    
    ParamDictionary = {}
    for Params in ModuleParamItr:
        #CleanedStr = re.sub("(@:)|(:@)|[\n\r\s]", "", Params.group())
        CleanedStr = re.sub("(@:)|(:@)|[\n\r]", "", Params.group())
        MyList = CleanedStr.split("=")
        ParamDictionary[MyList[0]] = ProcessParam(MyList[1])
        print(MyList)

    stream.close()
    
    return module

if __name__ == "__main__":
    InitializeBuild()