# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'typeWidget.ui'
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
        self.lineEditSignature = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditSignature.setReadOnly(True)
        self.lineEditSignature.setObjectName("lineEditSignature")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.lineEditSignature)
        self.labelExampleTypeList = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelExampleTypeList.setObjectName("labelExampleTypeList")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.LabelRole, self.labelExampleTypeList)
        self.lineEditExampleTypeList = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditExampleTypeList.setReadOnly(True)
        self.lineEditExampleTypeList.setObjectName("lineEditExampleTypeList")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.FieldRole, self.lineEditExampleTypeList)
        self.labelListRepresentation = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelListRepresentation.setObjectName("labelListRepresentation")
        self.formLayout.setWidget(3, QtWidgets.QFormLayout.LabelRole, self.labelListRepresentation)
        self.lineEditListRepresentation = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditListRepresentation.setReadOnly(True)
        self.lineEditListRepresentation.setObjectName("lineEditListRepresentation")
        self.formLayout.setWidget(3, QtWidgets.QFormLayout.FieldRole, self.lineEditListRepresentation)
        self.labelExampleList = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelExampleList.setObjectName("labelExampleList")
        self.formLayout.setWidget(4, QtWidgets.QFormLayout.LabelRole, self.labelExampleList)
        self.lineEditExampleList = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditExampleList.setReadOnly(True)
        self.lineEditExampleList.setObjectName("lineEditExampleList")
        self.formLayout.setWidget(4, QtWidgets.QFormLayout.FieldRole, self.lineEditExampleList)
        self.labelRemarks = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelRemarks.setObjectName("labelRemarks")
        self.formLayout.setWidget(5, QtWidgets.QFormLayout.LabelRole, self.labelRemarks)
        self.plainTextEditRemarks = QtWidgets.QPlainTextEdit(self.groupBoxTypeConstructor)
        self.plainTextEditRemarks.setReadOnly(True)
        self.plainTextEditRemarks.setObjectName("plainTextEditRemarks")
        self.formLayout.setWidget(5, QtWidgets.QFormLayout.FieldRole, self.plainTextEditRemarks)
        self.verticalLayout.addWidget(self.groupBoxTypeConstructor)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.groupBoxTypeConstructor.setTitle(_translate("Form", "Type Constructor"))
        self.labelName.setText(_translate("Form", "Name"))
        self.labelSignature.setText(_translate("Form", "Signature"))
        self.labelExampleTypeList.setText(_translate("Form", "Example Type List"))
        self.labelListRepresentation.setText(_translate("Form", "List Representation"))
        self.labelExampleList.setText(_translate("Form", "Example List"))
        self.labelRemarks.setText(_translate("Form", "Remarks"))

