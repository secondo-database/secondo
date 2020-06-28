# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'queryDialogView.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(600, 600)
        Dialog.setMinimumSize(QtCore.QSize(600, 600))
        Dialog.setMaximumSize(QtCore.QSize(600, 600))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/query.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Dialog.setWindowIcon(icon)
        self.gridLayout = QtWidgets.QGridLayout(Dialog)
        self.gridLayout.setObjectName("gridLayout")
        self.groupBoxFeaturesViewer = QtWidgets.QGroupBox(Dialog)
        self.groupBoxFeaturesViewer.setObjectName("groupBoxFeaturesViewer")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.groupBoxFeaturesViewer)
        self.verticalLayout.setObjectName("verticalLayout")
        self.plainTextEditQueryEditor = QtWidgets.QPlainTextEdit(self.groupBoxFeaturesViewer)
        self.plainTextEditQueryEditor.setObjectName("plainTextEditQueryEditor")
        self.verticalLayout.addWidget(self.plainTextEditQueryEditor)
        self.gridLayout.addWidget(self.groupBoxFeaturesViewer, 4, 0, 1, 1)
        self.groupBox = QtWidgets.QGroupBox(Dialog)
        self.groupBox.setObjectName("groupBox")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.pushButtonSaveToDatabase = QtWidgets.QPushButton(self.groupBox)
        self.pushButtonSaveToDatabase.setObjectName("pushButtonSaveToDatabase")
        self.horizontalLayout_3.addWidget(self.pushButtonSaveToDatabase)
        self.lineEditObjectName = QtWidgets.QLineEdit(self.groupBox)
        self.lineEditObjectName.setObjectName("lineEditObjectName")
        self.horizontalLayout_3.addWidget(self.lineEditObjectName)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem)
        self.gridLayout_2.addLayout(self.horizontalLayout_3, 3, 0, 1, 1)
        self.plainTextEditQueryResults = QtWidgets.QPlainTextEdit(self.groupBox)
        self.plainTextEditQueryResults.setReadOnly(True)
        self.plainTextEditQueryResults.setObjectName("plainTextEditQueryResults")
        self.gridLayout_2.addWidget(self.plainTextEditQueryResults, 0, 0, 1, 1)
        self.gridLayout.addWidget(self.groupBox, 5, 0, 1, 1)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.pushButtonExecute = QtWidgets.QPushButton(Dialog)
        self.pushButtonExecute.setObjectName("pushButtonExecute")
        self.horizontalLayout.addWidget(self.pushButtonExecute)
        self.pushButtonCancel = QtWidgets.QPushButton(Dialog)
        self.pushButtonCancel.setObjectName("pushButtonCancel")
        self.horizontalLayout.addWidget(self.pushButtonCancel)
        self.gridLayout.addLayout(self.horizontalLayout, 6, 0, 1, 1)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.label = QtWidgets.QLabel(Dialog)
        self.label.setObjectName("label")
        self.horizontalLayout_2.addWidget(self.label)
        self.lineEditCurrentDatabase = QtWidgets.QLineEdit(Dialog)
        self.lineEditCurrentDatabase.setReadOnly(True)
        self.lineEditCurrentDatabase.setObjectName("lineEditCurrentDatabase")
        self.horizontalLayout_2.addWidget(self.lineEditCurrentDatabase)
        self.gridLayout.addLayout(self.horizontalLayout_2, 0, 0, 1, 1)

        self.retranslateUi(Dialog)
        self.pushButtonCancel.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Execute Query"))
        self.groupBoxFeaturesViewer.setTitle(_translate("Dialog", "Query Editor"))
        self.groupBox.setTitle(_translate("Dialog", "Query Results (List Expression)"))
        self.pushButtonSaveToDatabase.setText(_translate("Dialog", "&Save to Database"))
        self.pushButtonExecute.setText(_translate("Dialog", "&Execute"))
        self.pushButtonCancel.setText(_translate("Dialog", "&Cancel"))
        self.label.setText(_translate("Dialog", "Current Database"))
