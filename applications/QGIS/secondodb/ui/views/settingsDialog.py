# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Settings Dialog
# settingsDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class SettingsDialog, which implements the view of the settings dialog.
"""

from PyQt5.QtWidgets import QDialog
from secondodb.ui.views.widgets.settingsView import Ui_Dialog
from secondodb.ui.models import settingsDialogModel


class SettingsDialog(QDialog):
    """
    This class implements the view of the settings dialog.
    """

    def __init__(self, main_window):
        """
        Constructor of the class.

        :param main_window: The Main Window View object.
        """

        super().__init__()
        self.mainWindow = main_window
        self.mainWindowModel = main_window.MainWindowModel

        # Initialize Model

        self.model = settingsDialogModel.SettingsDialogModel()

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)

        self.ui.lineEditHostname.setText(self.model.hostname)
        self.ui.lineEditPort.setText(self.model.port)
        # self.ui.lineEditMaxEntries.setText(self.model.maxEntries)
        self.ui.spinBoxFPS.setValue(int(self.model.frames_per_second))

        if self.model.loadTypes == 1:
            self.ui.checkBoxLoadTypes.setChecked(True)
        else:
            self.ui.checkBoxLoadTypes.setChecked(False)
        if self.model.loadAlgebras == 1:
            self.ui.checkBoxLoadAlgebras.setChecked(True)
        else:
            self.ui.checkBoxLoadAlgebras.setChecked(False)

        self.ui.pushButtonSave.clicked.connect(self.handle_save_parameters)
        self.exec_()

    def handle_save_parameters(self) -> None:
        """
        Handles the persistence of the settings.

        :return: None
        """
        self.model.hostname = self.ui.lineEditHostname.text()
        self.model.port = self.ui.lineEditPort.text()
        # self.model.maxEntries = self.ui.lineEditMaxEntries.text()

        self.model.frames_per_second = self.ui.spinBoxFPS.text()

        if self.ui.checkBoxLoadAlgebras.isChecked():
            self.model.loadAlgebras = 1
        else:
            self.model.loadAlgebras = 0

        if self.ui.checkBoxLoadTypes.isChecked():
            self.model.loadTypes = 1
        else:
            self.model.loadTypes = 0

        self.model.save_parameters()
        self.mainWindowModel.load_parameters()
        self.close()




