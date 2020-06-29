# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Main Window Model
# mainWindowModel.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class MainWindowModel, which implements the data model of the main window of the application.
"""

from PyQt5.QtCore import pyqtSlot, QThread, Qt, QModelIndex
from PyQt5.QtGui import QIcon, QStandardItem, QPixmap, QStandardItemModel
from PyQt5 import QtCore
from qgis._core import QgsSettings

import secondodb.ui.resources.resources_rc
import secondodb.api.secondoapi as api
from collections import namedtuple
from datetime import datetime

from timeit import default_timer as timer

SUPPORTED_OBJECTS = ['point', 'points', 'line', 'region', 'rel', 'mpoint', 'mregion']
SPATIAL_OBJECTS = ['point', 'points', 'line', 'region']
SPATIO_TEMPORAL_OBJECTS = ['mpoint', 'mregion']
NO_ALGEBRA = True
NO_TYPE_CONSTRUCTORS = True

ICON_FOLDER = QIcon()
ICON_LINEOBJECT = QIcon()
ICON_POINTOBJECT = QIcon()
ICON_MPOINTOBJECT = QIcon()
ICON_POINTSOBJECT = QIcon()
ICON_MAPOBJECT = QIcon()
ICON_REGIONOBJECT = QIcon()
ICON_GRAPHOBJECT = QIcon()
ICON_RELOBJECT = QIcon()
ICON_INSTANTOBJECT = QIcon()
ICON_STRINGOBJECT = QIcon()
ICON_REALOBJECT = QIcon()
ICON_EDGEOBJECT = QIcon()
ICON_ARRAYOBJECT = QIcon()
ICON_OTHEROBJECT = QIcon()
ICON_DB = QIcon()
ICON_ALGEBRA = QIcon()
ICON_TYPE = QIcon()
ICON_OPERATOR = QIcon()
ICON_DB_OPEN = QIcon()


def initialize_icons() -> None:
    """
    Initializes the icons to be displayed in the main window.

    :return: None
    """

    ICON_FOLDER.addPixmap(QPixmap(":/icons/folder.png"), QIcon.Normal, QIcon.Off)
    ICON_LINEOBJECT.addPixmap(QPixmap(":/icons/lineobject.png"), QIcon.Normal, QIcon.Off)
    ICON_POINTOBJECT.addPixmap(QPixmap(":/icons/objectpoint.png"), QIcon.Normal, QIcon.Off)
    ICON_MPOINTOBJECT.addPixmap(QPixmap(":/icons/objectmpoint.png"), QIcon.Normal, QIcon.Off)
    ICON_POINTSOBJECT.addPixmap(QPixmap(":/icons/objectpoints.png"), QIcon.Normal, QIcon.Off)
    ICON_MAPOBJECT.addPixmap(QPixmap(":/icons/objectmap.png"), QIcon.Normal, QIcon.Off)
    ICON_REGIONOBJECT.addPixmap(QPixmap(":/icons/objectregion.png"), QIcon.Normal, QIcon.Off)
    ICON_GRAPHOBJECT.addPixmap(QPixmap(":/icons/objectgraph.png"), QIcon.Normal, QIcon.Off)
    ICON_RELOBJECT.addPixmap(QPixmap(":/icons/objectrel.png"), QIcon.Normal, QIcon.Off)
    ICON_INSTANTOBJECT.addPixmap(QPixmap(":/icons/objectinstant.png"), QIcon.Normal, QIcon.Off)
    ICON_STRINGOBJECT.addPixmap(QPixmap(":/icons/objectstring.png"), QIcon.Normal, QIcon.Off)
    ICON_REALOBJECT.addPixmap(QPixmap(":/icons/objectreal.png"), QIcon.Normal, QIcon.Off)
    ICON_EDGEOBJECT.addPixmap(QPixmap(":/icons/objectedge.png"), QIcon.Normal, QIcon.Off)
    ICON_ARRAYOBJECT.addPixmap(QPixmap(":/icons/objectarray.png"), QIcon.Normal, QIcon.Off)
    ICON_OTHEROBJECT.addPixmap(QPixmap(":/icons/objectother.png"), QIcon.Normal, QIcon.Off)
    ICON_DB.addPixmap(QPixmap(":/icons/database.png"), QIcon.Normal, QIcon.Off)
    ICON_ALGEBRA.addPixmap(QPixmap(":/icons/algebra.png"), QIcon.Normal, QIcon.Off)
    ICON_TYPE.addPixmap(QPixmap(":/icons/datatype.png"), QIcon.Normal, QIcon.Off)
    ICON_OPERATOR.addPixmap(QPixmap(":/icons/operator.png"), QIcon.Normal, QIcon.Off)
    ICON_DB_OPEN.addPixmap(QPixmap(":/icons/databaseopen.png"), QIcon.Normal, QIcon.Off)


def get_icon_for_object(object_type: str) -> QIcon:
    """
    Returns an icon object for a given type.

    :param object_type: The object type.
    :return: An icon object.
    """

    if object_type == 'folder':
        return ICON_FOLDER
    elif object_type == 'line':
        return ICON_LINEOBJECT
    elif object_type == 'region':
        return ICON_REGIONOBJECT
    elif object_type == 'mregion':
        return ICON_REGIONOBJECT
    elif object_type == 'point':
        return ICON_POINTOBJECT
    elif object_type == 'points':
        return ICON_POINTSOBJECT
    elif object_type == 'mpoint':
        return ICON_MPOINTOBJECT
    elif object_type == 'graph':
        return ICON_GRAPHOBJECT
    elif object_type == 'map':
        return ICON_MAPOBJECT
    elif object_type == 'rel':
        return ICON_RELOBJECT
    elif object_type == 'trel':
        return ICON_RELOBJECT
    elif object_type == 'orel':
        return ICON_RELOBJECT
    elif object_type == 'instant':
        return ICON_INSTANTOBJECT
    elif object_type == 'mint':
        return ICON_REALOBJECT
    elif object_type == 'int':
        return ICON_REALOBJECT
    elif object_type == 'mreal':
        return ICON_REALOBJECT
    elif object_type == 'mstring':
        return ICON_STRINGOBJECT
    elif object_type == 'string':
        return ICON_STRINGOBJECT
    elif object_type == 'edge':
        return ICON_EDGEOBJECT
    elif object_type == 'array':
        return ICON_ARRAYOBJECT
    elif object_type == 'algebra':
        return ICON_ALGEBRA
    elif object_type == 'operator':
        return ICON_OPERATOR
    elif object_type == 'type':
        return ICON_TYPE
    elif object_type == 'database':
        return ICON_DB
    elif object_type == 'dbopen':
        return ICON_DB_OPEN
    else:
        return ICON_OTHEROBJECT


def create_data_container(data_type: str, data_object) -> object:
    """
    Creates a named tuple as a data container for the nodes of the navigation tree.

    :param data_type: The type of the data (type, database, algebra, etc.)
    :param data_object: The object containing the data of the node.
    :return: The data container.
    """
    data_container = namedtuple('datacontainer', ['data_type', 'data_object'])
    data_container.data_type = data_type
    data_container.data_object = data_object
    return data_container


def geometry_pretty_printer(geometry_type: str, geometry_value) -> str:
    """
    Create pretty printed version of a geometry to be displayed in a control.

    :param geometry_type: The type of the geometry.
    :param geometry_value: The geometry object.
    :return: The string with the pretty printed version of the geometry.
    """

    if geometry_type == 'point':
        return "(" + str(geometry_value.x) + ", " + str(geometry_value.y) + ")"
    elif geometry_type == 'points':
        quantity = 0
        return "Points " + "(" + str(quantity) + ")"
    elif geometry_type == 'line':
        quantity = len(geometry_value.segments)
        return "Line " + "(" + str(quantity) + " segments)"
    elif geometry_type == 'region':
        quantity_faces = len(geometry_value.faces)
        return "Region " + "(" + str(quantity_faces) + " faces)"
    elif geometry_type == 'mpoint':
        return "MPoint"
    elif geometry_type == 'mregion':
        return "MRegion"


def create_model_for_point_object(point_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a point object.

    :param point_object: A point object.
    :return: The model.
    """

    model = QStandardItemModel()

    root_node = model.invisibleRootItem()
    root_node.setData(point_object, Qt.UserRole)

    model.setHorizontalHeaderLabels(['X', 'Y'])
    point_item_x = QStandardItem(str(point_object.x))
    point_item_y = QStandardItem(str(point_object.y))

    root_node.appendRow([point_item_x, point_item_y])

    return model


