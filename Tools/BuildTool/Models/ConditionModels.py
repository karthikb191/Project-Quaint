from pydantic import BaseModel, Field
from .Models import TargetSettingsSchema, PrebuiltTargetSettingsSchema

#TODO: These should be validated. Claude generated project used Optional type here
class ConditionSchema(BaseModel):
    platform: str | list[str] | None = None
    compiler: str | list[str] | None = None
    config : str | list[str] | None = None

# Variant inherits target settings schema to perform additional path collection and flags injection
class Variant(TargetSettingsSchema):
    condition : ConditionSchema = Field(default_factory=ConditionSchema)

class PrebuiltVariant(PrebuiltTargetSettingsSchema):
    condition : ConditionSchema = Field(default_factory=ConditionSchema)    