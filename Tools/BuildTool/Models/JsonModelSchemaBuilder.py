import json
from typing import Any
from Models import Models, ProjectTypes
import JsonTemplateParser as Parser

AvailableTargets = {}
TargetsToEvaluate = {}

def GetValueOrNone(tbl : dict[str, Any] | None, key : str) -> Any:
    return None if tbl is None else \
            tbl[key] if key in tbl else None

def BuildProjectSchema(projectJsonPath) -> ProjectTypes.BuildProject:
    print("-> Reading Project Schema: " + projectJsonPath)

    tbl = Parser.ReadTemplateFile(projectJsonPath)

    #Reading schema into the model object. The signature should match with the json key data
    schema = Models.ProjectSchema(**tbl)

    # Create a project object and link the first target
    project = ProjectTypes.BuildProject()
    project.Schema = schema
    target = BuildTargetSchema(schema.TargetPath)
    project.Target = target

    print("-> Finished Reading Project Schema")
    return project

def BuildTargetSchema(targetJsonPath : str, level : int = 0) -> ProjectTypes.BuildTarget:

    print(f"{"->" * level} Reading Target Schema: " + targetJsonPath)

    tbl = Parser.ReadTemplateFile(targetJsonPath)
    schema = Models.TargetSchema(**tbl)

    ##TODO: If this target has already been processed, skip the next steps

    target = ProjectTypes.BuildTarget()
    target.Schema = schema

    print(f"{"->" * level} Finished Target Schema: " + targetJsonPath)
    return target