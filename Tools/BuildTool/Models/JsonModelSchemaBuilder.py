from typing import Any
from colorama import Fore
from Models import Models, ProjectTypes
import JsonTemplateParser as Parser
import Configs.Config as Config

AvailableTargets : dict[str, ProjectTypes.BuildTarget] = {}
TargetsToEvaluate : list[Models.TargetDependencySchema] = []


def BuildProjectSchema(projectJsonPath : str) -> ProjectTypes.BuildProject | None:
    print("-> Reading Project Schema: " + projectJsonPath)

    projectFullPath = Config.GetFullPath(projectJsonPath)
    tbl = Parser.ReadTemplateFile(projectFullPath)

    #Reading schema into the model object. The signature should match with the json key data
    schema = Models.ProjectSchema(**tbl)

    # Create a project object and link the first target
    project = ProjectTypes.BuildProject()
    project.Schema = schema

    target = __GatherTarget(schema.TargetPath)
    if target is None:
        print(Fore.RED + f"(!) Failed to parse project. Initial target provided failed to parse")
        return

    project.Target = target
    __GatherTargetDependencies(target)
    __LinkTargetsDependencies()

    print("-> Finished Reading Project Schema")
    return project

def __GatherTarget(targetPath: str) -> ProjectTypes.BuildTarget | None:
    print(Fore.CYAN + f"-> Reading Target Schema: {targetPath}")

    targetFullPath = Config.GetFullPath(targetPath)
    tbl = Parser.ReadTemplateFile(targetFullPath)

    if(len(tbl.items()) == 0):
        print(Fore.RED + f"(!) Following target failed to parse: {targetPath}")
        return

    schema = Models.TargetSchema(**tbl)

    target = ProjectTypes.BuildTarget()
    target.Schema = schema
    print(Fore.GREEN + f"-> Finished Reading Target: {targetPath}")

    return target

def __GatherTargetDependencies(initialTarget : ProjectTypes.BuildTarget):
    AvailableTargets[initialTarget.Name] = initialTarget
    targetDependencies = initialTarget.Schema.DependencyList.Targets
    if targetDependencies is None:
        print(Fore.YELLOW + "-> No Additional build targets found on root project.")
        return
    
    '''
        Loop through target dependencies and push them to the top of stack
        Fetch from top of stack, create new target schema
        Add the new target schema to the available list
        Fetch dependencies of this new target
        If dependency is already in the the Available target list, skip it
        Terminate once there are no dependencies left
    '''
    
    TargetsToEvaluate.extend(targetDependencies)

    while len(TargetsToEvaluate) > 0:
        targetDependency = TargetsToEvaluate.pop()

        # If this target is already processed, skip it
        if targetDependency.Name in AvailableTargets:
            continue

        target = __GatherTarget(targetDependency.Path)
        if(target is None):
            continue

        AvailableTargets[target.Name] = target
    pass

def __LinkTargetsDependencies():

    '''
        Iterate through AvailableTargets
        For each dependency defined, fetch it from the AvailableTargets and link to the current target
    '''
    print(Fore.CYAN + "-> Linking Target dependencies")

    for _, val in AvailableTargets.items():
        target = val
        deps = val.TargetDependencies
        if deps is None: continue

        for dep in deps:
            if dep.Name not in AvailableTargets:
                print(Fore.RED + f"(!) {dep.Name} has not been fetched as a dependency. Will not be added to target {target.Name}")
                continue

            target.Targets[dep.Name] = AvailableTargets[dep.Name]

    print(Fore.GREEN + "-> Finished Linking target dependencies")

def __GetValueOrNone(tbl : dict[str, Any] | None, key : str) -> Any:
    return None if tbl is None else \
            tbl[key] if key in tbl else None