from __future__ import annotations
from Models import Models

class BuildTarget:
    def __init__(self) -> None:
        self.Schema = Models.TargetSchema()
        self.Targets = {}
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

    Schema : Models.TargetSchema
    Targets : dict [str, BuildTarget]
    
class BuildProject:
    def __init__(self) -> None:
        self.Schema = Models.ProjectSchema()
        pass

    @property
    def Name(self):
        return self.Schema.Name

    Schema : Models.ProjectSchema
    Target : BuildTarget