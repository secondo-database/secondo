# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Add Database Dialog
# addDatabaseDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AddDatabaseDialog, which implements the view of the add database dialog.
"""

from PyQt5.QtWidgets import QDialog
from secondodb.ui.views.widgets.addDatabaseDialogView import Ui_Dialog
from secondodb.ui.models.addDatabaseDialogModel import AddDatabaseDialogModel


class AddDatabaseDialog(QDialog):
    """
    This class implements the view of the add database dialog.
    """

    def __init__(self, main_window):
        """
        Constructor of the class.

        :param main_window: The view object of the main window.
        """
        super().__init__()
        self.mainWindowView = main_window
        self.mainWindowModel = main_window.MainWindowModel

        # Initialize Model

        self.model = AddDatabaseDialogModel()

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)

        self.ui.addButton.clicked.connect(self.handle_add_database)
        self.exec_()

    def handle_add_database(self) -> None:
        """
        Handles the actions after clicking the button add database on the toolbar.

        :return: None
        """

        self.model.set_database_name(self.ui.databaseNameText.text())

        if self.model.get_database_name() is not None:

            self.mainWindowModel.add_database(self.model.get_database_name())
            self.close()





