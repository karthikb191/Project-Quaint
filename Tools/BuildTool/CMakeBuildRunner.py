import Configs.Config as Config
import Models.JsonModelSchemaBuilder as SchemaBuilder
from CMake.CMakeBuilder import CMakeBuilder as Builder

SampleProjectPath = "Scripts/BuildTemplates/Projects/BoltProject.json"

def RunBuild():
    builder = Builder()
    project = SchemaBuilder.BuildProjectSchema(SampleProjectPath)
    builder.RunBuild(project)
    