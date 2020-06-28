# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# About Dialog
# aboutDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AboutDialog, which implements the view of the about dialog.
"""

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QDialog
from secondodb.ui.views.widgets.aboutView import Ui_Dialog


class AboutDialog(QDialog):
    """
    This class implements the view of the about dialog.
    """

    def __init__(self):
        """
        Constructor of the class.
        """

        super().__init__()

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.setWindowFlag(Qt.WindowContextHelpButtonHint, False)
        self.exec_()





