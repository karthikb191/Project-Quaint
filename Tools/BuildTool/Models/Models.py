from __future__ import annotations
from dataclasses import dataclass
from pydantic import BaseModel, Field
from .ConditionModels import Variant, PrebuiltVariant

'''
This is my notes on how Pydantic works behind the scenes.
When a new object is created _init__ injects the values provided as raw data into the Model object
'''

@dataclass
class BaseTargetSchema():
    Name : str = ""
    Description : str = ""

@dataclass
class TargetDependencySchema(BaseTargetSchema):
    Path: str = ""

@dataclass
class TargetSettingsSchema():
    SourcePaths : list[str] | None = Field(default=[])
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    CompilerFlags : list[str] = Field(default=[])
    CompilerDefinitions : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])
    LinkerOptions : list[str] = Field(default=[])

@dataclass
class PrebuiltTargetSettingsSchema():
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])

@dataclass
class DependencySchema():
    Targets : list[TargetDependencySchema] | None = Field(default=None)
    StaticLibs : list[StaticLibSchema] | None = Field(default=None)
    DynamicLibs : list[DynamicLibSchema] | None = Field(default=None)
    #TODO: Add Header Only Library dependency

'''
    Idea is that the targets should represent CMake build targets
    Each target can specify any overrides based on the conditions in variants
'''
@dataclass
class TargetSchema(BaseTargetSchema, TargetSettingsSchema):
    Variants : list[Variant] | None = Field(default=None)
    Dependencies : DependencySchema = Field(default=DependencySchema())
    pass

@dataclass
class HeaderLibSchema(TargetSchema):
    HeaderPaths : list[str] = Field(default=[])
    CompilerFlags : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])
    pass

class ExternalTargetSchema(TargetSchema):
    pass

@dataclass
class BasePrebuiltTargetSchema(BaseTargetSchema, PrebuiltTargetSettingsSchema):
    Variants : list[PrebuiltVariant] | None = Field(default=None)

@dataclass
class StaticLibSchema(BasePrebuiltTargetSchema):
    FilePath : str = "" #Relative to the path variable

@dataclass
class DynamicLibSchema(BasePrebuiltTargetSchema):
    SourcePaths = None
    LibFilePath : str = "" #Relative to the path variable
    DllFilePath : str = "" #Relative to the path variable
    CopyDllToBuildPath : bool = False

'''
The Project schema contains the root project information.
Contains reference to the initial target schema to build.
'''
@dataclass
class ProjectSchema(BaseModel):
    Name: str = Field(default="N/A")
    Version: str = Field(default="1.0.0")
    TargetPath : str = Field(default="N/A") # The initial target to kick off the build process
    Variants : list[Variant] | None = Field(default=None)