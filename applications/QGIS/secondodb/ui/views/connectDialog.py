# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Connect Dialog
# connectDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class ConnectDialog, which implements the view of the connect dialog.
"""

from PyQt5.QtWidgets import QDialog
from secondodb.ui.views.widgets.connectDialogView import Ui_Dialog
from secondodb.ui.models import connectDialogModel


class ConnectDialog(QDialog):
    """
    This class implements the view of the connect dialog.
    """

    def __init__(self, main_window):
        """
        Constructor of the class

        :param main_window: The view object of the main window.
        """

        super().__init__()
        self.mainWindow = main_window
        self.mainWindowModel = main_window.MainWindowModel

        # Initialize Model

        self.model = connectDialogModel.ConnectDialogModel(self.mainWindowModel.parameters['hostname'],
                                                           self.mainWindowModel.parameters['port'])

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.hostText.setText(self.model.host)
        self.ui.portText.setText(self.model.port)

        self.ui.connectButton.clicked.connect(self.handle_connect_to_secondo)
        self.exec_()

    def handle_connect_to_secondo(self) -> None:
        """
        Handles the connect to |sec| action.

        :return: None
        """

        self.model.set_host(self.ui.hostText.text())
        self.model.set_port(self.ui.portText.text())

        if self.model.host is not None and self.model.port is not None:

            self.mainWindowModel.connect_to_secondo_server(self.model.host, self.model.port)
            self.close()

        else:
            pass

