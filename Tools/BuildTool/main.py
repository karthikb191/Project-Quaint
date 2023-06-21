import sys
import re

#TODO: Read this from a settings file
RootDirectory = "D:\\Works\\Project-Quaint\\"
BuildTemplatesDirectory = RootDirectory + "Scripts\\BuildTemplates\\"

ModuleParams = {
    "Settings" : {}
}

def ReadTemplateFile():
    TemplateFile = BuildTemplatesDirectory + "Bolt.buildTmpl"
    stream = open(TemplateFile, "r")
    
    contents = stream.read()
    
    ModuleParamItr = re.finditer("(@:)(.*(\n|\r|\r\n))+?(:@)", contents)

    if(ModuleParamItr == None):
        print("No Module params found for this Module. This will skip CMakeLists generation")
    
    for Params in ModuleParamItr:
        CleanedStr = re.sub("(@:)|(:@)|[\n\s]", "", Params.group())
        MyList = CleanedStr.split("=")
        print(MyList)
        if MyList[0] not in ModuleParams:
            print("Invalid Setting {} encountered. Skipping this", MyList[0])
            continue

    stream.close()
    pass

print(RootDirectory + "  " + BuildTemplatesDirectory)
ReadTemplateFile()
print('La la la la')