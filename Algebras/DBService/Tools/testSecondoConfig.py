import unittest
from SecondoConfiguration import SecondoConfiguration

class SecondoConfigTest(unittest.TestCase):

    def setUp(self):
        self.config = SecondoConfiguration(3, "myFile")

    def testNumberOfNodes(self):
        self.assertEquals(3, self.config.numberOfNodes)

    def testGetHost(self):
        self.config.values[(1, "host")] = "localhost"
        self.assertEquals("localhost", self.config.getHost(1))

    def testGetPort(self):
        self.config.values[(2, "port")] = 12345
        self.assertEquals(12345, self.config.getPort(2))

    def testGetDataDir(self):
        self.config.values[(3, "data")] = "/home/me/myDataDir"
        self.assertEquals("/home/me/myDataDir", self.config.getDataDir(3))

    def testGetConfig(self):
        self.config.values[(4, "config")] = "/home/me/myConfigFile.dat"
        self.assertEquals("/home/me/myConfigFile.dat", self.config.getConfig(4))

if __name__ == "__main__":
    unittest.main()