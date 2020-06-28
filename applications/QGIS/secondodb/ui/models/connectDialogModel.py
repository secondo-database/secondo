# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Connect Dialog Model
# connectDialogModel.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class ConnectDialogModel, which implements the data model of the connect to |sec| server
dialog.
"""


class ConnectDialogModel:
    """
    This class implements the data model of the connect to |sec| server dialog.
    """

    def __init__(self, host: str, port: str):
        """
        Constructor of the class.
        """
        self.host = host
        self.port = port

        # Set default values if the parameters couldn't be read from QGIS

        if host is None and port is None:
            self.host = '127.0.0.1'
            self.port = '1234'

    def get_host(self) -> str:
        """
        Gets the host name of the connection.

        :return: A string with the host.
        """
        return self.host

    def get_port(self) -> str:
        """
        Gets the port of the connection.

        :return: A string with the port.
        """
        return self.port

    def set_host(self, host: str) -> None:
        """
        Sets the host of the connection.

        :param host: A string with the host.
        :return: None
        """
        self.host = host

    def set_port(self, port: str) -> None:
        """
        Sets the port of the connection.

        :param port: A string with the port.
        :return: None
        """
        self.port = port
