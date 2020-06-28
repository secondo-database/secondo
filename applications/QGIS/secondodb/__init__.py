# -*- coding: utf-8 -*-

import os
# from qgis.PyQt.QtCore import QDir
from PyQt5.QtCore import QDir

def classFactory(iface):  # pylint: disable=invalid-name
    """Load SecondoDB class from file SecondoDB.

    :param iface: A QGIS interface instance.
    :type iface: QgsInterface
    """
    #
    QDir.addSearchPath("secondodb", os.path.dirname(__file__))

    from .SecondoDB import SecondoDB
    return SecondoDB(iface)
