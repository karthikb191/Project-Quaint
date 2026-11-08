from __future__ import annotations
from dataclasses import dataclass
from pydantic import BaseModel, Field

'''
This is my notes on how Pydantic works behind the scenes.
When a new object is created _init__ injects the values provided as raw data into the Model object
'''

class BaseTargetSchema(BaseModel):
    Name : str = Field(default="")
    Description : str = Field(default="")
    Path: str = Field(default="")

class TargetDependencySchema(BaseTargetSchema):
    pass

class TargetSettingsSchema(BaseModel):
    SourcePaths : list[str] | None = Field(default=[])
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    CompilerFlags : list[str] = Field(default=[])
    CompilerDefinitions : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])
    LinkerOptions : list[str] = Field(default=[])

class PrebuiltTargetSettingsSchema(BaseModel):
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])

class DependencySchema(BaseModel):
    Targets : list[TargetDependencySchema] | None = Field(default=None)
    ExternalTargets : list[TargetSchema] | None = Field(default=None)
    StaticLibs : list[StaticLibSchema] | None = Field(default=None)
    DynamicLibs : list[DynamicLibSchema] | None = Field(default=None)
    #TODO: Add Header Only Library dependency

class ConditionSchema(BaseModel):
    platform: str | list[str] | None = Field(default=None)
    compiler: str | list[str] | None = Field(default=None)
    config : str | list[str] | None = Field(default=None)

# Variant inherits target settings schema to perform additional path collection and flags injection
class Variant(TargetSettingsSchema):
    condition : ConditionSchema = Field(default_factory=ConditionSchema)

class PrebuiltVariant(PrebuiltTargetSettingsSchema):
    condition : ConditionSchema = Field(default_factory=ConditionSchema)   

'''
    Idea is that the targets should represent CMake build targets
    Each target can specify any overrides based on the conditions in variants
'''
class TargetSchema(BaseTargetSchema, TargetSettingsSchema):
    Variants : list[Variant] | None = Field(default=None)
    #TODO: Rename this to Dependencies
    DependencyList : DependencySchema = Field(default_factory=DependencySchema)
    pass

class HeaderLibSchema(TargetSchema):
    HeaderPaths : list[str] = Field(default=[])
    CompilerFlags : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])
    pass

class ExternalTargetSchema(TargetSchema):
    pass

class BasePrebuiltTargetSchema(BaseTargetSchema, PrebuiltTargetSettingsSchema):
    Variants : list[PrebuiltVariant] | None = Field(default=None)

class StaticLibSchema(BasePrebuiltTargetSchema):
    FilePath : str = Field(default="") #Relative to the path variable

class DynamicLibSchema(BasePrebuiltTargetSchema):
    SourcePaths : list [str] = Field(default = [])
    LibFilePath : str = Field(default="") #Relative to the path variable
    DllFilePath : str = Field(default="") #Relative to the path variable
    CopyDllToBuildPath : bool = Field(default=False)

'''
The Project schema contains the root project information.
Contains reference to the initial target schema to build.
'''
class ProjectSchema(BaseModel):
    Name: str = Field(default="N/A")
    Version: str = Field(default="1.0.0")
    TargetPath : str = Field(default="N/A") # The initial target to kick off the build process