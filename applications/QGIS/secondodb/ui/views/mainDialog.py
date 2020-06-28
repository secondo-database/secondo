# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Main Dialog
# mainDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class ImportObjectFromQGISDialog, which implements a dialog wrapper to handle the main window
widget of the SecondoDB plugin.
"""
from qgis._gui import QgisInterface
from secondodb.ui.views.widgets.mainDialogView import Ui_Dialog
from secondodb.ui.views.mainWindow import MainWindow
from PyQt5 import QtCore
from PyQt5.QtWidgets import QDialog


class MainDialog(QDialog):
    """
    This class implements a dialog wrapper to handle the main window widget of the SecondoDB plugin.
    """

    # noinspection PyTypeChecker
    def __init__(self, qgis_interface: QgisInterface):
        """
        Constructor of the class.

        :param qgis_interface: The QGIS-Interface object.
        """

        # Initialize the dialog wrapper

        super().__init__()
        # self.MainDialog = QtWidgets.QDialog()
        self.uiDialog = Ui_Dialog()
        self.uiDialog.setupUi(self)

        # Initialize the main window controller and add widget to dialog wrapper

        self.mainWindow = MainWindow(qgis_interface=qgis_interface, main_dialog=self)
        self.uiDialog.verticalLayout.addWidget(self.mainWindow)

        # Close the main dialog wrapper when exit in menu is triggered

        self.mainWindow.ui.actionExit.triggered.connect(self.close)

        # Show minimize/maximize buttons

        self.setWindowFlags(self.windowFlags() | QtCore.Qt.WindowSystemMenuHint | QtCore.Qt.WindowMinMaxButtonsHint)

