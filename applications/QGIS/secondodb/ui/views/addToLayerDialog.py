# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Add To Layer Dialog
# addToLayerDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AddToLayerDialog, which implements the view of the Add To Layer Dialog.
"""

from PyQt5.QtWidgets import QDialog
from qgis._gui import QgisInterface
from qgis.core import QgsProject, QgsVectorLayer

from secondodb.ui.io import qgisInput
from secondodb.ui.views.widgets.addToLayerDialogView import Ui_Dialog

from timeit import default_timer as timer


class AddToLayerDialog(QDialog):
    """
    This class implements the dialog for adding an spatial |sec| object to a new QGIS layer.
    """

    def __init__(self,
                 qgis_interface: QgisInterface,
                 object_name: str,
                 relation_tuples: [],
                 relation_fields: [],
                 geometry_type: str):
        """
        Constructor of the class.

        :param qgis_interface: The QGIS-Interface object.
        :param object_name: The name of the object.
        :param relation_tuples: The tuples of tne relation.
        :param relation_fields: The fields of the relation.
        :param geometry_type: The type of the geometry.
        """

        super().__init__()

        self.qgis_interface = qgis_interface
        self.object_name = object_name
        self.relation_tuples = relation_tuples
        self.relation_fields = relation_fields
        self.geometry_type = geometry_type

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.lineEditLayerName.setText(self.object_name)

        if self.geometry_type == 'line':
            self.ui.checkBoxAddAsPolyline.setEnabled(True)
        else:
            self.ui.checkBoxAddAsPolyline.setDisabled(True)

        # Initialize slots

        self.ui.pushButtonAdd.clicked.connect(self.handle_add_to_layer)

        # Execute view

        self.exec_()

    def handle_add_to_layer(self) -> None:
        """
        Handles the creation of the features and geometries to import the objects to QGIS.

        :return: None
        """

        # Add to new layer

        layer_name: str = self.ui.lineEditLayerName.text()

        # TODO: TIME FOR EXPERIMENTS

        start = timer()

        # print('Processing ' + layer_name)

        layer: QgsVectorLayer = qgisInput.create_new_vector_layer_for_relation(layer_name, self.relation_tuples,
                                                                               self.relation_fields, self.geometry_type)

        end = timer()
        delta = end - start
        # print('Layer created in: ' + str(delta))

        if layer is not None:

            # Add as a single polyline

            if self.ui.checkBoxAddAsPolyline.isChecked():
                layer = qgisInput.merge_multilines_of_layer(layer, self.qgis_interface)
                layer.setName(layer_name)

            QgsProject.instance().addMapLayers([layer])

        else:
            pass

        self.close()
