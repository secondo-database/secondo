# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'addDatabaseDialogView.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(180, 75)
        Dialog.setMinimumSize(QtCore.QSize(180, 75))
        Dialog.setMaximumSize(QtCore.QSize(180, 75))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/adddatabase.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Dialog.setWindowIcon(icon)
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(Dialog)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout()
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.verticalLayout = QtWidgets.QVBoxLayout()
        self.verticalLayout.setObjectName("verticalLayout")
        self.label = QtWidgets.QLabel(Dialog)
        self.label.setObjectName("label")
        self.verticalLayout.addWidget(self.label)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout()
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.databaseNameText = QtWidgets.QLineEdit(Dialog)
        self.databaseNameText.setObjectName("databaseNameText")
        self.verticalLayout_2.addWidget(self.databaseNameText)
        self.horizontalLayout.addLayout(self.verticalLayout_2)
        self.verticalLayout_3.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.addButton = QtWidgets.QPushButton(Dialog)
        self.addButton.setObjectName("addButton")
        self.horizontalLayout_2.addWidget(self.addButton)
        self.cancelButton = QtWidgets.QPushButton(Dialog)
        self.cancelButton.setObjectName("cancelButton")
        self.horizontalLayout_2.addWidget(self.cancelButton)
        self.verticalLayout_3.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3.addLayout(self.verticalLayout_3)
        self.label.setBuddy(self.databaseNameText)

        self.retranslateUi(Dialog)
        self.cancelButton.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Add database"))
        self.label.setText(_translate("Dialog", "Name"))
        self.addButton.setText(_translate("Dialog", "&Add"))
        self.cancelButton.setText(_translate("Dialog", "C&ancel"))
