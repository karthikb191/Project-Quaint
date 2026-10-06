import sys
from .TargetView import TargetView
from PySide6 import QtCore, QtWidgets, QtGui

class BuildToolGUI:
    def __init__(self) -> None:
        self.ToolWindow : BuildToolWindow
        self.Application : QtWidgets.QApplication
        pass

    def Run(self):
        print("Running GUI Tool")
        print("Constructing QApplication")
        self.Application = QtWidgets.QApplication([])

        print("Building root window")
        self.BuildView()

        pass

    def BuildView(self):
        self.ToolWindow = BuildToolWindow()
        self.ToolWindow.resize(800, 600)
        self.ToolWindow.show()
        pass

class BuildToolWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.ClickCount = 0
        self.TestText = QtWidgets.QLabel("Clicked: 0", alignment=QtCore.Qt.AlignmentFlag.AlignCenter);
        self.TestButton = QtWidgets.QPushButton("TestButton")

        self.TargetView = TargetView()

        self.setCentralWidget(self.TargetView)
        
        DockWidget = QtWidgets.QDockWidget("Dock Widget", self)
        DockWidget.setAllowedAreas(QtCore.Qt.DockWidgetArea.LeftDockWidgetArea)
        DockWidget.setWidget(self.TargetView.TargetListWidget)
        self.addDockWidget(QtCore.Qt.DockWidgetArea.LeftDockWidgetArea, DockWidget);
        

        #Vertial root layout 
        # self.RootLayout = QtWidgets.QVBoxLayout(self);
        # self.RootLayout.addWidget(self.TestButton);
        # self.RootLayout.addWidget(self.TestText);

        # self.TestButton.clicked.connect(self.OnTestButtonClick)

    def OnTestButtonClick(self):
        self.ClickCount += 1
        self.TestText.setText(f"{self.ClickCount}")

BuildTool = BuildToolGUI()