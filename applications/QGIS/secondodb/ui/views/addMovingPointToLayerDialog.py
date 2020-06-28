# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Add Moving Point To Layer Dialog
# addMovingPointToLayerDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AddMovingPointToLayerDialog, which implements the view of the Add Moving Point To Layer
Dialog.
"""

from collections import namedtuple
from PyQt5.QtWidgets import QDialog
from qgis._gui import QgisInterface
from qgis.core import QgsProject
from secondodb.ui.views.widgets.addMovingPointToLayerDialogView import Ui_Dialog
# from secondodb.ui.utilities import qgis_utilities
from secondodb.ui.io import qgisInput

from timeit import default_timer as timer


class AddMovingPointToLayerDialog(QDialog):
    """
    This class implements the view of the Add Moving Point To Layer Dialog.
    """

    def __init__(self,
                 qgis_interface: QgisInterface,
                 object_name: str,
                 relation_tuples: [],
                 relation_fields: [],
                 geometry_type: str,
                 frames_per_second: int):
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
        self.frames_per_second = frames_per_second

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.pushButtonAdd.clicked.connect(self.handle_add_to_layer)
        self.ui.lineEditLayerName.setText(object_name)
        self.exec_()

    def handle_add_to_layer(self) -> None:
        """
        Handles the add to layer action.

        :return: None
        """

        layer_name = self.ui.lineEditLayerName.text()

        if self.geometry_type == 'mpoint':

            # Add moving point as a trajectory line

            if self.ui.checkBoxAddAsTrajectoryLine.isChecked():

                headers = []
                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Name'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Start Time'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'End Time'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'X1'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Y1'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'X2'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Y2'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                layer_trajectory = \
                    qgisInput.create_layer_for_mpoint_as_linestring(layer_name=layer_name,
                                                                    relation_tuples=self.relation_tuples,
                                                                    relation_fields=headers)

                if layer_trajectory is not None:
                    QgsProject.instance().addMapLayers([layer_trajectory])
                else:
                    pass

            # Add moving point as a sequence of points to represent motion in QGIS

            if self.ui.checkBoxAddAsMovingPoint.isChecked():

                headers = []
                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Name'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Timestamp'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'X'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

                attribute.attribute_name = 'Y'
                attribute.attribute_type = 'string'

                headers.append(attribute)

                # TODO: TIME FOR EXPERIMENTS

                start = timer()

                # print('Processing ' + layer_name)

                layer_points = \
                        qgisInput.create_layer_for_mpoint_as_points_sequence(layer_name=layer_name,
                                                                             relation_tuples=self.relation_tuples,
                                                                             relation_fields=headers,
                                                                             frames_per_second=self.frames_per_second)

                end = timer()
                delta = end - start
                # print('Layer created in: ' + str(delta))

                if layer_points is not None:
                    QgsProject.instance().addMapLayers([layer_points])
                else:
                    pass

        self.close()
