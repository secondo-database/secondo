# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Main Window
# mainWindow.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class MainWindow, which implements the view of the main window.
"""

# General Python imports
import os
from collections import namedtuple
from datetime import datetime

# PyQt imports

from PyQt5 import QtGui, QtCore
from PyQt5.QtGui import QStandardItemModel
from PyQt5.QtCore import Qt, QModelIndex
from PyQt5.QtWidgets import QMessageBox, QMainWindow, QStackedWidget, QWidget, QApplication, QHeaderView, QComboBox, \
    QTableView, QAbstractItemView, QDialog, QProgressBar

# Views imports
from qgis._core import Qgis

from secondodb.ui.views.connectDialog import ConnectDialog
from secondodb.ui.views.addDatabaseDialog import AddDatabaseDialog
from secondodb.ui.views.addToLayerDialog import AddToLayerDialog
from secondodb.ui.views.addMovingPointToLayerDialog import AddMovingPointToLayerDialog
from secondodb.ui.views.addMovingRegionToLayerDialog import AddMovingRegionToLayerDialog
from secondodb.ui.views.importFeaturesFromQGISDialog import ImportFeaturesFromQGISDialog
from secondodb.ui.views.settingsDialog import SettingsDialog
from secondodb.ui.views.aboutDialog import AboutDialog
from secondodb.ui.views.queryDialog import QueryDialog

# Widgets imports

from secondodb.ui.views.widgets.mainWindowView import Ui_MainWindow
from secondodb.ui.views.widgets.tabWidgets import relationWidget, typeWidget, emptyWidget, createRelationWidget, \
    operatorWidget, spatioTemporalWidget, spatialWidget, spatialRegionWidget

# Model imports

from secondodb.ui.models.mainWindowModel import MainWindowModel

# QGIS imports

from qgis._gui import QgisInterface

# General settings for application - high resolution displays

if hasattr(QtCore.Qt, 'AA_EnableHighDpiScaling'):
    QApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling, True)

if hasattr(QtCore.Qt, 'AA_UseHighDpiPixmaps'):
    QApplication.setAttribute(QtCore.Qt.AA_UseHighDpiPixmaps, True)

SUPPORTED_TYPES_FOR_REL = ['int', 'bool', 'string', 'real', 'point', 'points', 'line', 'region']


class MainWindow(QMainWindow):
    """
    This class implements the view for the main window of the SecondoDB environment.
    """

    def __init__(self, qgis_interface: QgisInterface = None, main_dialog: QDialog = None):
        """
        The constructor of the class.

        :param qgis_interface: The QGIS-Interface object for the communication with the QGIS-Environment.
        """

        # Initialize QGIS Interface

        super().__init__()
        self.QgsInterface = qgis_interface
        self.main_dialog = main_dialog

        # Initialize Main Window View

        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.splitter.setStretchFactor(1, 2)

        # Initialize Menubar

        self.ui.actionConnectToSecondoServer.triggered.connect(self.handle_connect_to_server)
        self.ui.actionDisconnectFromServer.triggered.connect(self.handle_disconnect_from_server)
        self.ui.actionFrom_QGIS_Layer.triggered.connect(lambda: self.handle_import_from_qgis(False))
        self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.triggered.connect(lambda:
                                                                                   self.handle_import_from_qgis(True))
        self.ui.actionSettings.triggered.connect(self.handle_settings)
        self.ui.actionAbout_SecondoDB_for_QGIS.triggered.connect(self.handle_about)
        self.ui.actionUser_Documentation.triggered.connect(self.handle_user_documentation)

        self.ui.actionFrom_QGIS_Layer.setDisabled(True)
        self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(True)
        self.ui.actionDisconnectFromServer.setDisabled(True)

        # Initialize Toolbar

        self.ui.actionAdd_New_Database.triggered.connect(self.handle_add_database)
        self.ui.actionDelete_Database.triggered.connect(self.handle_delete_database)
        self.ui.actionConnect_to_Database.triggered.connect(self.handle_connect_to_database)
        self.ui.actionDisconnect.triggered.connect(self.handle_disconnect_from_database)
        self.ui.actionRefresh.triggered.connect(self.handle_refresh)
        self.ui.actionCreateNewRelation.triggered.connect(self.handle_create_new_relation)
        self.ui.actionDeleteObject.triggered.connect(self.handle_delete_object)
        self.ui.actionQuery.triggered.connect(self.handle_query)

        self.ui.actionConnect_to_Database.setDisabled(True)
        self.ui.actionDisconnect.setDisabled(True)
        self.ui.actionRefresh.setDisabled(True)
        self.ui.actionAdd_New_Database.setDisabled(True)
        self.ui.actionDelete_Database.setDisabled(True)
        self.ui.actionCreateNewRelation.setDisabled(True)
        self.ui.actionQuery.setDisabled(True)
        self.ui.actionDeleteObject.setDisabled(True)
        self.ui.actionFrom_QGIS_Layer.setDisabled(True)
        self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(True)

        # Initialize Main Window Model

        self.MainWindowModel = MainWindowModel()
        self.MainWindowModel.signalStatusBarMessage.connect(self.handle_status_message)
        self.MainWindowModel.signalNavigationTreeReady.connect(self.handle_navigation_tree_ready)
        self.MainWindowModel.signalNavigationTreeDatabases.connect(self.handle_refresh_database_items)
        self.MainWindowModel.signalConnectionReady.connect(self.handle_connection_ready)
        self.MainWindowModel.signalDisconnected.connect(self.handle_disconnected)
        self.MainWindowModel.signalConnectIconToggle.connect(self.handle_toggle_connect_disconnect_db)
        self.MainWindowModel.signalProgress.connect(self.handle_progress)
        self.MainWindowModel.signalStartProgress.connect(self.handle_start_progress)
        self.MainWindowModel.signalStopProgress.connect(self.handle_stop_progress)
        self.MainWindowModel.signalQGISMessageSuccess.connect(self.handle_qgis_message_success)
        self.MainWindowModel.signalQGISMessageInfo.connect(self.handle_qgis_message_info)
        self.MainWindowModel.signalQGISMessageError.connect(self.handle_qgis_message_error)

        # Initialize tree view

        self.ui.treeView.doubleClicked.connect(self.handle_double_clicked_tree)

        # Initialize widgets of tab area

        self.stackedWidgetTab1 = QStackedWidget()
        self.ui.tabLayout.addWidget(self.stackedWidgetTab1)

        # 0 - Empty

        self.emptyWidget = QWidget()
        self.emptyWidget.ui = emptyWidget.Ui_Form()
        self.emptyWidget.ui.setupUi(self.emptyWidget)
        self.stackedWidgetTab1.addWidget(self.emptyWidget)

        # 1 - Type

        self.typeWidget = QWidget()
        self.typeWidget.ui = typeWidget.Ui_Form()
        self.typeWidget.ui.setupUi(self.typeWidget)
        self.stackedWidgetTab1.addWidget(self.typeWidget)

        # 2 - Operator

        self.operatorWidget = QWidget()
        self.operatorWidget.ui = operatorWidget.Ui_Form()
        self.operatorWidget.ui.setupUi(self.operatorWidget)
        self.stackedWidgetTab1.addWidget(self.operatorWidget)

        # 3 - Spatial Object (point, points, line)

        self.spatialWidget = QWidget()
        self.spatialWidget.ui = spatialWidget.Ui_Form()
        self.spatialWidget.ui.setupUi(self.spatialWidget)
        self.stackedWidgetTab1.addWidget(self.spatialWidget)
        self.spatialWidget.ui.pushButtonAddToLayer.clicked.connect(self.handle_add_selected_rows_to_layer)

        # 4 - Spatial object (region)

        self.spatialRegionWidget = QWidget()
        self.spatialRegionWidget.ui = spatialRegionWidget.Ui_Form()
        self.spatialRegionWidget.ui.setupUi(self.spatialRegionWidget)
        self.stackedWidgetTab1.addWidget(self.spatialRegionWidget)
        self.spatialRegionWidget.ui.pushButtonAddToLayer.clicked.connect(self.handle_add_selected_rows_to_layer)

        # 5 - Relational object

        self.relationalWidget = QWidget()
        self.relationalWidget.ui = relationWidget.Ui_Form()
        self.relationalWidget.ui.setupUi(self.relationalWidget)
        self.stackedWidgetTab1.addWidget(self.relationalWidget)
        self.relationalWidget.ui.pushButtonAddToLayer.clicked.connect(self.handle_add_selected_rows_to_layer)

        # 6 - Spatio-Temporal object

        self.spatioTemporalWidget = QWidget()
        self.spatioTemporalWidget.ui = spatioTemporalWidget.Ui_Form()
        self.spatioTemporalWidget.ui.setupUi(self.spatioTemporalWidget)
        self.stackedWidgetTab1.addWidget(self.spatioTemporalWidget)
        self.spatioTemporalWidget.ui.pushButtonAddToLayer.clicked.connect(self.handle_add_moving_object_to_layer)

        # 7 - Create empty relation

        self.createRelationWidget = QWidget()
        self.createRelationWidget.ui = createRelationWidget.Ui_Form()
        self.createRelationWidget.ui.setupUi(self.createRelationWidget)
        self.stackedWidgetTab1.addWidget(self.createRelationWidget)

        self.stackedWidgetTab1.setCurrentIndex(0)  # Set empty widget at start

        self.dataViewerModel = None
        self.progress = QProgressBar()

    def handle_start_progress(self, maximum: int) -> None:
        """
        Handles the initialization of the progress display when importing objects to |sec|.

        :param maximum: Maximal value for the progress bar.
        :return: None
        """

        progress_message_bar = self.QgsInterface.messageBar().createMessage("Importing object to Secondo...")
        self.progress = QProgressBar()
        self.progress.setMaximum(maximum)
        self.progress.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        progress_message_bar.layout().addWidget(self.progress)
        self.QgsInterface.messageBar().pushWidget(progress_message_bar, Qgis.Info)
        self.main_dialog.close()

    def handle_progress(self, value: int) -> None:
        """
        Handles the setting of a value to refresh the progress bar.

        :param value:
        :return:
        """
        self.progress.setValue(value)

    def handle_stop_progress(self) -> None:
        """
        Handles the finishing of a progress display when importing objects to |sec|.

        :return: None
        """
        self.main_dialog.show()
        self.handle_refresh()

    def handle_qgis_message_success(self, message: str) -> None:
        """
        Handles the display of a success message in QGIS.

        :param message: The message to be displayed.
        :return: None
        """
        self.QgsInterface.messageBar().clearWidgets()
        self.QgsInterface.messageBar().pushMessage(message, level=Qgis.Success, duration=5)

    def handle_qgis_message_info(self, message: str) -> None:
        """
        Handles the display of an information message in QGIS.

        :param message: The message to be displayed.
        :return: None
        """
        self.QgsInterface.messageBar().clearWidgets()
        self.QgsInterface.messageBar().pushMessage(message, level=Qgis.Info, duration=5)

    def handle_qgis_message_error(self, message: str) -> None:
        """
        Handles the display of an error message in QGIS. If a progress bar was initialized, it will be cleared.

        :param message: The message to be displayed.
        :return: None
        """
        # self.QgsInterface.messageBar().clearWidgets()
        self.QgsInterface.messageBar().pushMessage(message, level=Qgis.Critical, duration=20)
        self.main_dialog.show()

    def handle_query(self) -> None:
        """
        Handles the display of the query dialog.

        :return: None
        """
        QueryDialog(self.MainWindowModel.connection)

    def handle_about(self) -> None:
        """
        Handles the display of the about dialog.

        :return: None
        """
        AboutDialog()

    def handle_user_documentation(self) -> None:
        """
        Handles the display of the user documentation.

        :return: None
        """

        dir_views = os.path.dirname(__file__)
        dir_ui = os.path.dirname(dir_views)
        dir_secondodb = os.path.dirname(dir_ui)

        filename = os.path.join(dir_secondodb, 'help', 'userdoc.pdf')
        try:
            os.startfile(filename)
        except:
            self.handle_status_message("User documentation is not available.")

    def handle_settings(self) -> None:
        """
        Handles the display of the settings dialog.

        :return: None
        """
        SettingsDialog(self)

    def handle_delete_object(self) -> None:
        """
        Handles the deletion of the selected object from the database.

        :return: None
        """
        index = self.ui.treeView.currentIndex()

        try:
            data = index.model().itemData(index)
        except AttributeError as e:
            self.handle_status_message('Error: Please select a database from the navigation tree.')
        else:
            data_type = data[257].data_type
            if data_type == 'object':

                object_name = data[257].data_object.object_name

                # Show message box to confirm

                message = QMessageBox()
                message.setIcon(QMessageBox.Information)
                message.setWindowTitle('Delete an object')

                icon_deleteobject = QtGui.QIcon()
                icon_deleteobject.addPixmap(QtGui.QPixmap(":/icons/deleteobject.png"),
                                            QtGui.QIcon.Normal,
                                            QtGui.QIcon.Off)
                message.setWindowIcon(icon_deleteobject)

                message.setText('You are about to delete an object')
                message.setInformativeText('Are you sure you want to delete the object '
                                           + object_name + ' permanently?')

                message.setStandardButtons(QMessageBox.Yes | QMessageBox.No)

                return_value = message.exec_()

                if return_value == QMessageBox.Yes:
                    return_value = self.MainWindowModel.delete_object(object_name)

                    if return_value:
                        self.handle_refresh()
                        # self.handle_toggle_connect_disconnect_db(True)
                    else:
                        self.handle_status_message("Error: The object " + object_name + " couldn't be deleted.")

                elif return_value == QMessageBox.No:
                    self.handle_status_message("Action canceled by the user.")

            else:
                self.handle_status_message('Error: Please select an object from the navigation tree.')

    def handle_create_new_relation(self) -> None:
        """
        Handles the creation of a new relation.

        :return: None
        """

        model = None
        current_widget = None

        def add_attribute() -> None:
            """
            Adds an attribute to the relation.

            :return: None
            """
            row = []
            name_item = QtGui.QStandardItem()
            type_item = QtGui.QStandardItem()
            row.append(name_item)
            row.append(type_item)
            model.appendRow(row)

            combobox_type = QComboBox()
            combobox_type.addItems(['string', 'int', 'point', 'line', 'region'])

            index = model.indexFromItem(type_item)
            current_widget.ui.tableView.setIndexWidget(index, combobox_type)

        def delete_attribute() -> None:
            """
            Deletes an attribute of the relation.

            :return: None
            """
            selected_rows = current_widget.ui.tableView.selectionModel().selectedRows()

            for index in selected_rows:
                row_number = index.row()
                model.removeRow(row_number)

        def create_attributes_model() -> QStandardItemModel:
            """
            Creates a Qt-Model for the relation.

            :return: The model object.
            """
            _model = QtGui.QStandardItemModel()
            _model.setHorizontalHeaderLabels(['Name', 'Type'])
            return _model

        def create_relation() -> None:
            """
            Creates the relation and saves it in the |sec|-server.

            :return: None
            """

            # Get data together

            relation_name = current_widget.ui.lineEditName.text()
            _model: QtGui.QStandardItemModel = current_widget.ui.tableView.model()

            attributes = []
            for i in range(_model.rowCount()):
                attribute_name = _model.index(i, 0).data()

                table_view: QTableView = current_widget.ui.tableView
                attribute_type = table_view.indexWidget(_model.index(i, 1)).currentText()

                attribute = [attribute_name, attribute_type]
                attributes.append(attribute)

            # Send data to main window model to perform DB query

            self.MainWindowModel.create_new_relation(relation_name, attributes)
            self.handle_refresh()

        # Set widget and label

        self.stackedWidgetTab1.setCurrentIndex(7)
        self.ui.tabWidget.setTabText(0, 'Create new relation')
        current_widget = self.stackedWidgetTab1.currentWidget()

        current_widget.ui.tableView.setEditTriggers(QAbstractItemView.AllEditTriggers)

        # Connect actions of the table view to add/delete attributes

        current_widget.ui.pushButtonAddAttribute.clicked.connect(add_attribute)
        current_widget.ui.pushButtonDeleteAttribute.clicked.connect(delete_attribute)
        current_widget.ui.pushButtonCreate.clicked.connect(create_relation)

        # Set initial model

        model = create_attributes_model()
        current_widget.ui.tableView.setModel(model)

    def handle_connection_ready(self) -> None:
        """
        Handles the actions after the connection to the |sec| server.

        :return: None
        """
        self.ui.actionConnect_to_Database.setDisabled(False)
        self.ui.actionAdd_New_Database.setDisabled(False)
        self.ui.actionDisconnectFromServer.setDisabled(False)
        self.ui.actionConnectToSecondoServer.setDisabled(True)

    def handle_disconnected(self) -> None:
        """
        Handles the actions after the disconnection from the |sec| server.

        :return: None
        """
        self.ui.actionDisconnectFromServer.setDisabled(True)
        self.ui.actionConnectToSecondoServer.setDisabled(False)
        self.ui.actionConnect_to_Database.setDisabled(True)
        self.ui.actionDisconnect.setDisabled(True)
        self.ui.actionRefresh.setDisabled(True)
        self.ui.actionDeleteObject.setDisabled(True)
        self.ui.actionFrom_QGIS_Layer.setDisabled(True)
        self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(True)
        self.ui.actionAdd_New_Database.setDisabled(True)
        self.ui.actionDelete_Database.setDisabled(True)
        self.ui.actionCreateNewRelation.setDisabled(True)
        self.ui.actionQuery.setDisabled(True)
        self.stackedWidgetTab1.setCurrentIndex(0)
        self.ui.tabWidget.setTabText(0, '')

    def handle_connect_to_server(self) -> None:
        """
        Opens the dialog for the connection to the |sec| server.

        :return: None
        """
        ConnectDialog(self)

    def handle_disconnect_from_server(self) -> None:
        """
        Handles the disconnect from server action.

        :return: None
        """
        self.MainWindowModel.disconnect_from_secondo_server()

    def handle_import_from_qgis(self, only_selected: bool) -> None:
        """
        Handles the import of objects from QGIS.

        :param only_selected: Set True, if only selected features of layer should be imported.
        :return: None
        """

        ImportFeaturesFromQGISDialog(self.QgsInterface,
                                     secondo_connection=self.MainWindowModel.connection,
                                     only_selected=only_selected, main_window=self)

    def handle_import_to_secondo(self, relation_name, field_attributes, tuples):

        self.MainWindowModel.handle_import_to_secondo(relation_name, field_attributes, tuples)

    def handle_status_message(self, message: str) -> None:
        """
        Handles the display of messages in the status bar.

        :param message: A message.
        :return: None
        """
        self.ui.statusbar.showMessage(message, 3000)
        self.ui.statusbar.repaint()

    def handle_navigation_tree_ready(self, model: QStandardItemModel) -> None:
        """
        Handles the ready signal after creation of the tree view model.

        :param model: The model object as a StandardItemModel.
        :return: None
        """
        self.ui.treeView.setModel(model)

    def handle_double_clicked_tree(self, index: QModelIndex) -> None:
        """
        Handles the double clicked signal from the tree view object.

        :param index: The index object from the clicked item in the tree view.
        :return: None
        """

        item = self.ui.treeView.selectedIndexes()[0]
        data = item.model().itemData(index)
        data_type = data[257].data_type
        data_object = data[257].data_object

        if data_type == 'type':
            self._set_tab_widget_for_type(data_object)
        elif data_type == 'algebra':
            print('oh, an algebra!')
        elif data_type == 'operator':
            self._set_tab_widget_for_operator(data_object)
        elif data_type == 'database':
            self.handle_connect_to_database()
        elif data_type == 'object':
            if data_object.object_type == 'point' \
                    or data_object.object_type == 'points' \
                    or data_object.object_type == 'line':
                self._set_tab_widget_for_spatial_object(data_object)
            if data_object.object_type == 'region':
                self._set_tab_widget_for_spatial_region_object(data_object)
            if data_object.object_type == 'rel':
                self._set_tab_widget_for_relation_object(data_object)
            if data_object.object_type == 'mpoint':
                self._set_tab_view_for_spatiotemporal_object(data_object)
            if data_object.object_type == 'mregion':
                self._set_tab_view_for_spatiotemporal_object(data_object)

    def handle_add_database(self) -> None:
        """
        Handles the creation of a new database.

        :return: None
        """
        AddDatabaseDialog(self)

    def handle_delete_database(self) -> None:
        """
        Deletes the currently selected database in the tree view.

        :return: None
        """
        index = self.ui.treeView.currentIndex()

        try:
            data = index.model().itemData(index)
        except AttributeError as e:
            self.handle_status_message('Error: Please select a database from the navigation tree.')
        else:
            data_type = data[257].data_type
            if data_type == 'database':

                dbname = data[257].data_object

                # Show message box to confirm

                message = QMessageBox()
                message.setIcon(QMessageBox.Information)
                message.setWindowTitle('Delete a database')

                icon_deletedb = QtGui.QIcon()
                icon_deletedb.addPixmap(QtGui.QPixmap(":/icons/deletedatabase.png"),
                                        QtGui.QIcon.Normal,
                                        QtGui.QIcon.Off)
                message.setWindowIcon(icon_deletedb)

                message.setText('You are about to delete a database')
                message.setInformativeText('Are you sure you want to delete the database ' + dbname + ' permanently?')
                message.setStandardButtons(QMessageBox.Yes | QMessageBox.No)

                return_value = message.exec_()

                if return_value == QMessageBox.Yes:
                    return_value = self.MainWindowModel.delete_database(dbname)

                    if return_value:
                        self.handle_toggle_connect_disconnect_db(True)
                    else:
                        self.handle_status_message("Error: The database " + dbname + " couldn't be deleted.")

                elif return_value == QMessageBox.No:
                    self.handle_status_message("Action canceled by the user.")

            else:
                self.handle_status_message('Error: Please select and open a database from the navigation tree.')

    def handle_connected_to_db(self) -> None:
        """
        Handles the actions after opening a database.

        :return: None
        """

        # Expand folder of connected DB in tree view

        self.handle_refresh_database_items()

        # Activate DB functions in toolbar und activate DB functions in menubar

        self.handle_toggle_connect_disconnect_db(False)

    def handle_refresh_database_items(self) -> None:
        """
        Handles the refreshing of the database items in the tree view.

        :return: None
        """

        # Expand folder of DBs in tree view

        db_folder_item = self.MainWindowModel.model.item(0)
        db_folder_index = self.MainWindowModel.model.indexFromItem(db_folder_item)
        self.ui.treeView.expand(db_folder_index)

    def handle_connect_to_database(self) -> None:
        """
        Handles the connection to a selected database on the navigation tree.

        :return: None
        """

        index = self.ui.treeView.currentIndex()
        item_data = self.MainWindowModel.model.itemData(index)

        try:
            data_type = item_data[257].data_type
        except:
            self.handle_status_message('Error: Please select a database from the navigation tree.')
        else:
            if data_type == 'database':
                dbname = item_data[257].data_object
                try:
                    self.MainWindowModel.open_database(dbname, index)
                except AttributeError as e:
                    self.handle_status_message('Error: Please select a database from the navigation tree.')
                else:
                    self.handle_connected_to_db()
            else:
                self.handle_status_message('Error: Please select a database from the navigation tree.')

    def handle_disconnect_from_database(self) -> None:
        """
        Handles the closing of the currently opened database.

        :return: None
        """

        self.MainWindowModel.close_database()
        self.stackedWidgetTab1.setCurrentIndex(0)
        self.ui.tabWidget.setTabText(0, '')

    def handle_toggle_connect_disconnect_db(self, toggle_disconnect: bool) -> None:
        """
        Handles the enabling/disabling of tools after connecting/disconnecting to/from a database.

        :param toggle_disconnect: Set True to disable the functions available only with an available connection to a
                                  database.
        :return: None
        """

        if toggle_disconnect:  # Once disconnected
            self.ui.actionAdd_New_Database.setDisabled(False)
            self.ui.actionConnect_to_Database.setDisabled(False)
            self.ui.actionDisconnect.setDisabled(True)
            self.ui.actionCreateNewRelation.setDisabled(True)
            self.ui.actionRefresh.setDisabled(True)
            self.ui.actionDelete_Database.setDisabled(True)
            self.ui.actionDeleteObject.setDisabled(True)
            self.ui.actionQuery.setDisabled(True)
            self.ui.actionFrom_QGIS_Layer.setDisabled(True)
            self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(True)

            self.ui.actionFrom_QGIS_Layer.setDisabled(True)
            self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(True)

        else:  # Once connected
            self.ui.actionAdd_New_Database.setDisabled(True)
            self.ui.actionConnect_to_Database.setDisabled(True)
            self.ui.actionDisconnect.setDisabled(False)
            self.ui.actionCreateNewRelation.setDisabled(False)
            self.ui.actionRefresh.setDisabled(False)
            self.ui.actionDelete_Database.setDisabled(False)
            self.ui.actionDeleteObject.setDisabled(False)
            self.ui.actionQuery.setDisabled(False)
            self.ui.actionFrom_QGIS_Layer.setDisabled(False)
            self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(False)

            self.ui.actionFrom_QGIS_Layer.setDisabled(False)
            self.ui.actionFrom_Selected_Feature_in_Active_QGIS_Layer.setDisabled(False)

    def handle_refresh(self) -> None:
        """
        Handles the refreshing of the navigation tree.

        :return: None
        """
        open_database_index = self.MainWindowModel.open_database_index

        if open_database_index is not None:
            self.MainWindowModel.load_navigation_tree_at_database_level(open_database_index)
            self.ui.treeView.expand(open_database_index)

    def _set_tab_widget_for_type(self, data_object) -> None:
        """
        Sets the data for the type widget after double click on object.

        :param data_object: A data model object with the data of the type constructor.
        :return: None
        """
        self.stackedWidgetTab1.setCurrentIndex(1)
        self.ui.tabWidget.setTabText(0, 'Type')

        type_widget = self.stackedWidgetTab1.currentWidget()

        try:
            type_widget.ui.lineEditName.setText(data_object.type_name)
        except:
            type_widget.ui.lineEditName.setText('')
        try:
            type_widget.ui.lineEditSignature.setText(data_object.properties['Signature'])
        except:
            type_widget.ui.lineEditSignature.setText('')
        try:
            type_widget.ui.lineEditExampleTypeList.setText(data_object.properties['Example Type List'])
        except:
            type_widget.ui.lineEditExampleTypeList.setText('')
        try:
            type_widget.ui.lineEditListRepresentation.setText(data_object.properties['List Rep'])
        except:
            type_widget.ui.lineEditListRepresentation.setText('')
        try:
            type_widget.ui.lineEditExampleList.setText(data_object.properties['Example List'])
        except:
            type_widget.ui.lineEditExampleList.setText('')
        try:
            type_widget.ui.plainTextEditRemarks.setPlainText(data_object.properties['Remarks'])
        except:
            type_widget.ui.plainTextEditRemarks.setPlainText('')

    def _set_tab_widget_for_operator(self, data_object) -> None:
        """
        Sets the data for the operator widget after double click on object.

        :param data_object: A data model object with the data of the operator.
        :return: None
        """
        self.stackedWidgetTab1.setCurrentIndex(2)

        self.ui.tabWidget.setTabText(0, 'Operator')

        operator_widget = self.stackedWidgetTab1.currentWidget()

        try:
            operator_widget.ui.lineEditName.setText(data_object.operator_name)
        except:
            operator_widget.ui.lineEditName.setText('')
        try:
            operator_widget.ui.plainTextEditSignature.setPlainText(data_object.properties['Signature'])
        except:
            operator_widget.ui.plainTextEditSignature.setPlainText('')
        try:
            operator_widget.ui.lineEditSyntax.setText(data_object.properties['Syntax'])
        except:
            operator_widget.ui.lineEditSyntax.setText('')
        try:
            operator_widget.ui.plainTextEditMeaning.setPlainText(data_object.properties['Meaning'])
        except:
            operator_widget.ui.plainTextEditMeaning.setPlainText('')
        try:
            operator_widget.ui.lineEditExample.setText(data_object.properties['Example'])
        except:
            operator_widget.ui.lineEditExample.setText('')

    def _set_tab_widget_for_spatial_object(self, data_object) -> None:
        """
        Sets the tab widget for a spatial object (point, points, line).

        :param data_object: A data object.
        :return: None
        """

        self.stackedWidgetTab1.setCurrentIndex(3)
        self.ui.tabWidget.setTabText(0, 'Object')
        spatial_widget = self.stackedWidgetTab1.currentWidget()

        try:
            spatial_widget.ui.lineEditName.setText(data_object.object_name)
        except:
            spatial_widget.ui.lineEditName.setText('')
        try:
            spatial_widget.ui.lineEditType.setText(data_object.object_type)
        except:
            spatial_widget.ui.lineEditType.setText('')

        model = self.MainWindowModel.load_object_from_database(data_object.object_name, data_object.object_type)

        if model is not None:
            spatial_widget.ui.tableView.setModel(model)
            header = spatial_widget.ui.tableView.horizontalHeader()

            for i in range(model.columnCount()):
                header.setSectionResizeMode(i, QHeaderView.ResizeToContents)

            self.dataViewerModel = model
            spatial_widget.ui.pushButtonAddToLayer.setDisabled(False)
        else:
            self.handle_status_message("Fatal error: Object model couldn't be created.")

    def _set_tab_widget_for_spatial_region_object(self, data_object) -> None:
        """
        Sets the tab widget for a region object.

        :param data_object: A data object.
        :return: None
        """

        self.stackedWidgetTab1.setCurrentIndex(4)
        self.ui.tabWidget.setTabText(0, 'Object')
        spatial_widget = self.stackedWidgetTab1.currentWidget()

        try:
            spatial_widget.ui.lineEditName.setText(data_object.object_name)
        except:
            spatial_widget.ui.lineEditName.setText('')
        try:
            spatial_widget.ui.lineEditType.setText(data_object.object_type)
        except:
            spatial_widget.ui.lineEditType.setText('')

        model = self.MainWindowModel.load_object_from_database(data_object.object_name, data_object.object_type)

        if model is not None:
            spatial_widget.ui.treeView.setModel(model)
            self.dataViewerModel = model
            spatial_widget.ui.pushButtonAddToLayer.setDisabled(False)
        else:
            self.handle_status_message("Fatal error: Object model couldn't be created.")

    def _set_tab_widget_for_relation_object(self, data_object) -> None:
        """
        Sets the tab widget for a relation.

        :param data_object: A data object.
        :return: None
        """

        self.stackedWidgetTab1.setCurrentIndex(5)
        self.ui.tabWidget.setTabText(0, 'Object')
        relational_widget = self.stackedWidgetTab1.currentWidget()

        try:
            relational_widget.ui.lineEditName.setText(data_object.object_name)
        except:
            relational_widget.ui.lineEditName.setText('')
        try:
            relational_widget.ui.lineEditType.setText(data_object.object_type)
        except:
            relational_widget.ui.lineEditType.setText('')

        model = self.MainWindowModel.load_object_from_database(data_object.object_name, data_object.object_type)

        if model is not None:
            relational_widget.ui.tableView.setModel(model)
            header = relational_widget.ui.tableView.horizontalHeader()

            for i in range(model.columnCount()):
                header.setSectionResizeMode(i, QHeaderView.ResizeToContents)

            self.dataViewerModel = model
            relational_widget.ui.pushButtonAddToLayer.setDisabled(False)

            # Check compatibility of attributes in relation for import

            for attribute in data_object.attributes:
                if attribute.attribute_type not in SUPPORTED_TYPES_FOR_REL:
                    relational_widget.ui.pushButtonAddToLayer.setDisabled(True)

        else:
            self.handle_status_message("Fatal error: Object model couldn't be created.")

    def _set_tab_view_for_spatiotemporal_object(self, data_object) -> None:
        """
        Sets the tab widget for a spatio-temporal object.

        :param data_object: A data object.
        :return: None
        """

        self.stackedWidgetTab1.setCurrentIndex(6)
        self.ui.tabWidget.setTabText(0, 'Object')
        spatio_temporal_widget = self.stackedWidgetTab1.currentWidget()

        try:
            spatio_temporal_widget.ui.lineEditName.setText(data_object.object_name)
        except:
            spatio_temporal_widget.ui.lineEditName.setText('')
        try:
            spatio_temporal_widget.ui.lineEditType.setText(data_object.object_type)
        except:
            spatio_temporal_widget.ui.lineEditType.setText('')

        model = self.MainWindowModel.load_object_from_database(data_object.object_name, data_object.object_type)

        if model is not None:

            spatio_temporal_widget.ui.tableView.setModel(model)

            try:
                spatio_temporal_widget.ui.lineEditStartTime.setText(
                    datetime.strftime(model.horizontalHeaderItem(0).data(), '%Y-%m-%d %H:%M:%S.%f')[:-3])
            except:
                spatio_temporal_widget.ui.lineEditStartTime.setText('')

            try:
                spatio_temporal_widget.ui.lineEditEndTime.setText(
                    datetime.strftime(model.horizontalHeaderItem(1).data(), '%Y-%m-%d %H:%M:%S.%f')[:-3])
            except:
                spatio_temporal_widget.ui.lineEditEndTime.setText('')

            header = spatio_temporal_widget.ui.tableView.horizontalHeader()

            for i in range(model.columnCount()):
                header.setSectionResizeMode(i, QHeaderView.ResizeToContents)

            self.dataViewerModel = model
            spatio_temporal_widget.ui.pushButtonAddToLayer.setDisabled(False)

        else:
            self.handle_status_message("Fatal error: Object model couldn't be created.")

    def handle_add_moving_object_to_layer(self) -> None:
        """
        Handles the transfer of the current moving object to a layer in QGIS.

        :return: None
        """

        current_widget = self.stackedWidgetTab1.currentWidget()
        model: QStandardItemModel = self.dataViewerModel
        object_name = current_widget.ui.lineEditName.text()
        object_type = current_widget.ui.lineEditType.text()

        root_item = model.invisibleRootItem()
        data_from_object = root_item.data(Qt.UserRole)

        selected_data = [object_name, data_from_object]

        frames_per_second = self.MainWindowModel.parameters['framespersecond']

        if object_type == 'mpoint':

            AddMovingPointToLayerDialog(qgis_interface=self.QgsInterface,
                                        object_name=object_name,
                                        relation_tuples=selected_data,
                                        relation_fields=None,
                                        geometry_type=object_type,
                                        frames_per_second=frames_per_second)

        elif object_type == 'mregion':

            AddMovingRegionToLayerDialog(qgis_interface=self.QgsInterface,
                                         object_name=object_name,
                                         relation_tuples=selected_data,
                                         relation_fields=None,
                                         geometry_type=object_type,
                                         frames_per_second=frames_per_second)

    def handle_add_selected_rows_to_layer(self):
        """
        Handles the transfer of the selected objects of a relation to a layer in QGIS.

        :return: None
        """

        spatial_types = ['point', 'points', 'line', 'region']
        geometry_type = None
        selected_data = []
        headers = []

        current_widget = self.stackedWidgetTab1.currentWidget()

        object_name = current_widget.ui.lineEditName.text()
        object_type = current_widget.ui.lineEditType.text()

        if object_type == 'rel':

            model = current_widget.ui.tableView.selectionModel().model()
            number_of_columns = current_widget.ui.tableView.selectionModel().model().columnCount()
            selected_rows = current_widget.ui.tableView.selectionModel().selectedRows()
            number_of_selected_rows = len(selected_rows)

            row_numbers = []
            for index in selected_rows:
                row_number = index.row()
                row_numbers.append(row_number)

            # With index, create a list of tuples with the selected data

            selected_data = []
            for selected_row in range(number_of_selected_rows):

                row_data = []
                for column in range(number_of_columns):

                    # The geometry data will be read from the item data for the UserRole. The data of the other
                    # attributes will be read from the DisplayRole as usual. The display value of the geometry data in
                    # the control won't be transferred.

                    current_index = model.index(row_numbers[selected_row], column)
                    geometry = current_index.data(Qt.UserRole)

                    if geometry is not None:
                        row_data.append(geometry)
                    else:
                        # Display Role data for other attributes
                        cell_data = current_index.data()
                        row_data.append(cell_data)

                selected_data.append(row_data)

            headers = []
            for column in range(number_of_columns):

                # Get type definition of the column

                header_item_data = model.horizontalHeaderItem(column).data()

                if header_item_data.attribute_type in spatial_types:
                    geometry_type = header_item_data.attribute_type

                headers.append(header_item_data)

        elif object_type == 'point' \
            or object_type == 'points' \
            or object_type == 'line' \
            or object_type == 'region':
            # or object_type == 'mpoint':

            # If object type is a spatial object and not a relation, create a pseudo-relation

            headers = []
            attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

            attribute.attribute_name = 'Name'
            attribute.attribute_type = 'string'

            headers.append(attribute)

            attribute = namedtuple('attribute', ['attribute_name', 'attribute_type'])

            attribute.attribute_name = 'GeoData'
            attribute.attribute_type = object_type

            headers.append(attribute)

            geometry_type = object_type

            model = QStandardItemModel()

            if object_type == 'point' or object_type == 'points' or object_type == 'line':
                model: QStandardItemModel = current_widget.ui.tableView.model()
            elif object_type == 'region':
                model: QStandardItemModel = current_widget.ui.treeView.model()

            root_item = model.invisibleRootItem()
            data_from_object = root_item.data(Qt.UserRole)

            selected_data = [[object_name, data_from_object]]

        # After acquiring data, call method to display the data in QGIS

        if len(selected_data) > 0:

            AddToLayerDialog(qgis_interface=self.QgsInterface,
                             object_name=object_name,
                             relation_tuples=selected_data,
                             relation_fields=headers,
                             geometry_type=geometry_type)

        else:
            self.handle_status_message("No entries selected. Please select one or more entries in the data viewer.")



