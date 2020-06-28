# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'spatialRegionWidget.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")
        Form.resize(467, 377)
        self.verticalLayout = QtWidgets.QVBoxLayout(Form)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBoxTypeConstructor = QtWidgets.QGroupBox(Form)
        self.groupBoxTypeConstructor.setObjectName("groupBoxTypeConstructor")
        self.formLayout = QtWidgets.QFormLayout(self.groupBoxTypeConstructor)
        self.formLayout.setObjectName("formLayout")
        self.labelName = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelName.setObjectName("labelName")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.LabelRole, self.labelName)
        self.lineEditName = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditName.setReadOnly(True)
        self.lineEditName.setObjectName("lineEditName")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.FieldRole, self.lineEditName)
        self.labelSignature = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelSignature.setObjectName("labelSignature")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.LabelRole, self.labelSignature)
        self.lineEditType = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditType.setReadOnly(True)
        self.lineEditType.setObjectName("lineEditType")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.lineEditType)
        self.verticalLayout.addWidget(self.groupBoxTypeConstructor)
        self.groupBox = QtWidgets.QGroupBox(Form)
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.treeView = QtWidgets.QTreeView(self.groupBox)
        self.treeView.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
        self.treeView.setObjectName("treeView")
        self.verticalLayout_2.addWidget(self.treeView)
        self.frame = QtWidgets.QFrame(self.groupBox)
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.frame)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pushButtonAddToLayer = QtWidgets.QPushButton(self.frame)
        self.pushButtonAddToLayer.setObjectName("pushButtonAddToLayer")
        self.horizontalLayout.addWidget(self.pushButtonAddToLayer)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout_2.addWidget(self.frame)
        self.verticalLayout.addWidget(self.groupBox)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.groupBoxTypeConstructor.setTitle(_translate("Form", "Spatial Object"))
        self.labelName.setText(_translate("Form", "Name"))
        self.labelSignature.setText(_translate("Form", "Type"))
        self.groupBox.setTitle(_translate("Form", "Data Viewer"))
        self.pushButtonAddToLayer.setText(_translate("Form", "Add to layer"))

