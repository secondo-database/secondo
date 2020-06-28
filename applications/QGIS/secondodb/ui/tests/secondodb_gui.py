from secondodb.ui.views import Ui_MainWindow
from secondodb.ui.views import Ui_Dialog
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QStandardItemModel
from PyQt5.QtGui import QStandardItem
from PyQt5 import QtGui, QtWidgets
import secondodb.api.secondoapi as secondo


def connect_to_secondo(host, port, connectDialog):
    ui.statusbar.showMessage('Connecting to Secondo Server on ' + host + '/' + port + '...')
    ui.statusbar.repaint()
    connectDialog.close()
    conn = secondo.connect(host, port)
    ui.statusbar.showMessage('Connection succesfully!')
    model = create_model_from_secondo(conn)
    populate_tree(model)
    # return conn

def populate_tree(model):
    ui.treeView.setModel(model)


def create_model_from_secondo(connection):

    icon_db = QtGui.QIcon()
    icon_db.addPixmap(QtGui.QPixmap("../resources/database.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)

    icon_folder = QtGui.QIcon()
    icon_folder.addPixmap(QtGui.QPixmap("../resources/folder.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)

    icon_algebra = QtGui.QIcon()
    icon_algebra.addPixmap(QtGui.QPixmap("../resources/algebra.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)

    model = QStandardItemModel()

    databases = connection.get_list_databases()
    counter = 0

    for dbname in databases:
        if dbname == 'BERLINTEST':
            item = QStandardItem(dbname)
            item.setIcon(icon_db)

            algebra_folder = QStandardItem('Algebras')
            algebra_folder.setIcon(icon_folder)

            list_algebras = connection.get_list_algebras()

            alg_counter = 0

            for algebra_name in list_algebras:
                algebra = QStandardItem(algebra_name)
                algebra.setIcon(icon_algebra)
                algebra_folder.setChild(alg_counter, 0, algebra)
                alg_counter += 1

            item.setChild(0, 0, algebra_folder)

            model.setItem(counter, 0, item)
            counter += 1
        else:
            item = QStandardItem(dbname)
            item.setIcon(icon_db)
            model.setItem(counter, 0, item)
            counter += 1

    model.setHorizontalHeaderLabels(["Data Sources"])

    return model


def open_connect_dialog():
    dialog = QDialog()
    dialog.ui = Ui_Dialog()
    dialog.ui.setupUi(dialog)
    dialog.ui.hostText.setText('127.0.0.1')
    dialog.ui.portText.setText('1234')
    host = dialog.ui.hostText.text()
    port = dialog.ui.portText.text()
    dialog.ui.connectButton.clicked.connect(lambda: connect_to_secondo(host, port, dialog))
    # dialog.exec_()
    dialog.show()

if __name__ == "__main__":

    # connection = connect_to_secondo()

    import sys
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    ui.splitter.setStretchFactor(1, 2)
    # populate_tree()
    # ui.cancelButton.clicked.connect(Dialog.close)
    ui.actionConnectToSecondoServer.triggered.connect(open_connect_dialog)
    MainWindow.show()
    sys.exit(app.exec_())