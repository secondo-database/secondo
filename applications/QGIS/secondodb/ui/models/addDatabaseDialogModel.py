# ----------------------------------------------------------------------------------------------------------------------
# SecondoDB Plugin for QGIS
# Victor Silva (victor.silva@posteo.de)
# May 2020
# ----------------------------------------------------------------------------------------------------------------------
# Module
# ----------------------------------------------------------------------------------------------------------------------
# Add Database Dialog Model
# addDatabaseDialogModel.py
# ----------------------------------------------------------------------------------------------------------------------
"""
This module contains the class AddDatabaseDialogModel, which implements the data model of the add database dialog.
"""


class AddDatabaseDialogModel:
    """
    This class implements the data model of the add database dialog.
    """

    def __init__(self):
        """
        Constructor of the class.
        """
        self.databaseName = ''

    def set_database_name(self, database_name: str) -> None:
        """
        Sets the name of the database in the model.

        :param database_name: A string with the name of the database.
        :return: None
        """
        self.databaseName = database_name

    def get_database_name(self) -> str:
        """
        Gets the name of the database in the model.

        :return: A string with the name of the database.
        """
        return self.databaseName

