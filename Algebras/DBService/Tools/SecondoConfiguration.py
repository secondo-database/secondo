import ConfigParser
import os

class SecondoConfiguration():

    def __init__(self, numberOfNodes, fileName):
        self.values = {}
        self.numberOfNodes = numberOfNodes
        self.fileName = fileName

    def readConfigFromFile(self):
        configFileParser = ConfigParser.RawConfigParser()
        configFileParser.read(self.fileName)

        for n in range(1, int(self.numberOfNodes)):
            node = "%d" % n
            self.values[(n, "host")] = configFileParser.get(node, "host")
            self.values[(n, "port")] = configFileParser.get(node, "port")
            self.values[(n, "data")] = configFileParser.get(node, "data")
            self.values[(n, "config")] = configFileParser.get(node, "config")

    def getHost(self, node):
        return self.values[(node,"host")]

    def getPort(self, node):
        return self.values[(node,"port")]

    def getDataDir(self, node):
        return self.values[(node,"data")]

    def getConfig(self, node):
        return self.values[(node,"config")]
