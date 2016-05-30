import unittest
import os
from remoteMonitors import RemoteMonitors
import logging
from env import UnsuitableEnvironmentException

class RemoteMonitorsTest(unittest.TestCase):


    def setUp(self):
        """save environment variables to be able to restore them after the test"""
        self.platformEnvKey = "SECONDO_PLATFORM"
        self.platform = os.getenv(self.platformEnvKey, None)
        self.buildDirEnvKey = "SECONDO_BUILD_DIR"
        self.buildDir = os.getenv(self.buildDirEnvKey, None)
        self.sdkEnvKey = "SECONDO_SDK"
        self.sdk = os.getenv(self.sdkEnvKey, None)

    def tearDown(self):
        self.resetEnvironmentVariable(self.platformEnvKey, self.platform)
        self.resetEnvironmentVariable(self.buildDirEnvKey, self.buildDir)
        self.resetEnvironmentVariable(self.sdkEnvKey, self.sdk)

    def resetEnvironmentVariable(self, key, originalValue):
        if key is not None:
            os.environ[key] = originalValue
        else:
            os.unsetenv(key)

    def runEnvironmentVariableTest(self, key):
        del os.environ[key]
        try:
            RemoteMonitors(logging.getLogger(__name__))
            self.fail("__init__ of RemoteMonitors must throw an exception")
        except UnsuitableEnvironmentException as exc:
            self.assertEquals("Environment variable %s is not set" % key, exc.message)

    def testPlatformNone(self):
        self.runEnvironmentVariableTest(self.platformEnvKey)

    def testBuildDirNone(self):
        self.runEnvironmentVariableTest(self.buildDirEnvKey)

    def testSdkNone(self):
        self.runEnvironmentVariableTest(self.sdkEnvKey)

if __name__ == "__main__":
    unittest.main()