import unittest
from testRemoteMonitors import RemoteMonitorsTest
from testSecondoConfig import SecondoConfigTest

def suite():
    suite = unittest.TestSuite()
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(RemoteMonitorsTest))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(SecondoConfigTest))
    return suite

if __name__ == "main":
    unittest.TextTestRunner(verbosity=2).run(suite())