# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'createRelationWidget.ui'
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
        self.lineEditName.setObjectName("lineEditName")
        self.formLayout.setWidget(0, QtWidgets.QFormLayout.FieldRole, self.lineEditName)
        self.verticalLayout.addWidget(self.groupBoxTypeConstructor)
        self.groupBox = QtWidgets.QGroupBox(Form)
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.frame_2 = QtWidgets.QFrame(self.groupBox)
        self.frame_2.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_2.setObjectName("frame_2")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout(self.frame_2)
        self.horizontalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.pushButtonAddAttribute = QtWidgets.QPushButton(self.frame_2)
        self.pushButtonAddAttribute.setObjectName("pushButtonAddAttribute")
        self.horizontalLayout_3.addWidget(self.pushButtonAddAttribute)
        self.pushButtonDeleteAttribute = QtWidgets.QPushButton(self.frame_2)
        self.pushButtonDeleteAttribute.setObjectName("pushButtonDeleteAttribute")
        self.horizontalLayout_3.addWidget(self.pushButtonDeleteAttribute)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_3.addItem(spacerItem)
        self.verticalLayout_2.addWidget(self.frame_2)
        self.tableView = QtWidgets.QTableView(self.groupBox)
        self.tableView.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
        self.tableView.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.tableView.setObjectName("tableView")
        self.verticalLayout_2.addWidget(self.tableView)
        self.frame = QtWidgets.QFrame(self.groupBox)
        self.frame.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.frame)
        self.horizontalLayout.setContentsMargins(9, 9, 9, 9)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.pushButtonCreate = QtWidgets.QPushButton(self.frame)
        self.pushButtonCreate.setObjectName("pushButtonCreate")
        self.horizontalLayout.addWidget(self.pushButtonCreate)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem1)
        self.verticalLayout_2.addWidget(self.frame)
        self.verticalLayout.addWidget(self.groupBox)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.groupBoxTypeConstructor.setTitle(_translate("Form", "Relation Name"))
        self.labelName.setText(_translate("Form", "Name"))
        self.groupBox.setTitle(_translate("Form", "Attributes"))
        self.pushButtonAddAttribute.setText(_translate("Form", "Add attribute"))
        self.pushButtonDeleteAttribute.setText(_translate("Form", "Delete attribute"))
        self.pushButtonCreate.setText(_translate("Form", "Create"))

