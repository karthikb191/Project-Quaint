from typing import Any
from colorama import Fore
from Models import Models, ProjectTypes
import JsonTemplateParser as Parser
import Configs.Config as Config

TargetsToEvaluate : list[Models.TargetDependencySchema] = []
ExternalTargetsToEvaluate : list[Models.TargetSchema] = []
AvailableTargets : dict[str, ProjectTypes.BuildTarget] = {}

def BuildProjectSchema(projectJsonPath : str) -> ProjectTypes.BuildProject:
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
        return project

    project.Target = target
    project.Target.BuildType = ProjectTypes.ETargetBuildType.EXECUTABLE
    if target.Schema.DependencyList.Targets is not None:
        TargetsToEvaluate.extend(target.Schema.DependencyList.Targets)
    if target.Schema.DependencyList.ExternalTargets is not None:
        ExternalTargetsToEvaluate.extend(target.Schema.DependencyList.ExternalTargets)

    __GatherTargetDependencies(target)
    __LinkTargetsDependencies()
    project.AvailableTargets = AvailableTargets

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
    target = __GatherFromTargetSchema(schema)
    if(target is not None):
        print(Fore.GREEN + f"-> Finished Reading Target: {targetPath}")
    
    return target

def __GatherFromTargetSchema(schema: Models.TargetSchema) -> ProjectTypes.BuildTarget | None:
    if(schema is None): 
        print(Fore.RED, "Invalid target schema encountered")
        return
    
    target = ProjectTypes.BuildTarget()
    target.Schema = schema

    #Gather sub-targets for this target    
    if schema.DependencyList.SubTargets is not None:
        for subTarget in schema.DependencyList.SubTargets:
            st = __GatherTarget(subTarget.Path)
            if st is None: continue
            target.SubTargets[st.Name] = st
            pass

    return target

def __GatherTargetDependencies(initialTarget : ProjectTypes.BuildTarget):
    AvailableTargets[initialTarget.Name] = initialTarget
    if TargetsToEvaluate is None:
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

    while len(TargetsToEvaluate) > 0:
        targetDependency = TargetsToEvaluate.pop()

        # If this target is already processed, skip it
        if targetDependency.Name in AvailableTargets:
            continue

        target = __GatherTarget(targetDependency.Path)
        if(target is None):
            continue

        #TODO: Clean this up. Not very optimal
        schema = target.Schema
        if schema.DependencyList.Targets is not None:
            for depTarget in schema.DependencyList.Targets:
                if depTarget.Name not in AvailableTargets:
                    TargetsToEvaluate.append(depTarget)

        if schema.DependencyList.ExternalTargets is not None:
            for extTarget in schema.DependencyList.ExternalTargets:
                if extTarget.Name not in AvailableTargets:
                    ExternalTargetsToEvaluate.append(extTarget)

        AvailableTargets[target.Name] = target

    while len(ExternalTargetsToEvaluate) > 0:
        targetDependency = ExternalTargetsToEvaluate.pop()
        
        # If this target is already processed, skip it
        if targetDependency.Name in AvailableTargets:
            continue

        target = __GatherFromTargetSchema(targetDependency)
        if(target is None):
            continue

        #TODO: Clean this up. Not very optimal
        schema = target.Schema
        if schema.DependencyList.Targets is not None:
            for depTarget in schema.DependencyList.Targets:
                if depTarget.Name not in AvailableTargets:
                    TargetsToEvaluate.append(depTarget)

        if schema.DependencyList.ExternalTargets is not None:
            for extTarget in schema.DependencyList.ExternalTargets:
                if extTarget.Name not in AvailableTargets:
                    ExternalTargetsToEvaluate.append(extTarget)

        AvailableTargets[target.Name] = target


def __LinkTargetsDependencies():

    '''
        Iterate through AvailableTargets
        For each dependency defined, fetch it from the AvailableTargets and link to the current target
    '''
    print(Fore.CYAN + "-> Linking Target dependencies")

    for _, val in AvailableTargets.items():
        target = val
        deps = val.TargetDependencies
        extDeps = val.ExternalTargetDependencies

        if deps is not None:
            for dep in deps:
                if dep.Name not in AvailableTargets:
                    print(Fore.RED + f"(!) {dep.Name} has not been fetched as a dependency. Will not be added to target {target.Name}")
                    continue
                target.Targets[dep.Name] = AvailableTargets[dep.Name]

        if extDeps is not None:
            for dep in extDeps:
                if dep.Name not in AvailableTargets:
                    print(Fore.RED + f"(!) {dep.Name} has not been fetched as a dependency. Will not be added to target {target.Name}")
                    continue
                target.ExternalTargets[dep.Name] = AvailableTargets[dep.Name]

    print(Fore.GREEN + "-> Finished Linking target dependencies")

def __GetValueOrNone(tbl : dict[str, Any] | None, key : str) -> Any:
    return None if tbl is None else \
            tbl[key] if key in tbl else None