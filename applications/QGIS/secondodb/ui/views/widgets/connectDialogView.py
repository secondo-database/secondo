# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'connectDialogView.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(186, 100)
        Dialog.setMinimumSize(QtCore.QSize(186, 100))
        Dialog.setMaximumSize(QtCore.QSize(186, 101))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/connectserver.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
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
        self.label_2 = QtWidgets.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.verticalLayout.addWidget(self.label_2)
        self.horizontalLayout.addLayout(self.verticalLayout)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout()
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.hostText = QtWidgets.QLineEdit(Dialog)
        self.hostText.setObjectName("hostText")
        self.verticalLayout_2.addWidget(self.hostText)
        self.portText = QtWidgets.QLineEdit(Dialog)
        self.portText.setObjectName("portText")
        self.verticalLayout_2.addWidget(self.portText)
        self.horizontalLayout.addLayout(self.verticalLayout_2)
        self.verticalLayout_3.addLayout(self.horizontalLayout)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        self.connectButton = QtWidgets.QPushButton(Dialog)
        self.connectButton.setObjectName("connectButton")
        self.horizontalLayout_2.addWidget(self.connectButton)
        self.cancelButton = QtWidgets.QPushButton(Dialog)
        self.cancelButton.setObjectName("cancelButton")
        self.horizontalLayout_2.addWidget(self.cancelButton)
        self.verticalLayout_3.addLayout(self.horizontalLayout_2)
        self.horizontalLayout_3.addLayout(self.verticalLayout_3)
        self.label.setBuddy(self.hostText)
        self.label_2.setBuddy(self.portText)

        self.retranslateUi(Dialog)
        self.cancelButton.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Connect to Secondo Server"))
        self.label.setText(_translate("Dialog", "&Host"))
        self.label_2.setText(_translate("Dialog", "&Port"))
        self.connectButton.setText(_translate("Dialog", "&Connect"))
        self.cancelButton.setText(_translate("Dialog", "C&ancel"))

