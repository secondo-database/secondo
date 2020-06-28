# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Settings Dialog Model
# settingsDialogModel.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class SettingsDialogModel, which implements the data model of the settings dialog. The class
uses the QgsSettings object of the active instance of QGIS to save the parameters.
"""

from qgis._core import QgsSettings


class SettingsDialogModel:
    """
    This class implements the model of the settings dialog.
    """

    def __init__(self):
        """
        Constructor of the class.
        """
        self.hostname: str = ''
        self.port: str = ''
        self.loadTypes: int = 0
        self.loadAlgebras: int = 0
        # self.maxEntries: int = 0
        self.frames_per_second: int = 1
        self.load_parameters()

    def load_parameters(self) -> None:
        """
        Loads the parameters from the settings object of QGIS.

        :return: None
        """

        s = QgsSettings()

        hostname = s.value("secondodb/hostname")

        if hostname is not None:
            self.hostname = hostname
        else:
            self.hostname = '127.0.0.1'

        port = s.value("secondodb/port")

        if port is not None:
            self.port = port
        else:
            self.port = '1234'

        loadTypes = s.value("secondodb/loadtypes")

        if loadTypes is not None:
            self.loadTypes = loadTypes
        else:
            self.loadTypes = 0

        loadAlgebras = s.value("secondodb/loadalgebras")

        if loadAlgebras is not None:
            self.loadAlgebras = loadAlgebras
        else:
            self.loadAlgebras = 0

        # self.maxEntries = s.value("secondodb/maxentries")

        frames_per_second = s.value("secondodb/framespersecond")

        if frames_per_second is not None:
            self.frames_per_second = frames_per_second
        else:
            self.frames_per_second = 1

    def save_parameters(self) -> None:
        """
        Save the parameters to the settings object of QGIS.

        :return: None
        """

        s = QgsSettings()
        s.setValue("secondodb/hostname", self.hostname)
        s.setValue("secondodb/port", self.port)
        s.setValue("secondodb/loadtypes", self.loadTypes)
        s.setValue("secondodb/loadalgebras", self.loadAlgebras)
        # s.setValue("secondodb/maxentries", self.maxEntries)
        s.setValue("secondodb/framespersecond", self.frames_per_second)

