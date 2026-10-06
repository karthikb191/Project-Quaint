
class Args:
    Platform : str
    Config: str
    ProjectFilePath : str
    Initialized : bool

    @staticmethod
    def Init(platform : str, config: str, projectFilePath : str):
        #TODO: Validate against supported arguments

        Args.Platform = platform
        Args.Config = config
        Args.ProjectFilePath = projectFilePath
        Args.Initialized = True

