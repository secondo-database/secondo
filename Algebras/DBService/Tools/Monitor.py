import os
import subprocess
import shlex
import logging

class Monitor:

    def __init__(self, binDir, dataDir, port, configFile, logger):
        self.binDir = binDir
        self.dataDir = dataDir
        self.port = port
        self.configFile = configFile
        self.logger = logger

    def start(self):
        self.logger.info("Starting monitor at port %s (config file: %s, data volume: %s)" % (self.port, self.configFile, self.dataDir))
        os.chdir(self.binDir)
        args = shlex.split("./SecondoMonitor -d %s -p %s -c %s" % (self.dataDir, self.port, self.configFile))
        logFile = open("SecondoMonitor-%s.log" % self.port, 'w+')
        errorLog = open("SecondoMonitor-%s-error.log" % self.port, 'w+')
        subprocess.Popen(args,stdout=logFile, stderr=errorLog)