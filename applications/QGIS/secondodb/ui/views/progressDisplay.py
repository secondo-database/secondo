from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QDialog

from secondodb.ui.views.widgets.progressView import Ui_Dialog
import secondodb.ui.models.importFeaturesFromQGISDialogModel as importqgis


class ProgressDisplay(QDialog):
    """
    This class implements the view of the progress display dialog.
    """

    def __init__(self, import_qgis_model=None):
        """
        Constructor of the class.
        """

        super().__init__()

        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.model: importqgis.ImportFeaturesFromQGISDialogModel = import_qgis_model

        # Set slots

        self.model.signalProgress.connect(self.handle_set_progress)

        self.exec_()

    def handle_set_progress(self, progress_value: float) -> None:
        """
        Handles the display of the current progress in the dialog.

        :param progress_value: The percentage completed.
        :return: None
        """
        self.ui.progressBar.setValue(progress_value)

