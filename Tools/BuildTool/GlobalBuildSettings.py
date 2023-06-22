from enum import Enum

class BuildSystem(Enum):
    CMAKE = 1

class GlobalBuildSettings:
    BuildTarget = ""
    BuildSystem = BuildSystem.CMAKE