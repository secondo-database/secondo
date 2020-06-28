# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'addToLayerDialogView.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(350, 180)
        Dialog.setMinimumSize(QtCore.QSize(350, 180))
        Dialog.setMaximumSize(QtCore.QSize(350, 180))
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/objectmap.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Dialog.setWindowIcon(icon)
        self.gridLayout_3 = QtWidgets.QGridLayout(Dialog)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.groupBox = QtWidgets.QGroupBox(Dialog)
        self.groupBox.setMinimumSize(QtCore.QSize(0, 60))
        self.groupBox.setMaximumSize(QtCore.QSize(400, 125))
        self.groupBox.setObjectName("groupBox")
        self.gridLayout = QtWidgets.QGridLayout(self.groupBox)
        self.gridLayout.setObjectName("gridLayout")
        self.lineEditLayerName = QtWidgets.QLineEdit(self.groupBox)
        self.lineEditLayerName.setObjectName("lineEditLayerName")
        self.gridLayout.addWidget(self.lineEditLayerName, 0, 1, 1, 1)
        self.labelLayerName = QtWidgets.QLabel(self.groupBox)
        self.labelLayerName.setEnabled(True)
        self.labelLayerName.setObjectName("labelLayerName")
        self.gridLayout.addWidget(self.labelLayerName, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.groupBox, 0, 0, 1, 1)
        self.groupBox_2 = QtWidgets.QGroupBox(Dialog)
        self.groupBox_2.setMinimumSize(QtCore.QSize(0, 60))
        self.groupBox_2.setObjectName("groupBox_2")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.groupBox_2)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.checkBoxAddAsPolyline = QtWidgets.QCheckBox(self.groupBox_2)
        self.checkBoxAddAsPolyline.setObjectName("checkBoxAddAsPolyline")
        self.gridLayout_2.addWidget(self.checkBoxAddAsPolyline, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.groupBox_2, 1, 0, 1, 1)
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
        self.gridLayout_3.addLayout(self.horizontalLayout, 2, 0, 1, 1)
        self.labelLayerName.setBuddy(self.lineEditLayerName)

        self.retranslateUi(Dialog)
        self.pushButtonCancel.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Add to layer"))
        self.groupBox.setTitle(_translate("Dialog", "Layer"))
        self.labelLayerName.setText(_translate("Dialog", "Layer name"))
        self.groupBox_2.setTitle(_translate("Dialog", "Options"))
        self.checkBoxAddAsPolyline.setText(_translate("Dialog", "Add as a single polyline"))
        self.pushButtonAdd.setText(_translate("Dialog", "&Add"))
        self.pushButtonCancel.setText(_translate("Dialog", "&Cancel"))

