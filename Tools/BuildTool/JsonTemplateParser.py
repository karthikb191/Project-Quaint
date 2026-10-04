import json;
from typing import Any

OverrideFlags = {
    "UNKNOWN_PLATFORM" : 0,
    "WIN32" : 1,
    "DEBUG_BUILD" : 1
}

def ReadTemplateFile(TemplateFilePath) -> dict[str, Any] | None:
    parsedFile : dict = {}

    with open(TemplateFilePath, 'r', encoding='utf-8') as file:
        parsedFile = json.load(file)

    EvaluateData(parsedFile)

    # overrides = parsedFile["Overrides"]
    # for key, val in overrides.items():
    #     if(key in OverrideFlags == False): 
    #         continue

    #     for itemKey, itemVal in overrides[key].items():
    #         if(itemVal is list):
    #             if parsedFile[itemKey] is None:
    #                 parsedFile[itemKey] = []
    #             parsedFile[itemKey].append(itemVal)
    #         elif(itemVal is dict):
    #             if parsedFile[itemKey] is None:
    #                 parsedFile[itemKey] = {}
    #             parsedFile[itemKey].update(itemVal)
    #         else:
    #             parsedFile[itemKey] = itemVal

    # # We should've accumulated the necessary values from override now.
    # parsedFile["Overrides"] = None

    return parsedFile

def EvaluateData(data : dict | list):
    if(isinstance(data, list)):
        for item in data:
            EvaluateData(item)
    elif(isinstance(data, dict)):
        GatherOverrides(data)
        for _, val in data.items():
            EvaluateData(val);

def GatherOverrides(blockDict: dict):
    if(blockDict.get("Overrides") is None):
        return
    
    overrides = blockDict["Overrides"]

    # inject override items into the current dictionary
    for key, val in overrides.items():
        if(key not in OverrideFlags): 
            continue

        for itemKey, itemVal in overrides[key].items():
            if(isinstance(itemVal, list)):
                if itemKey not in blockDict:
                    blockDict[itemKey] = []
                blockDict[itemKey].extend(itemVal)
            elif(isinstance(itemVal, dict)):
                if itemKey not in blockDict:
                    blockDict[itemKey] = {}
                blockDict[itemKey].update(itemVal)
                GatherOverrides(blockDict[itemKey])
            else:
                blockDict[itemKey] = itemVal

    # We should've accumulated the necessary values from override now.
    blockDict["Overrides"] = None