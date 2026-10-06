from pydantic import BaseModel, Field
from .ConditionModels import Variant, PrebuiltVariant

'''
This is my notes on how Pydantic works behind the scenes.
When a new object is created _init__ injects the values provided as raw data into the Model object
'''

class ProjectSchema(BaseModel):
    Name: str = Field(default="N/A")
    TargetPath : str = Field(default="N/A") # The initial target to kick off the build process
    Variants : list[Variant] | None = Field(default=None) # 

class BaseTargetSchema():
    Name : str
    Description : str

class TargetSettingsSchema():
    SourcePaths : list[str] | None = Field(default=[])
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    CompilerFlags : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])

class PrebuiltTargetSettingsSchema():
    HeaderPaths : list[str] = Field(default=[])
    ExcludePaths : list[str] = Field(default=[])
    PreprocessorDefines : list[str] = Field(default=[])

'''
    Idea is that the targets should represent CMake build targets
    Each target can specify any overrides based on the conditions in variants
'''
class TargetSchema(BaseTargetSchema, TargetSettingsSchema):
    Variants : list[Variant] | None = Field(default=None)
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
    FilePath : str #Relative to the path variable

class DynamicLibSchema(BasePrebuiltTargetSchema):
    SourcePaths = None
    LibFilePath : str #Relative to the path variable
    DllFilePath : str #Relative to the path variable
    CopyDllToBuildPath : bool = False
