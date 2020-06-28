# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Import Selected Feature From QGIS Dialog
# addDatabaseDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class ImportSelectedFeatureFromQGISDialog, which implements the view of the Import Selected
Feature From QGIS dialog.
"""
from PyQt5 import QtCore
from PyQt5.QtWidgets import QDialog

from qgis._core import Qgis
from qgis._gui import QgisInterface
import secondodb.api.secondoapi as api
from secondodb.ui.views.widgets.importSelectedFeatureFromQGISDialogView import Ui_Dialog
from secondodb.ui.models.importFeaturesFromQGISDialogModel import ImportFeaturesFromQGISDialogModel


class ImportFeaturesFromQGISDialog(QDialog):
    """
    This class implements the view of the Import Selected Feature From QGIS dialog.
    """

    def __init__(self, qgis_interface: QgisInterface,
                 secondo_connection: api.Connection,
                 only_selected: bool,
                 main_window):
        """
        Constructor of the class.

        :param qgis_interface: The QGIS-Interface object.
        :param secondo_connection: The |sec| connection object.
        """
        super().__init__()

        self.qgis_interface: QgisInterface = qgis_interface
        self.only_selected: bool = only_selected
        self.main_window = main_window

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)

        # Initialize model

        self.model = ImportFeaturesFromQGISDialogModel(qgis_interface, secondo_connection, self.only_selected,
                                                       self.main_window)

        # Initialize slots

        self.ui.pushButtonImport.clicked.connect(self.handle_import_to_secondo)
        self.ui.lineEditRelationName.textChanged.connect(self.handle_relation_name_changed)

        # self.model.signalProgress.connect(self.handle_progress)

        # Initialize ui values

        self.ui.lineEditLayer.setText(self.model.lineEditLayer)
        self.ui.lineEditGeometry.setText(self.model.lineEditGeometry)
        self.ui.lineEditWkbType.setText(self.model.lineEditWkbType)
        self.ui.lineEditFeaturesCount.setText(str(self.model.lineEditFeaturesCount))
        self.ui.lineEditRelationName.setText(self.model.lineEditRelationName)
        self.ui.tableViewFeatures.setModel(self.model.tableViewFeaturesModel)
        self.ui.listViewFieldSelection.setModel(self.model.listViewFieldSelectionModel)

        if self.only_selected:
            self.setWindowTitle("Import Selected Features from QGIS layer")
        else:
            self.setWindowTitle("Import All Features from QGIS layer")

        self.exec_()

    def handle_relation_name_changed(self) -> None:
        """
        Handles the changing of the relation name.

        :return: None
        """
        self.model.lineEditRelationName = self.ui.lineEditRelationName.text()

    def handle_import_to_secondo(self) -> None:
        """
        Handles the import to |sec| action.

        :return: None
        """

        # Get selected fields

        selected_indexes = self.ui.listViewFieldSelection.selectedIndexes()

        # Process import

        self.close()
        self.model.handle_import_to_secondo(selected_indexes,
                                            self.ui.checkBoxImportGeometry.isChecked())