def create_model_for_points_object(points_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a points object.

    :param points_object: The points object.
    :return: The model.
    """

    model = QStandardItemModel()
    root_node = model.invisibleRootItem()
    root_node.setData(points_object, Qt.UserRole)

    model.setHorizontalHeaderLabels(['X', 'Y'])

    for point in points_object:
        point_item_x = QStandardItem(str(point.x))
        point_item_y = QStandardItem(str(point.y))
        root_node.appendRow([point_item_x, point_item_y])

    return model


def create_model_for_line_object(line_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a line object.

    :param line_object: A line object.
    :return: The model.
    """

    model = QStandardItemModel()
    root_node = model.invisibleRootItem()
    root_node.setData(line_object, Qt.UserRole)

    model.setHorizontalHeaderLabels(['X1', 'Y1', 'X2', 'Y2'])

    for segment in line_object.segments:
        point_item_x1 = QStandardItem(str(segment.x1))
        point_item_y1 = QStandardItem(str(segment.y1))
        point_item_x2 = QStandardItem(str(segment.x2))
        point_item_y2 = QStandardItem(str(segment.y2))
        root_node.appendRow([point_item_x1, point_item_y1, point_item_x2, point_item_y2])
        # model.setItem(counter, 0, point_item_x1)
        # model.setItem(counter, 1, point_item_y1)
        # model.setItem(counter, 2, point_item_x2)
        # model.setItem(counter, 3, point_item_y2)

    return model


def create_model_for_region_object(region_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a region object.

    :param region_object: A region object.
    :return: The model.
    """

    model = QStandardItemModel()

    root_node = model.invisibleRootItem()
    root_node.setData(region_object, Qt.UserRole)

    model.setHorizontalHeaderLabels(['Face/Cycle', 'X', 'Y'])

    face_counter = 0
    for face in region_object.faces:

        face_item = QStandardItem('Face ' + str(face_counter + 1))

        outercycle_item = QStandardItem('Outer cycle')

        point_counter = 0
        for point in face.outercycle:
            point_x = QStandardItem(str(point.x))
            point_y = QStandardItem(str(point.y))
            outercycle_item.setChild(point_counter, 1, point_x)
            outercycle_item.setChild(point_counter, 2, point_y)
            point_counter += 1
        face_item.setChild(0, 0, outercycle_item)

        if len(face.holecycles) > 0:

            for holecycle in face.holecycles:

                holecycle_item = QStandardItem('Hole cycle')

                point_counter = 0
                for point in holecycle:
                    point_x = QStandardItem(str(point.x))
                    point_y = QStandardItem(str(point.y))
                    holecycle_item.setChild(point_counter, 1, point_x)
                    holecycle_item.setChild(point_counter, 2, point_y)
                    point_counter += 1
                face_item.setChild(1, 0, holecycle_item)

        model.setItem(face_counter, 0, face_item)
        face_counter += 1

    return model


def create_model_for_mpoint_object(mpoint_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a mpoint object.

    :param mpoint_object: A mpoint object.
    :return: The model.
    """

    model = QStandardItemModel()

    root_node = model.invisibleRootItem()
    root_node.setData(mpoint_object, Qt.UserRole)

    # Set header items for start and end time

    column_start_time_item = QStandardItem()
    column_start_time_item.setData(mpoint_object.intervals[0].interval.start_time)
    model.setHorizontalHeaderItem(0, column_start_time_item)

    column_end_time_item = QStandardItem()
    column_end_time_item.setData(mpoint_object.intervals[len(mpoint_object.intervals) - 1].interval.end_time)
    model.setHorizontalHeaderItem(1, column_end_time_item)

    # Set header labels

    model.setHorizontalHeaderLabels(['Start time', 'End time', 'Close left', 'Close right',
                                     'X1', 'Y1', 'X2', 'Y2'])

    # Iterate over intervals and add rows to model

    counter = 0
    for point_in_interval in mpoint_object.intervals:
        start_time = QStandardItem(datetime.strftime(point_in_interval.interval.start_time,
                                                     '%Y-%m-%d %H:%M:%S.%f')[:-3])
        end_time = QStandardItem(datetime.strftime(point_in_interval.interval.end_time,
                                                   '%Y-%m-%d %H:%M:%S.%f')[:-3])

        close_left = QStandardItem(str(point_in_interval.interval.close_left))
        close_right = QStandardItem(str(point_in_interval.interval.close_right))

        x1 = QStandardItem(str(point_in_interval.motion_vector.x1))
        y1 = QStandardItem(str(point_in_interval.motion_vector.y1))
        x2 = QStandardItem(str(point_in_interval.motion_vector.x2))
        y2 = QStandardItem(str(point_in_interval.motion_vector.y2))

        close_left.setTextAlignment(Qt.AlignCenter)
        close_right.setTextAlignment(Qt.AlignCenter)
        x1.setTextAlignment(Qt.AlignCenter)
        x2.setTextAlignment(Qt.AlignCenter)
        y1.setTextAlignment(Qt.AlignCenter)
        y2.setTextAlignment(Qt.AlignCenter)

        model.setItem(counter, 0, start_time)
        model.setItem(counter, 1, end_time)
        model.setItem(counter, 2, close_left)
        model.setItem(counter, 3, close_right)
        model.setItem(counter, 4, x1)
        model.setItem(counter, 5, y1)
        model.setItem(counter, 6, x2)
        model.setItem(counter, 7, y2)

        counter += 1

    return model


def create_model_for_mregion_object(mregion_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a mregion object.

    :param mregion_object: A mregion object.
    :return: The model.
    """
    # TODO: DO IT

    model = QStandardItemModel()

    root_node = model.invisibleRootItem()
    root_node.setData(mregion_object, Qt.UserRole)

    # Set header items for start and end time

    column_start_time_item = QStandardItem()
    column_start_time_item.setData(mregion_object.intervals[0].interval.start_time)
    model.setHorizontalHeaderItem(0, column_start_time_item)

    column_end_time_item = QStandardItem()
    column_end_time_item.setData(mregion_object.intervals[len(mregion_object.intervals) - 1].interval.end_time)
    model.setHorizontalHeaderItem(1, column_end_time_item)

    # Set header labels

    model.setHorizontalHeaderLabels(['Start time', 'End time', 'Close left', 'Close right',
                                     'Geometry'])

    # Iterate over intervals and add rows to model

    counter = 0
    for region_in_interval in mregion_object.intervals:
        start_time = QStandardItem(datetime.strftime(region_in_interval.interval.start_time,
                                                     '%Y-%m-%d %H:%M:%S.%f')[:-3])
        end_time = QStandardItem(datetime.strftime(region_in_interval.interval.end_time,
                                                   '%Y-%m-%d %H:%M:%S.%f')[:-3])

        close_left = QStandardItem(str(region_in_interval.interval.close_left))
        close_right = QStandardItem(str(region_in_interval.interval.close_right))

        geometry = QStandardItem('Region (' + str(len(region_in_interval.map_faces)) + ' faces)')
        geometry.setIcon(get_icon_for_object('region'))
        geometry.setData(region_in_interval, Qt.UserRole)

        close_left.setTextAlignment(Qt.AlignCenter)
        close_right.setTextAlignment(Qt.AlignCenter)
        geometry.setTextAlignment(Qt.AlignCenter)

        model.setItem(counter, 0, start_time)
        model.setItem(counter, 1, end_time)
        model.setItem(counter, 2, close_left)
        model.setItem(counter, 3, close_right)
        model.setItem(counter, 4, geometry)

        counter += 1

    return model


def create_model_for_rel_object(relation_object) -> QStandardItemModel:
    """
    Creates a StandardItemModel for a relation object.

    :param relation_object: A relation object.
    :return: The model.
    """

    model = QStandardItemModel()
    root_node = model.invisibleRootItem()
    root_node.setData(relation_object, Qt.UserRole)

    # Set at horizontal header level the labels of the fields and the type definitions as header items

    field_labels = []
    column_counter = 0
    for attribute in relation_object.attributes:
        field_labels.append(attribute.attribute_name)
        column_item = QStandardItem()
        column_item.setData(attribute)
        model.setHorizontalHeaderItem(column_counter, column_item)
        column_counter += 1

    model.setHorizontalHeaderLabels(field_labels)

    row_counter = 0
    for row in relation_object.data:
        column_counter = 0
        for field_name, field_value in row.items():

            single_row = QStandardItem()

            # Check type of the value

            column_type = relation_object.attributes[column_counter].attribute_type

            # IMPORTANT: Geometry objects (points, lines, regions) have their own data object in the model. For
            # display purposes in the table view, the main node has a "pretty printer" version of the object
            # (like (122, 352) for a point), which will be displayed through the DisplayRole of the item. The
            # actual geometry object is stored in the same item using the role UserRole. The geometry object is
            # stored as a named tuple, which can be called later using their attributes.

            if column_type in SPATIAL_OBJECTS:
                geo_as_str = geometry_pretty_printer(column_type, field_value)
                single_row.setIcon(get_icon_for_object(column_type))
                single_row.setData(field_value, Qt.UserRole)
                single_row.setText(geo_as_str)
            if column_type in SPATIO_TEMPORAL_OBJECTS:
                geo_as_str = geometry_pretty_printer(column_type, field_value)
                single_row.setIcon(get_icon_for_object(column_type))
                single_row.setData(field_value, Qt.UserRole)
                single_row.setText(geo_as_str)
            else:
                single_row.setText(str(field_value))

            model.setItem(row_counter, column_counter, single_row)
            column_counter += 1

        row_counter += 1

    return model


class ConnectionThread(QThread):
    """
    This class implements a thread to establish a connection with a |sec| server.
    """

    signalNotification = QtCore.pyqtSignal(str)
    signalConnectionReady = QtCore.pyqtSignal(object)

    def __init__(self, host, port):
        super(ConnectionThread, self).__init__()
        self.host = host
        self.port = port
        self.connection = None

    @pyqtSlot()
    def run(self):
        self.signalNotification.emit('Connecting to Secondo Server on ' + self.host + '/' + self.port)
        try:
            self.connection = api.connect(self.host, self.port)
        except api.InterfaceError as e:
            self.signalNotification.emit(e.message)
        except api.OperationalError as e:
            self.signalNotification.emit(e.message)
        else:
            self.signalNotification.emit('Connection successful!')
            self.signalConnectionReady.emit(self.connection)


class ImportToSecondo(QThread):
    """
    This class implements a thread to transfer a relation to a database in the |sec| server.
    """

    signalStart = QtCore.pyqtSignal(int)
    signalProgress = QtCore.pyqtSignal(int)
    signalReady = QtCore.pyqtSignal(str)
    signalError = QtCore.pyqtSignal(str)

    def __init__(self, secondo_connection: api.Connection,
                 relation_name: str, field_attributes: [],
                 tuples: []):
        super(ImportToSecondo, self).__init__()
        self.connection = secondo_connection
        self.relation_name = relation_name
        self.field_attributes = field_attributes
        self.tuples = tuples

    @pyqtSlot()
    def run(self):

        self.signalStart.emit(len(self.tuples))

        cursor = self.connection.cursor()

        try:

            cursor.execute_create_empty_relation(self.relation_name, self.field_attributes)

            # Call Secondo-API method to insert values into created relation

            tuple_counter = 0
            for single_tuple in self.tuples:

                try:
                    cursor.execute_insert_tuple_into_relation(self.relation_name, single_tuple)
                except api.InterfaceError as e:
                    self.signalError.emit(e.message)

                except api.ProgrammingError as e:
                    self.signalError.emit(e.message)

                # Display progress

                tuple_counter += 1
                self.signalProgress.emit(tuple_counter)

            self.signalReady.emit("The relation " + self.relation_name + " was created successfully.")

        except api.InterfaceError as e:
            self.signalError.emit(e.message)
        except api.ProgrammingError as e:
            self.signalError.emit(e.message)


class MainWindowModel(QtCore.QObject):
    """
    This class implements the data model of the main window of the SecondoDB App. All interactions with the |sec| API is
    implemented in this class.
    """

    signalStatusBarMessage = QtCore.pyqtSignal(str)
    signalNavigationTreeReady = QtCore.pyqtSignal(object)
    signalNavigationTreeDatabases = QtCore.pyqtSignal()
    signalConnectionReady = QtCore.pyqtSignal()
    signalDisconnected = QtCore.pyqtSignal()
    signalConnectIconToggle = QtCore.pyqtSignal(bool)
    signalStartProgress = QtCore.pyqtSignal(int)
    signalProgress = QtCore.pyqtSignal(int)
    signalStopProgress = QtCore.pyqtSignal()
    signalQGISMessageSuccess = QtCore.pyqtSignal(str)
    signalQGISMessageInfo = QtCore.pyqtSignal(str)
    signalQGISMessageError = QtCore.pyqtSignal(str)

    def __init__(self):

        super(MainWindowModel, self).__init__()

        self.connection: api.Connection
        self.thread = None
        self.model: QStandardItemModel = QStandardItemModel()
        self.open_database_index = None
        self.parameters = {}
        initialize_icons()
        self.load_parameters()

    def load_parameters(self) -> None:
        """
        Loads the parameters from the settings file.

        :return: None
        """

        s = QgsSettings()

        if s.value("secondodb/hostname") is not None:
            self.parameters.update({'hostname': s.value("secondodb/hostname")})
        else:
            self.parameters.update({'hostname': '127.0.0.1'})

        if s.value("secondodb/port") is not None:
            self.parameters.update({'port': s.value("secondodb/port")})
        else:
            self.parameters.update({'port': '1234'})

        if s.value("secondodb/loadtypes") is not None:
            self.parameters.update({'loadtypes': s.value("secondodb/loadtypes")})
        else:
            self.parameters.update({'loadtypes': 0})

        if s.value("secondodb/loadalgebras") is not None:
            self.parameters.update({'loadalgebras': s.value("secondodb/loadalgebras")})
        else:
            self.parameters.update({'loadalgebras': 0})

        # self.parameters.update({'maxentries': s.value("secondodb/maxentries")})

        if s.value("secondodb/framespersecond") is not None:
            self.parameters.update({'framespersecond': s.value("secondodb/framespersecond")})
        else:
            self.parameters.update({'framespersecond': 1})

    def handle_import_to_secondo(self, relation_name: str, field_attributes: [], tuples: []) -> None:
        """
        Handles the import of objects to |sec| in a separated thread.

        :param relation_name: The name of the relation.
        :param field_attributes: The attributes of the fields of the relation.
        :param tuples: The entries of the relation.
        :return: None
        """

        self.thread = ImportToSecondo(self.connection, relation_name, field_attributes, tuples)
        self.thread.signalStart.connect(self.handle_start_progress)
        self.thread.signalProgress.connect(self.handle_progress)
        self.thread.signalReady.connect(self.handle_qgis_message_success)
        self.thread.signalReady.connect(self.handle_stop_progress)
        self.thread.signalError.connect(self.handle_qgis_message_error)
        self.thread.start()

    def handle_start_progress(self, maximum) -> None:
        """
        Emits the start signal when processing in background.

        :param maximum: The maximum to be set in the progress bar.
        :return: None
        """
        self.signalStartProgress.emit(maximum)

    def handle_progress(self, value) -> None:
        """
        Emits the value of the progress to be refreshed in the progress bar during a background work.

        :param value: The value to be displayed.
        :return: None
        """
        self.signalProgress.emit(value)

    def handle_stop_progress(self) -> None:
        """
        Emits the stop progress signal after processing in background.

        :return: None
        """
        self.signalStopProgress.emit()

    def handle_qgis_message_success(self, message: str) -> None:
        """
        Emits an success message to QGIS.

        :param message: The message to be displayed.
        :return: None
        """
        self.signalQGISMessageSuccess.emit(message)

    def handle_qgis_message_info(self, message: str) -> None:
        """
        Emits an information message to QGIS.

        :param message: The message to be displayed.
        :return: None
        """
        self.signalQGISMessageInfo.emit(message)

    def handle_qgis_message_error(self, message: str) -> None:
        """
        Emits an error message to QGIS.

        :param message: The message to be displayed.
        :return: None
        """
        self.signalQGISMessageError.emit(message)

    def handle_notifications(self, message: str) -> None:
        """
        This method handles the notifications sent by the signals of the implemented objects of the model, for example
        notifications from the server or from the connection process. The method emits a signal, which can be used to
        deliver information to the user in the status bar of the GUI.

        :param message: A string with the message.
        :return: None
        """
        self.signalStatusBarMessage.emit(message)

    def get_connection(self):
        """
        Returns a connection object, if it was previously created and initialized.

        :return: The connection object.
        """
        if self.connection is not None:
            if self.connection.initialized:
                return self.connection
        else:
            return None

    def connect_to_secondo_server(self, host: str, port: str) -> None:
        """
        This method negotiates a connection with a |sec| server fpr a combination of an IP-Address/hostname and a port.
        The connection will be initiated in a separated thread in order to not to freeze the GUI during the connection
        process.

        :param host: The IP-Address or the hostname of the |sec| server.
        :param port: The port of the |sec| server.
        :return: None
        """
        self.thread = ConnectionThread(host, port)
        self.thread.signalNotification.connect(self.handle_notifications)
        self.thread.signalConnectionReady.connect(self.handle_connection_ready)
        self.thread.start()

    def disconnect_from_secondo_server(self, fatal_error_disconnection=False) -> None:
        """
        Disconnects from the |sec| server.

        :return: None
        """

        self.handle_notifications('Disconnecting...')
        try:
            self.connection.close()
        except api.InterfaceError as e:
            self.handle_notifications('Disconnection failed - ' + e.args[0])
        except api.OperationalError as e:
            self.handle_notifications('Disconnection failed - ' + e.args[0])
        else:
            self.model.clear()
            if not fatal_error_disconnection:
                self.handle_notifications('Disconnected')
            else:
                self.handle_notifications('Error in server connection: Disconnected.')
            self.signalDisconnected.emit()

    def handle_connection_ready(self, connection: api.Connection) -> None:
        """
        Handles the events after the creation of a connection object.

        :param connection: A connection object.
        :return: None
        """
        self.connection = connection
        self.signalConnectionReady.emit()
        self.load_navigation_tree_at_server_level()

    def set_open_database_icon(self, index: QModelIndex) -> None:
        """
        Sets the open database icon in the model after connection to a database.

        :param index: The index of the selected database.
        :return: None
        """
        item = self.model.itemFromIndex(index)

        item.setIcon(get_icon_for_object('dbopen'))

        self.signalNavigationTreeReady.emit(self.model)
        self.signalNavigationTreeDatabases.emit()

    def open_database(self, dbname: str, index: QModelIndex) -> bool:
        """
        Opens a database connection in the |sec| server.

        :param index: The index object of the selected database in the model.
        :param dbname: The name of the database.
        :return: True, if the connection object is now connected to a specific database, otherwise false.
        """

        if self.connection.initialized:
            try:
                self.connection.open_database(dbname)
            except api.InterfaceError as e:
                self.handle_notifications(e.message)
                return False
            except api.OperationalError as e:
                self.handle_notifications(e.message)
                return False
            else:
                self.open_database_index = index
                self.signalConnectIconToggle.emit(False)
                self.set_open_database_icon(index)
                self.load_navigation_tree_at_database_level(index)
                self.handle_notifications('Database ' + dbname + ' opened successfully.')
                return True

    def create_new_relation(self, relation_name: str, attributes: []) -> bool:
        """
        Creates a new relation in the database.

        :param relation_name: The name of the relation.
        :param attributes: A list with the attributes of the relation.
        :return: True, if the relation was successfully created.
        """

        if self.connection is not None:

            if self.connection.initialized and not self.connection.server_mode_only:
                try:
                    cursor = self.connection.cursor()
                    cursor.execute_create_empty_relation(relation_name, attributes)
                except api.InterfaceError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                except api.OperationalError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                except api.InternalError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                except api.ProgrammingError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                else:
                    self.handle_notifications('Relation ' + relation_name + ' created successfully.')
                    return True

    def delete_object(self, object_name: str) -> bool:
        """
        Deletes the object from the database using its name.

        :param object_name: The name of the object.
        :return: True, if the object was successfully deleted.
        """

        if self.connection is not None:

            if self.connection.initialized:
                try:
                    cursor = self.connection.cursor()
                    cursor.execute_delete(object_name)
                except api.InterfaceError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                else:
                    self.handle_notifications('Object ' + object_name.upper() + ' deleted successfully.')
                    return True
        else:
            self.handle_notifications('Error: The connection to the Secondo server is not available.')
            return False

    def add_database(self, dbname: str) -> bool:
        """
        Handles the creation of a database in the |sec| server.

        :param dbname: The name of the database.
        :return: True, if the database was added successfully.
        """

        if self.connection is not None:

            if self.connection.initialized:
                try:
                    self.connection.create_database(dbname)
                except api.InterfaceError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                except api.OperationalError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                else:
                    self.handle_notifications('Database ' + dbname.upper() + ' created successfully.')
                    self.refresh_databases_navigation_tree()
                    return True

        else:
            self.handle_notifications('Error: The connection to the Secondo server is not available.')
            return False

    def delete_database(self, dbname: str) -> bool:
        """
        Deletes the database using its name.

        :param dbname: The name of the database.
        :return: True, if the database was successfully deleted.
        """

        if self.connection is not None:

            if self.connection.initialized:
                try:
                    self.connection.delete_database(dbname)
                except api.InterfaceError as e:
                    self.handle_notifications('Error: ' + e.message)
                    return False
                else:
                    self.handle_notifications('Database ' + dbname.upper() + ' deleted successfully.')
                    self.refresh_databases_navigation_tree()
                    return True
        else:
            self.handle_notifications('Error: The connection to the Secondo server is not available.')
            return False

    def close_database(self) -> bool:
        """
        Closes the current connection.

        :return: True, if the database was closed successfully.
        """

        if self.connection is not None:
            if self.connection.initialized:
                try:
                    self.connection.close_database()
                except api.InterfaceError as e:  # Fatal error -> Disconnect from server, restore models
                    self.handle_notifications('Error: ' + e.message + ' Disconnecting...')
                    self.disconnect_from_secondo_server(fatal_error_disconnection=True)
                    return False
                else:
                    self.handle_notifications('Database closed successfully.')
                    self.refresh_databases_navigation_tree()
                    self.open_database_index = None
                    self.signalConnectIconToggle.emit(True)
                    return True
        else:
            self.handle_notifications('Error: The connection to the Secondo server is not available.')
            return False

    def check_connection(self) -> bool:
        """
        Checks if a connection object has been created.

        :return: True, if the connection object is available, otherwise false.
        """

        if self.connection is not None:
            return True
        else:
            return False

    def load_object_from_database(self, object_name: str, object_type: str) -> QStandardItemModel:
        """
        Loads an object from the database using its name and type. After receiving the data from server a model will be
        created, which can be used in the data viewer of the object or for exporting purposes.

        :param object_name: The name of the object.
        :param object_type: The type of the object.
        :return: A model.
        """

        cursor = self.connection.cursor()
        try:
            self.handle_notifications('Loading object ' + object_name + '...')
            query_results = cursor.execute('query ' + object_name)
        except api.InterfaceError as e:  # No database open
            self.handle_notifications(e.message)
        except api.ProgrammingError as e:
            self.handle_notifications(e.message)
        else:
            self.handle_notifications('Object ' + object_name + ' loaded successfully.')

            if object_type == 'point':

                return create_model_for_point_object(query_results[0])

            if object_type == 'points':

                return create_model_for_points_object(query_results[0])

            if object_type == 'line':

                return create_model_for_line_object(query_results[0])

            if object_type == 'region':

                return create_model_for_region_object(query_results[0])

            if object_type == 'rel':

                return create_model_for_rel_object(query_results[0])

            if object_type == 'mpoint':

                return create_model_for_mpoint_object(query_results[0])

            if object_type == 'mregion':

                return create_model_for_mregion_object(query_results[0])

    def refresh_databases_navigation_tree(self) -> None:
        """
        Reloads the database items of the model.

        :return: None
        """

        if self.model is not None:

            data_type = 'folder'

            db_folder = QStandardItem('Databases')
            db_folder.setIcon(get_icon_for_object(data_type))
            db_folder.setToolTip('Available databases on the Secondo server')

            data_container = create_data_container(data_type, None)
            db_folder.setData(data_container)

            # Get list databases from secondodb

            db_list = self.connection.get_list_databases()

            counter = 0
            for dbname in db_list:
                data_type = 'database'
                item = QStandardItem(dbname)
                item.setIcon(get_icon_for_object(data_type))
                data_container = create_data_container(data_type, dbname)
                item.setData(data_container)
                db_folder.setChild(counter, 0, item)
                counter += 1

            self.model.setItem(0, 0, db_folder)
            self.signalNavigationTreeReady.emit(self.model)
            self.signalNavigationTreeDatabases.emit()

    def load_navigation_tree_at_database_level(self, index: QModelIndex) -> None:
        """
        Loads the objects after the connection to a database.

        :param index: The index object of the selected database in the model.
        :return: None
        """

        objects = None

        try:
            objects = self.connection.get_list_objects()
        except api.InterfaceError as e:  # Fatal error -> Disconnect
            self.handle_notifications(e.message)
            self.disconnect_from_secondo_server(fatal_error_disconnection=True)

        if objects is not None:

            object_folder = QStandardItem()

            data_type = 'folder'
            data_container = create_data_container(data_type, None)
            object_folder.setData(data_container)

            object_folder.setIcon(get_icon_for_object(data_type))

            counter = 0
            for single_object in objects:

                if single_object.object_type in SUPPORTED_OBJECTS:

                    object_item = QStandardItem(single_object.object_name + ' (' + single_object.object_type + ')')

                    data_type = 'object'
                    data_container = create_data_container(data_type, single_object)
                    object_item.setData(data_container)
                    object_item.setIcon(get_icon_for_object(single_object.object_type))

                    if single_object.object_type == 'rel':

                        attribute_counter = 0
                        for attribute in single_object.attributes:
                            attribute_item = QStandardItem(attribute.attribute_name + ' (' + attribute.attribute_type + ')')
                            data_type = 'attribute'
                            data_container = create_data_container(data_type, attribute)
                            attribute_item.setData(data_container)
                            attribute_item.setIcon(get_icon_for_object(attribute.attribute_type))
                            object_item.setChild(attribute_counter, 0, attribute_item)
                            attribute_counter += 1

                    object_folder.setChild(counter, 0, object_item)
                    counter += 1

                object_folder.setText('Objects (' + str(counter) + ')')

            database_model = self.model.itemFromIndex(index)
            database_model.setChild(0, 0, object_folder)

            self.signalNavigationTreeReady.emit(self.model)

        else:
            self.handle_notifications("Fatal error: Object model couldn't be created.")
            self.disconnect_from_secondo_server(fatal_error_disconnection=True)

    def load_navigation_tree_at_server_level(self) -> None:
        """
        Loads the data model for the navigation tree at server level.

        :return: None
        """

        if self.connection.initialized:

            # Create model object and set title

            model = QStandardItemModel()
            model.setHorizontalHeaderLabels(["Navigation"])

            # Create main folders (databases, algebras)

            data_type = 'folder'

            db_folder = QStandardItem('Databases')
            algebra_folder = QStandardItem('Algebras')
            type_folder = QStandardItem('Types')

            db_folder.setIcon(get_icon_for_object(data_type))
            algebra_folder.setIcon(get_icon_for_object(data_type))
            type_folder.setIcon(get_icon_for_object(data_type))

            db_folder.setToolTip('Available databases on the Secondo server')
            algebra_folder.setToolTip('Available algebra modules on the Secondo server')
            type_folder.setToolTip('Available types on the Secondo server')

            data_container = create_data_container(data_type, None)
            db_folder.setData(data_container)
            algebra_folder.setData(data_container)
            type_folder.setData(data_container)

            model.setItem(0, 0, db_folder)
            model.setItem(1, 0, algebra_folder)
            model.setItem(2, 0, type_folder)

            # Get list databases from secondodb

            self.handle_notifications('Getting databases from Secondo...')
            db_list = self.connection.get_list_databases()

            data_type = 'database'

            counter = 0
            for dbname in db_list:
                item = QStandardItem(dbname)
                item.setIcon(get_icon_for_object(data_type))
                data_container = create_data_container(data_type, dbname)
                item.setData(data_container)
                db_folder.setChild(counter, 0, item)
                counter += 1

            # Get list algebras from secondodb

            if self.parameters['loadalgebras'] == 1:

                self.handle_notifications('Getting algebras from Secondo...')
                algebra_list = self.connection.get_list_algebras()

                counter = 0
                for algebraname in algebra_list:

                    data_type = 'algebra'

                    item = QStandardItem(algebraname)
                    item.setIcon(get_icon_for_object(data_type))

                    algebra_folder.setChild(counter, 0, item)

                    # Get details for algebra

                    algebra = self.connection.get_algebra(algebraname)

                    data_container = create_data_container(data_type, algebra)
                    item.setData(data_container)

                    algebra_type_folder = QStandardItem('Types')
                    algebra_operator_folder = QStandardItem('Operators')

                    data_type = 'folder'

                    algebra_type_folder.setIcon(get_icon_for_object(data_type))
                    algebra_operator_folder.setIcon(get_icon_for_object(data_type))

                    data_container = create_data_container(data_type, None)
                    algebra_type_folder.setData(data_container)
                    algebra_operator_folder.setData(data_container)

                    item.setChild(0, 0, algebra_type_folder)
                    item.setChild(1, 0, algebra_operator_folder)

                    data_type = 'type'

                    algebra_type_counter = 0
                    for type_constructor in algebra.type_list:
                        type_item = QStandardItem(type_constructor.type_name)
                        type_item.setIcon(get_icon_for_object(data_type))

                        data_container = create_data_container(data_type, type_constructor)
                        type_item.setData(data_container)

                        algebra_type_folder.setChild(algebra_type_counter, 0, type_item)
                        algebra_type_counter += 1

                    data_type = 'operator'

                    algebra_operator_counter = 0
                    for operator in algebra.operator_list:
                        operator_item = QStandardItem(operator.operator_name)
                        operator_item.setIcon(get_icon_for_object(data_type))

                        data_container = create_data_container(data_type, operator)
                        operator_item.setData(data_container)

                        algebra_operator_folder.setChild(algebra_operator_counter, 0, operator_item)
                        algebra_operator_counter += 1

                    counter += 1

            # Get list type constructors from secondodb

            if self.parameters['loadtypes'] == 1:

                self.handle_notifications('Getting type constructors from Secondo...')
                type_list = self.connection.get_list_type_constructors()

                data_type = 'type'

                counter = 0
                for typename in type_list:
                    item = QStandardItem(typename)
                    item.setIcon(get_icon_for_object(data_type))

                    data_container = create_data_container(data_type, None)
                    item.setData(data_container)

                    type_folder.setChild(counter, 0, item)
                    counter += 1

            # Emit signal tree ready for display - propagate model to controller

            self.signalNavigationTreeReady.emit(model)
            self.handle_notifications('Ready')
            self.model = model
