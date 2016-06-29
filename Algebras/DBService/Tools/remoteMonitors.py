#!/usr/bin/env python
import logging
import os
from Exceptions import UnsuitableEnvironmentException
from optparse import OptionParser
from SecondoConfiguration import SecondoConfiguration
from Monitor import Monitor

class RemoteMonitors():

    def __init__(self, logger):
        self.logger = logger
        self.__checkEnvironment()

    def __getEnvironmentVariable(self, key):
        value = os.getenv(key, None)
        if value is None:
            raise UnsuitableEnvironmentException("Environment variable %s is not set" % key)
        return value

    def __checkEnvironment(self):
        self.platform = self.__getEnvironmentVariable("SECONDO_PLATFORM")
        self.buildDir = self.__getEnvironmentVariable("SECONDO_BUILD_DIR")
        self.secondoSDK = self.__getEnvironmentVariable("SECONDO_SDK")
        self.homeDir = self.__getEnvironmentVariable("HOME")

    def __getProgramOptionsAndArgs(self):
        parser = OptionParser(usage="%prog [start|stop|check]")
        parser.add_option("-n", "--nodes", help="number of worker nodes", dest="nodes", default=1)
        parser.add_option("-c", "--config", help = "path to config file", dest="config", default="secondo.config")
        parser.add_option("-v", "--verbose", help="print debug output", dest="verbose", default=False)
        return parser.parse_args()

    def __requestParameter(requestText, defaultValue):
        value = raw_input(requestText + " [%s]" % str(defaultValue) + ": ")
        return (value or defaultValue)

    def __evaluateOptions(self, options):
        if options.verbose:
            self.logger.setLevel(logging.DEBUG)
        self.logger.debug(options)
        self.nodes = options.nodes

    def __loadConfig(self, options, args):
        self.config = SecondoConfiguration(options.nodes, options.config)
        self.config.readConfigFromFile()

    def _doAction(self, options, args):
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

    def run(self):
        (options, args) = self.__getProgramOptionsAndArgs()
        self.__evaluateOptions(options)
        self.__loadConfig(options, args)
        self._doAction(options,args)

    def start(self):
        for node in range(1, int(self.nodes)):
            monitor = Monitor(os.path.join(self.buildDir, "bin"), self.config.getDataDir(node), self.config.getPort(node), self.config.getConfig(node), self.logger)
            monitor.start()

    def check(self):
        pass

    def stop(self):
        pass

if __name__=="__main__":
    logger = logging.getLogger(__name__)
    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.INFO)
    remoteMonitors = RemoteMonitors(logger)
    remoteMonitors.run()
