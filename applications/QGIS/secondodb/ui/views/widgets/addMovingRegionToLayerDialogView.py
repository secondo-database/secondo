# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'addMovingRegionToLayerDialogView.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(280, 112)
        Dialog.setMinimumSize(QtCore.QSize(280, 112))
        Dialog.setMaximumSize(QtCore.QSize(280, 112))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/objectmap.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Dialog.setWindowIcon(icon)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(Dialog)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.groupBox_2 = QtWidgets.QGroupBox(Dialog)
        self.groupBox_2.setObjectName("groupBox_2")
        self.formLayout = QtWidgets.QFormLayout(self.groupBox_2)
        self.formLayout.setObjectName("formLayout")
        self.labelLayerName = QtWidgets.QLabel(self.groupBox_2)
        self.labelLayerName.setEnabled(True)
        self.labelLayerName.setObjectName("labelLayerName")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.LabelRole, self.labelLayerName)
        self.lineEditLayerName = QtWidgets.QLineEdit(self.groupBox_2)
        self.lineEditLayerName.setMinimumSize(QtCore.QSize(0, 20))
        self.lineEditLayerName.setObjectName("lineEditLayerName")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.FieldRole, self.lineEditLayerName)
        self.verticalLayout_2.addWidget(self.groupBox_2)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.pushButtonAdd = QtWidgets.QPushButton(Dialog)
        self.pushButtonAdd.setObjectName("pushButtonAdd")
        self.horizontalLayout.addWidget(self.pushButtonAdd)
        self.pushButtonCancel = QtWidgets.QPushButton(Dialog)
        self.pushButtonCancel.setObjectName("pushButtonCancel")
        self.horizontalLayout.addWidget(self.pushButtonCancel)
        self.verticalLayout_2.addLayout(self.horizontalLayout)
        self.labelLayerName.setBuddy(self.lineEditLayerName)

        self.retranslateUi(Dialog)
        self.pushButtonCancel.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Add to layer"))
        self.groupBox_2.setTitle(_translate("Dialog", "Layer"))
        self.labelLayerName.setText(_translate("Dialog", "Layer name"))
        self.pushButtonAdd.setText(_translate("Dialog", "&Add"))
        self.pushButtonCancel.setText(_translate("Dialog", "&Cancel"))
