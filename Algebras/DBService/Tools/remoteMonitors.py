#!/usr/bin/env python
import logging
import os
from env import UnsuitableEnvironmentException

class RemoteMonitors():

    def __init__(self, logger):
        self.logger = logger
        self.platform = self._getEnvironmentVariable("SECONDO_PLATFORM")
        self.buildDir = self._getEnvironmentVariable("SECONDO_BUILD_DIR")
        self.secondoSDK = self._getEnvironmentVariable("SECONDO_SDK")

    def _getEnvironmentVariable(self, key):
        value = os.getenv(key, None)
        if value is None:
            raise UnsuitableEnvironmentException("Environment variable %s is not set" % key)
        return value

if __name__=="__main__":
    logger = logging.getLogger(__name__)
    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.INFO)
    logger.info("Checking configuration.")
    remoteMonitors = RemoteMonitors(logger)