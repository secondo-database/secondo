#!/usr/bin/env python
import logging
import os
from env import UnsuitableEnvironmentException
from optparse import OptionParser
#import subprocess
#import shlex

class RemoteMonitors():

    def __init__(self, logger):
        self.logger = logger
        self.platform = self.__getEnvironmentVariable("SECONDO_PLATFORM")
        self.buildDir = self.__getEnvironmentVariable("SECONDO_BUILD_DIR")
        self.secondoSDK = self.__getEnvironmentVariable("SECONDO_SDK")

    def __getEnvironmentVariable(self, key):
        value = os.getenv(key, None)
        if value is None:
            raise UnsuitableEnvironmentException("Environment variable %s is not set" % key)
        return value

    def run(self, options, args):
        if not args or len(args) != 1:
            raise ValueError("Call with exactly one action: [start|stop|check]")
        action = args[0]
        if action == "start":
            self.start()
        elif action == "check":
            self.check()
        elif action == "stop":
            self.stop()
        else:
            raise ValueError("Action must be one of [start|stop|check]")

    def start(self):
        pass

    def check(self):
        pass

    def stop(self):
        pass

if __name__=="__main__":
    logger = logging.getLogger(__name__)
    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.INFO)
    logger.info("Checking configuration.")
    remoteMonitors = RemoteMonitors(logger)
    parser = OptionParser(usage="%prog [start|stop|check]")
    parser.add_option("-c", "--configFile", dest="filename",
                  help="use configuration from specified file", metavar="FILE")

    (options, args) = parser.parse_args()
    remoteMonitors.run(options, args)