#!/usr/bin/env python
import logging
import os

class RemoteMonitors():

    def __init__(self, logger):
        self.logger = logger
        self.platform = os.getenv("SECONDO_PLATFORM", None)
        if self.platform is None:
            self.logger.error("SECONDO_PLATFORM is not set.")
        self.buildDir = os.getenv("SECONDO_BUILD_DIR", None)
        if self.buildDir is None:
            self.logger.error("SECONDO_BUILD_DIR is not set.")
        self.secondoSDK = os.getenv("SECONDO_SDK", None)
        if self.secondoSDK is None:
            self.logger.error("SECONDO_SDK is not set.")

if __name__=="__main__":
    logger = logging.getLogger(__name__)
    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.INFO)
    logger.info("Checking configuration.")
    remoteMonitors = RemoteMonitors(logger)