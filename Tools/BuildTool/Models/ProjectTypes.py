from __future__ import annotations
from Models import Models

class BuildTarget:
    def __init__(self) -> None:
        self.Schema = Models.TargetSchema()
        pass

    @property
    def StaticLibs(self):
        return self.Schema.Dependencies.StaticLibs
    
    @property
    def DynamicLibs(self):
        return self.Schema.Dependencies.DynamicLibs

    @property
    def TargetDependencies(self):
        return self.Schema.Dependencies.Targets

    Schema : Models.TargetSchema
    DependentTargets : dict [str, BuildTarget]
    
class BuildProject:
    def __init__(self) -> None:
        self.Schema = Models.ProjectSchema()
        pass

    Schema : Models.ProjectSchema
    Target : BuildTarget