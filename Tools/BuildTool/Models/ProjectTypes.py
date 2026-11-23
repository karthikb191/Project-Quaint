from __future__ import annotations
from enum import Enum
from Models import Models

class ETargetBuildType(Enum):
    EXECUTABLE = 0
    STATIC = 1
    DYNAMIC = 2

class BuildTarget:
    def __init__(self) -> None:
        self.Schema = Models.TargetSchema()
        self.Sources = []
        self.BuildType = ETargetBuildType.STATIC
        self.Targets = {}
        self.SubTargets = {}
        self.ExternalTargets = {}
        pass

    @property
    def Name(self):
        return self.Schema.Name;

    @property
    def StaticLibs(self) -> list[Models.StaticLibSchema] | None:
        return self.Schema.DependencyList.StaticLibs
    
    @property
    def DynamicLibs(self):
        return self.Schema.DependencyList.DynamicLibs

    @property
    def TargetDependencies(self):
        return self.Schema.DependencyList.Targets

    @property
    def ExternalTargetDependencies(self):
        return self.Schema.DependencyList.ExternalTargets

    Schema : Models.TargetSchema
    Targets : dict [str, BuildTarget]
    ExternalTargets : dict[str, BuildTarget]
    
    # Targets of sub-targets are ignored
    SubTargets : dict [str, BuildTarget]
    Sources : list[str]
    BuildType : ETargetBuildType
    
class BuildProject:
    def __init__(self) -> None:
        self.Schema = Models.ProjectSchema()
        pass

    @property
    def Name(self):
        return self.Schema.Name

    Schema : Models.ProjectSchema
    Target : BuildTarget
    AvailableTargets : dict[str, BuildTarget] = {}