# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Import Selected Feature From QGIS Dialog Model
# importSelectedFeatureFromQGISDialogModel.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class ImportSelectedFeatureFromQGISDialogModel, which implements the data model of the import
selected features from QGIS dialog.
"""
from PyQt5 import QtCore
from PyQt5.QtCore import QStringListModel, Qt, QModelIndex, pyqtSlot, QThread
from PyQt5.QtGui import QStandardItemModel, QStandardItem
from PyQt5.QtWidgets import QProgressDialog, QProgressBar
from qgis.core import QgsField, Qgis, QgsFeature, QgsFeatureIterator, QgsGeometry, QgsWkbTypes
from qgis._gui import QgisInterface

from secondodb.ui.io import qgisOutput
from secondodb.api import secondoapi
from secondodb.api.algebras import secondospatialalgebra as spatial
from timeit import default_timer as timer


class ImportFeaturesFromQGISDialogModel:
    """
    This class implements the data model of the import selected features from QGIS dialog.
    """

    signalProgress = QtCore.pyqtSignal(int)

    def __init__(self,
                 qgis_interface: QgisInterface,
                 secondo_connection: secondoapi.Connection,
                 only_selected: bool,
                 main_window):
        """
        Constructor of the class.

        :param qgis_interface: The QGIS-Interface object.
        :param secondo_connection: The |sec|-Connection object.
        :param only_selected: Set True, if only selected features of layer should be imported.
        """
        self.qgis_interface: QgisInterface = qgis_interface
        self.tableViewFeaturesModel = QStandardItemModel()
        self.listViewFieldSelectionModel = QStringListModel()
        self.lineEditGeometry: str = ''
        self.lineEditFeaturesCount = None
        self.lineEditLayer: str = ''
        self.lineEditRelationName: str = ''
        self.secondo_connection = secondo_connection
        self.only_selected = only_selected
        self.main_window = main_window
        # self.progress = QProgressBar()

        # Get active layer

        self.lineEditLayer = self.handle_get_active_layer_name()

        # Get geometry type

        self.lineEditGeometry = self.handle_get_geometry_type()
        self.lineEditWkbType = self.handle_get_wkb_type()

        # Get attribute definition

        self.fields = self.handle_get_fields()

        field_names = []
        for field in self.fields:
            field_names.append(field.name())

        self.tableViewFeaturesModel.setHorizontalHeaderLabels(field_names)
        self.listViewFieldSelectionModel.setStringList(field_names)

        # Get features
        if self.only_selected:
            self.selected_features: QgsFeatureIterator = self.handle_get_selected_features()
        else:
            self.selected_features: QgsFeatureIterator = self.handle_get_all_features()

        row_count = 0
        feature: QgsFeature
        for feature in self.selected_features:

            attr_values = feature.attributes()

            column_count = 0
            for column in attr_values:
                item = QStandardItem(str(column))
                self.tableViewFeaturesModel.setItem(row_count, column_count, item)
                column_count += 1

            row_count += 1

        # self.lineEditFeaturesCount = self.handle_get_selected_features_count()
        self.lineEditFeaturesCount = str(row_count)
        self.lineEditRelationName = self.lineEditLayer

    def handle_get_active_layer_name(self) -> str:
        """
        Handles the get active layer name action.

        :return: The name of the active layer.
        """
        return qgisOutput.get_active_layer_name(self.qgis_interface)

    def handle_get_geometry_type(self) -> str:
        """
        Handles the get geometry type action.

        :return: The name of the geometry type.
        """
        return qgisOutput.get_geometry_type_of_active_layer(self.qgis_interface)

    def handle_get_wkb_type(self) -> str:
        """
        Handles the get WKB type action.

        :return: The name of the WKB type.
        """
        return qgisOutput.get_wkb_type_of_active_layer(self.qgis_interface)

    def handle_get_fields(self) -> []:
        """
        Handles the get fields action.

        :return: A list with the fields of the active layer.
        """
        return qgisOutput.get_pending_fields_of_active_layer(self.qgis_interface)

    def handle_get_selected_features(self) -> []:
        """
        Handles the get selected features of the active layer action.

        :return: A list with the selected features of the active layer.
        """
        return qgisOutput.get_selected_features_of_active_layer(self.qgis_interface)

    def handle_get_all_features(self) -> []:
        """
        Handles the get selected features of the active layer action.

        :return: A list with the selected features of the active layer.
        """
        return qgisOutput.get_all_features_of_active_layer(self.qgis_interface)

    def handle_get_selected_features_count(self) -> int:
        """
        Handles the get selected features count action.

        :return: The quantity of the selected features as integer.
        """
        return qgisOutput.get_selected_features_count_of_active_layer(self.qgis_interface)

    def get_selected_field_names(self, selected_indexes: []) -> []:
        """
        Gets the selected field names in the dialog.

        :param selected_indexes: The selected indexes.
        :return: A list with the names of the selected fields.
        """
        field_names = []
        index: QModelIndex
        for index in selected_indexes:
            data = self.listViewFieldSelectionModel.data(index, Qt.DisplayRole)
            field_names.append(data)
        return field_names

    @staticmethod
    def get_selected_field_index(selected_indexes: []) -> []:
        """
        Gets the indexes in the model of the selected fields in the dialog.

        :param selected_indexes: The selected indexes.
        :return: A list with the field indexes.
        """
        field_indexes = []
        index: QModelIndex
        for index in selected_indexes:
            row_index = index.row()
            field_indexes.append(row_index)
        return field_indexes

    def get_selected_field_attributes(self, field_names: []) -> []:
        """
        Gets the attributes of the selected fields.

        :param field_names: A list with the field names.
        :return: A list with the field attributes.
        """
        field_attributes = []

        field: QgsField
        for field in self.fields:

            if field.name() in field_names:

                sec_field_name = field.name().capitalize()

                if field.typeName() == 'String':
                    sec_field_type = 'string'
                elif field.typeName() == 'Integer':
                    sec_field_type = 'int'
                else:
                    sec_field_type = 'string'

                field_attributes.append([sec_field_name, sec_field_type])

        return field_attributes

    def handle_import_to_secondo(self, selected_indexes: [], with_geometry: bool) -> None:
        """
        Handles the import to |sec| action.

        :param selected_indexes: A list with the selected indexes.
        :param with_geometry: Set true, if the geometry should be imported as well.
        :return: None
        """

        # Prepare geometry type and attribute definition for type construction

        field_names = self.get_selected_field_names(selected_indexes)
        field_attributes = self.get_selected_field_attributes(field_names)

        if self.lineEditGeometry is not None and self.lineEditWkbType is not None and with_geometry:
            geometry_type = self.lineEditGeometry.lower()
            if self.lineEditWkbType == 'MultiPoint':
                geometry_type = 'points'

            field_attributes.append(["GeoData", geometry_type])

        relation_name = self.lineEditRelationName.lower()

        # Prepare values (attributes and geometry)

        if self.only_selected:
            self.selected_features = self.handle_get_selected_features()
        else:
            self.selected_features = self.handle_get_all_features()

        field_indexes = self.get_selected_field_index(selected_indexes)

        tuples = []

        feature: QgsFeature
        for feature in self.selected_features:

            error_flag = False  # Set true if errors due to geometry support appear.

            attr_values = feature.attributes()

            relation_values = []
            relation_types = []

            attr_pos = 0
            column_count = 0
            for column in attr_values:

                # Get values only for selected fields

                if column_count in field_indexes:

                    data_type = field_attributes[attr_pos][1]
                    if data_type == 'string':
                        str_raw_value = str(column)

                        # Remove character " because it will collide with the Secondo syntax
                        str_after_replace = str_raw_value.replace('"', '')
                        str_value = '"' + str_after_replace + '"'

                        relation_types.append('string')
                    elif data_type == 'int':
                        str_value = str(column)
                        relation_types.append('int')
                    # elif data_type == 'double':
                    #     str_value = str(column)
                    #     relation_types.append('real')
                    else:
                        str_value = str(column)

                    relation_values.append(str_value)
                    attr_pos += 1

                column_count += 1

            if with_geometry:

                # Get the geometry of the feature

                geometry: QgsGeometry = feature.geometry()
                self.lineEditWkbType = QgsWkbTypes.displayString(geometry.wkbType())

                if self.lineEditGeometry == 'Point':

                    point = None
                    points = None
                    if self.lineEditWkbType == 'Point':
                        point = qgisOutput.convert_point_to_point(geometry.asPoint())
                    elif self.lineEditWkbType == 'PointZ':
                        point = qgisOutput.convert_point_to_point(geometry.asPoint())
                    elif self.lineEditWkbType == 'Point25D':
                        point = qgisOutput.convert_point_to_point(geometry.asPoint())
                    elif self.lineEditWkbType == 'MultiPoint':
                        points = qgisOutput.convert_multipoint_to_points(geometry.asMultiPoint())
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of WKB type " + self.lineEditWkbType + " is currently not supported.",
                            level=Qgis.Critical, duration=5)

                    if point is not None:
                        list_exp = spatial.convert_point_to_list_exp_str(point)
                        relation_values.append(list_exp)
                        relation_types.append('point')
                    elif points is not None:
                        list_exp = spatial.convert_points_to_list_exp_str(points)
                        relation_values.append(list_exp)
                        relation_types.append('points')
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of type line couldn't be created correctly.",
                            level=Qgis.Critical, duration=5)

                elif self.lineEditGeometry == 'Line':

                    line = None

                    if self.lineEditWkbType == 'LineString25D':
                        line = qgisOutput.convert_polyline_to_line(geometry.asPolyline())
                    elif self.lineEditWkbType == 'LineString':
                        line = qgisOutput.convert_polyline_to_line(geometry.asPolyline())
                    elif self.lineEditWkbType == 'LineStringZ':
                        line = qgisOutput.convert_polyline_to_line(geometry.asPolyline())
                    elif self.lineEditWkbType == 'MultiLineString':
                        line = qgisOutput.convert_multipolyline_to_line(geometry.asMultiPolyline())
                    elif self.lineEditWkbType == 'MultiLineStringZ':
                        line = qgisOutput.convert_multipolyline_to_line(geometry.asMultiPolyline())
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of WKB type " + self.lineEditWkbType + " is currently not supported.",
                            level=Qgis.Critical, duration=5)

                    if line is not None:
                        list_exp = spatial.convert_line_to_list_exp_str(line)
                        relation_values.append(list_exp)
                        relation_types.append('line')
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of type line couldn't be created correctly.",
                            level=Qgis.Critical, duration=5)

                elif self.lineEditGeometry == 'Region':

                    region = None

                    if self.lineEditWkbType == 'MultiPolygon':
                        region = qgisOutput.convert_multi_polygon_to_region(geometry.asMultiPolygon())
                    elif self.lineEditWkbType == 'Polygon':
                        region = qgisOutput.convert_polygon_to_region(geometry.asPolygon())
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of WKB type " + self.lineEditWkbType + " is currently not supported.",
                            level=Qgis.Critical, duration=5)

                    if region is not None:
                        list_exp = spatial.convert_region_to_list_exp_str(region)
                        relation_values.append(list_exp)
                        relation_types.append('region')
                    else:
                        error_flag = True
                        self.qgis_interface.messageBar().pushMessage(
                            "Error",
                            "Geometry of type region couldn't be created correctly.",
                            level=Qgis.Critical, duration=5)

                else:
                    error_flag = True
                    self.qgis_interface.messageBar().pushMessage(
                        "Error",
                        "Geometry type " + self.lineEditGeometry + " is currently not supported.",
                        level=Qgis.Critical, duration=5)

            # Append feature only if no errors

            if not error_flag:
                tuples.append([relation_values, relation_types])

        # Call Secondo-API method to create relation via let-command

        if len(tuples) > 0:
            self.main_window.handle_import_to_secondo(relation_name, field_attributes, tuples)








