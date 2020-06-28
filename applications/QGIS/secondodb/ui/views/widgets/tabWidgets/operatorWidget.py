# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'operatorWidget.ui'
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
        self.plainTextEditSignature = QtWidgets.QPlainTextEdit(self.groupBoxTypeConstructor)
        self.plainTextEditSignature.setMaximumSize(QtCore.QSize(16777215, 60))
        self.plainTextEditSignature.setReadOnly(True)
        self.plainTextEditSignature.setObjectName("plainTextEditSignature")
        self.formLayout.setWidget(1, QtWidgets.QFormLayout.FieldRole, self.plainTextEditSignature)
        self.labelExampleTypeList = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelExampleTypeList.setObjectName("labelExampleTypeList")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.LabelRole, self.labelExampleTypeList)
        self.lineEditSyntax = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditSyntax.setReadOnly(True)
        self.lineEditSyntax.setObjectName("lineEditSyntax")
        self.formLayout.setWidget(2, QtWidgets.QFormLayout.FieldRole, self.lineEditSyntax)
        self.labelListRepresentation = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelListRepresentation.setObjectName("labelListRepresentation")
        self.formLayout.setWidget(3, QtWidgets.QFormLayout.LabelRole, self.labelListRepresentation)
        self.plainTextEditMeaning = QtWidgets.QPlainTextEdit(self.groupBoxTypeConstructor)
        self.plainTextEditMeaning.setMaximumSize(QtCore.QSize(16777215, 100))
        self.plainTextEditMeaning.setReadOnly(True)
        self.plainTextEditMeaning.setObjectName("plainTextEditMeaning")
        self.formLayout.setWidget(3, QtWidgets.QFormLayout.FieldRole, self.plainTextEditMeaning)
        self.labelExampleList = QtWidgets.QLabel(self.groupBoxTypeConstructor)
        self.labelExampleList.setObjectName("labelExampleList")
        self.formLayout.setWidget(4, QtWidgets.QFormLayout.LabelRole, self.labelExampleList)
        self.lineEditExample = QtWidgets.QLineEdit(self.groupBoxTypeConstructor)
        self.lineEditExample.setReadOnly(True)
        self.lineEditExample.setObjectName("lineEditExample")
        self.formLayout.setWidget(4, QtWidgets.QFormLayout.FieldRole, self.lineEditExample)
        self.verticalLayout.addWidget(self.groupBoxTypeConstructor)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.groupBoxTypeConstructor.setTitle(_translate("Form", "Operator"))
        self.labelName.setText(_translate("Form", "Name"))
        self.labelSignature.setText(_translate("Form", "Signature"))
        self.labelExampleTypeList.setText(_translate("Form", "Syntax"))
        self.labelListRepresentation.setText(_translate("Form", "Meaning"))
        self.labelExampleList.setText(_translate("Form", "Example"))

