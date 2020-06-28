# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Query Dialog
# queryDialog.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AboutDialog, which implements the view of the about dialog.
"""
from PyQt5 import QtGui
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QDialog, QMessageBox


from secondodb.ui.views.widgets.queryDialogView import Ui_Dialog
import secondodb.api.secondoapi as api

SUPPORTED_TYPES = ['point', 'points', 'line', 'region', 'mpoint', 'mregion', 'rel']


class QueryDialog(QDialog):
    """
    This class implements the view of the about dialog.
    """

    def __init__(self, secondo_connection: api.Connection):
        """
        Constructor of the class.
        """

        super().__init__()

        # Initialize View

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.connection = secondo_connection
        self.setWindowFlag(Qt.WindowContextHelpButtonHint, False)
        self.ui.pushButtonExecute.clicked.connect(self.handle_execute_query)

        self.ui.pushButtonSaveToDatabase.setDisabled(True)
        self.ui.lineEditObjectName.setDisabled(True)
        self.ui.pushButtonSaveToDatabase.clicked.connect(self.handle_save_to_database)

        self.ui.lineEditCurrentDatabase.setText(self.connection.database)
        self.exec_()

    def handle_save_to_database(self):
        """
        Handles the saving of the current query to the database. In the current implementation of the plugin, the query
        will be performed again using the let command of the |sec| system.

        :return: None
        """

        if self.ui.lineEditObjectName.text() != '':

            query_string = self.ui.plainTextEditQueryEditor.toPlainText()

            let_str = query_string.replace('query ', '')

            try:
                cursor = self.connection.cursor()
            except api.OperationalError as e:
                pass
            else:
                try:
                    cursor.execute_let(self.ui.lineEditObjectName.text(), let_str)
                except api.InternalError:
                    pass
                except api.ProgrammingError as e:
                    icon_addobject = QtGui.QIcon()
                    icon_addobject.addPixmap(QtGui.QPixmap(":/icons/addobject.png"),
                                             QtGui.QIcon.Normal,
                                             QtGui.QIcon.Off)
                    message = QMessageBox()
                    message.setIcon(QMessageBox.Information)
                    message.setWindowTitle('Add a new object from a query')
                    message.setWindowIcon(icon_addobject)
                    message.setText(e.message)
                    message.setStandardButtons(QMessageBox.Ok)
                    message.exec_()
                except api.OperationalError:
                    icon_addobject = QtGui.QIcon()
                    icon_addobject.addPixmap(QtGui.QPixmap(":/icons/addobject.png"),
                                             QtGui.QIcon.Normal,
                                             QtGui.QIcon.Off)
                    message = QMessageBox()
                    message.setIcon(QMessageBox.Information)
                    message.setWindowTitle('Add a new object from a query')
                    message.setWindowIcon(icon_addobject)
                    message.setText('Please enter a valid object name')
                    message.setStandardButtons(QMessageBox.Ok)
                    message.exec_()
                else:
                    icon_addobject = QtGui.QIcon()
                    icon_addobject.addPixmap(QtGui.QPixmap(":/icons/addobject.png"),
                                             QtGui.QIcon.Normal,
                                             QtGui.QIcon.Off)
                    message = QMessageBox()
                    message.setIcon(QMessageBox.Information)
                    message.setWindowTitle('Add a new object from a query')
                    message.setWindowIcon(icon_addobject)
                    message.setText('Object saved to database ' + self.ui.lineEditCurrentDatabase.text()
                                    + ' successfully.')
                    message.setStandardButtons(QMessageBox.Ok)
                    message.exec_()
        else:
            icon_addobject = QtGui.QIcon()
            icon_addobject.addPixmap(QtGui.QPixmap(":/icons/addobject.png"),
                                     QtGui.QIcon.Normal,
                                     QtGui.QIcon.Off)
            message = QMessageBox()
            message.setIcon(QMessageBox.Information)
            message.setWindowTitle('Add a new object from a query')
            message.setWindowIcon(icon_addobject)
            message.setText('Please enter an object name')
            message.setStandardButtons(QMessageBox.Ok)
            message.exec_()

    def handle_execute_query(self):
        """
        Handles the execution of a query.

        :return: None
        """

        query_string = self.ui.plainTextEditQueryEditor.toPlainText()

        if 'query' in query_string:  # Simple check of query validity

            if self.connection.initialized:
                if not self.connection.server_mode_only:
                    cursor = self.connection.cursor()
                    try:
                        results, object_type, listexp_str = cursor.execute(query_string)
                    except api.ProgrammingError as e:
                        self.ui.plainTextEditQueryResults.setPlainText(e.args[0])
                    except api.InterfaceError as e:
                        self.ui.plainTextEditQueryResults.setPlainText(e.args[0])
                    else:
                        self.ui.plainTextEditQueryResults.setPlainText(listexp_str)

                        if object_type in SUPPORTED_TYPES:
                            self.ui.pushButtonSaveToDatabase.setDisabled(False)
                            self.ui.lineEditObjectName.setDisabled(False)
                            self.ui.lineEditObjectName.setPlaceholderText('Enter name for new object...')
                        else:
                            self.ui.pushButtonSaveToDatabase.setDisabled(True)
                            self.ui.lineEditObjectName.setDisabled(True)
                            self.ui.lineEditObjectName.setPlaceholderText('')

                else:
                    print("Connected on server mode, no queries possible.")
            else:
                print("The connection is not initialized.")

        else:
            self.ui.plainTextEditQueryResults.setPlainText("Invalid query.")






