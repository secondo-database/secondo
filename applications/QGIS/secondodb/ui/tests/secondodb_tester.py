from secondodb.ui.views.mainWindow import MainWindow
from PyQt5 import QtWidgets

# import secondodb.ui.resources.resources_rc
# pyuic5 -o mainWindowView.py mainWindowView.ui
# pyrcc5 resources.qrc -o resources_rc.py

if __name__ == "__main__":
    import sys

    app = QtWidgets.QApplication(sys.argv)

    mainWindow = MainWindow()
    mainWindow.show()

    sys.exit(app.exec_())
