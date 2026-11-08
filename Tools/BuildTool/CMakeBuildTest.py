import Configs.Config as Config
import Models.JsonModelSchemaBuilder as SchemaBuilder

SampleProjectPath = "Scripts/BuildTemplates/Projects/BoltProject.json"

def RunBuild():
    SchemaBuilder.BuildProjectSchema(SampleProjectPath)