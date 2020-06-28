from secondodb.ui.views.mainDialog import MainDialog
from PyQt5 import QtWidgets


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)

    mainDialogController = MainDialog()
    mainDialogController.MainDialog.exec_()

    sys.exit(app.exec_())