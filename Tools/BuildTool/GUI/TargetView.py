from PySide6 import QtCore, QtWidgets, QtGui

class DummyTarget:
    def __init__(self, name:str) -> None:
        self.Name : str = name
        self.Targets : list[DummyTarget] = []
        pass


Parent = DummyTarget("parent")
Target1 = DummyTarget("target1")
Target2 = DummyTarget("target2")
Target3 = DummyTarget("target3")
Target4 = DummyTarget("target4")


TargetDict = {
    "parent":Parent,
    "target1":Target1,
    "target2":Target2,
    "target3":Target3,
    "target4":Target4,
}

Parent.Targets.append(Target1)
Parent.Targets.append(Target2)
Parent.Targets.append(Target3)
Parent.Targets.append(Target4)
Target1.Targets.append(Target2)
Target1.Targets.append(Target4)

Target2.Targets.append(Target1)
Target2.Targets.append(Target4)

class TargetView(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.TargetViewLayout = QtWidgets.QHBoxLayout(self)

        self.TargetListWidget = QtWidgets.QWidget(self)
        self.TargetDescWidget = QtWidgets.QWidget(self)

        self.TargetListLayout = QtWidgets.QVBoxLayout()
        self.TargetInfoLayout = QtWidgets.QVBoxLayout()

        self.TargetListLayout.setAlignment(QtCore.Qt.AlignmentFlag.AlignTop | QtCore.Qt.AlignmentFlag.AlignCenter)
        self.TestTarget1 = QtWidgets.QLabel("TestTarget1")
        self.TestTarget2 = QtWidgets.QLabel("TestTarget2")
        self.TestTarget3 = QtWidgets.QLabel("TestTarget3")
        self.TargetListLayout.addWidget(self.TestTarget1)
        self.TargetListLayout.addWidget(self.TestTarget2)
        self.TargetListLayout.addWidget(self.TestTarget3)

        self.TargetTreeWidget = QtWidgets.QTreeWidget(self, columnCount=1)

        self.RootTreeWidgetItem = QtWidgets.QTreeWidgetItem(["parent"])
        self.AddTargetsToTreeWidgetItem(self.RootTreeWidgetItem)
        self.TargetTreeWidget.insertTopLevelItems(0, [self.RootTreeWidgetItem])

        self.TargetTreeWidget.itemExpanded.connect(self.OnTargetItemExpanded)

        self.TargetListLayout.addWidget(self.TargetTreeWidget)

        self.TargetListWidget.setLayout(self.TargetListLayout)

        self.TestDesc1 = QtWidgets.QLabel("TestDesc1")
        self.TestDesc2 = QtWidgets.QLabel("TestDesc2")
        self.TestDesc3 = QtWidgets.QLabel("TestDesc3")
        self.TargetInfoLayout.addWidget(self.TestDesc1)
        self.TargetInfoLayout.addWidget(self.TestDesc2)
        self.TargetInfoLayout.addWidget(self.TestDesc3)
        self.TargetDescWidget.setLayout(self.TargetInfoLayout)

        
        #DockWidget.setLayout(self.TargetViewLayout)

        #self.TargetViewLayout.addLayout(self.TargetListLayout)
        self.TargetViewLayout.addWidget(self.TargetDescWidget)
        pass

    def AddTargetsToTreeWidgetItem(self, item: QtWidgets.QTreeWidgetItem | None):
        #Only insert if the target currently has no active children. Data wouldn't dynamically change
        if item == None or item.childCount() > 0:
            return
        
        #treeItem = QtWidgets.QTreeWidgetItem(["Teststetse"])
        #item.addChild(treeItem)

        key = item.text(0)
        if(key in TargetDict):
            dummyTarget = TargetDict[key]
            for val in dummyTarget.Targets:
                treeItem = QtWidgets.QTreeWidgetItem([val.Name])
                item.addChild(treeItem)

    def OnTargetItemExpanded(self, item: QtWidgets.QTreeWidgetItem):
        for i in range(item.childCount()):
            self.AddTargetsToTreeWidgetItem(item.child(i))

        self.TargetTreeWidget.expandItem(item)


    def SetupView():
        pass

class DisplayView(QtWidgets.QWidget):
    def SetupView(self):
        pass
