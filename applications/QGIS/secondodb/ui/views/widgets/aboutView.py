# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'aboutView.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Dialog(object):
    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(438, 263)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(2)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(Dialog.sizePolicy().hasHeightForWidth())
        Dialog.setSizePolicy(sizePolicy)
        Dialog.setMinimumSize(QtCore.QSize(400, 250))
        Dialog.setMaximumSize(QtCore.QSize(438, 263))
        Dialog.setContextMenuPolicy(QtCore.Qt.NoContextMenu)
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/icons/mainwindow.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Dialog.setWindowIcon(icon)
        Dialog.setModal(False)
        self.gridLayout = QtWidgets.QGridLayout(Dialog)
        self.gridLayout.setObjectName("gridLayout")
        self.label = QtWidgets.QLabel(Dialog)
        self.label.setContextMenuPolicy(QtCore.Qt.DefaultContextMenu)
        self.label.setObjectName("label")
        self.gridLayout.addWidget(self.label, 0, 3, 1, 1)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.pushButtonSave = QtWidgets.QPushButton(Dialog)
        self.pushButtonSave.setObjectName("pushButtonSave")
        self.horizontalLayout.addWidget(self.pushButtonSave)
        self.gridLayout.addLayout(self.horizontalLayout, 1, 3, 1, 1)
        self.label_2 = QtWidgets.QLabel(Dialog)
        self.label_2.setObjectName("label_2")
        self.gridLayout.addWidget(self.label_2, 0, 1, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(10, 20, QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem1, 0, 2, 1, 1)
        spacerItem2 = QtWidgets.QSpacerItem(10, 20, QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem2, 0, 0, 1, 1)

        self.retranslateUi(Dialog)
        self.pushButtonSave.clicked.connect(Dialog.close)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "About SecondoDB for QGIS"))
        self.label.setText(_translate("Dialog", "<html><head/><body><p><span style=\" font-size:12pt; font-weight:600;\">SecondoDB Plugin for QGIS</span><br/><br/>Victor Silva<br/>Master Thesis Project - MSc. Practical Computer Science<br/><br/>Copyright (C) 2008-2020<br/>University of Hagen<br/>Faculty of Mathematics and Computer Science<br/>Database Systems for New Applications<br/><br/>Blue UI icons by <a href=\"https://icons8.de/\"><span style=\" text-decoration: underline; color:#0000ff;\">Icons8</span></a><br/><br/>Disclaimer: This is free software, see the source for copying conditions.<br/>There is NO warranty, not even for MERCHANTABILITY or FITNESS<br/>FOR A PARTICULAR PURPOSE.<br/></p></body></html>"))
        self.pushButtonSave.setText(_translate("Dialog", "&Ok"))
        self.label_2.setText(_translate("Dialog", "<html><head/><body><p><img src=\":/icons/mainwindow.png\"/></p></body></html>"))

