import unittest
import os
from remoteMonitors import RemoteMonitors
import logging
from Exceptions import UnsuitableEnvironmentException

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
            del os.environ[key]

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

    def testRunWithNoneAsArg(self):
        rm = RemoteMonitors(logging.getLogger(__name__))
        try:#TODO use assertRaises
            rm._doAction(None, None)
            self.fail("run method must throw an exception if called with None as args")
        except ValueError as err:
            self.assertEquals("Call with exactly one action: [start|stop|check]", err.message)

    def testRunWithMoreThanOneArg(self):
        rm = RemoteMonitors(logging.getLogger(__name__))
        try:#TODO use assertRaises
            rm._doAction(None, ["start", "check"])
            self.fail("run method must throw an exception if called with more than one arg")
        except ValueError as err:
            self.assertEquals("Call with exactly one action: [start|stop|check]", err.message)

    def testRunWithInvalidActionAsArg(self):
        rm = RemoteMonitors(logging.getLogger(__name__))
        try:#TODO use assertRaises
            rm._doAction(None, ["sleep"])
            self.fail("run method must throw an exception if action is invalid")
        except ValueError as err:
            self.assertEquals("Action must be one of [start|stop|check]", err.message)

if __name__ == "__main__":
    unittest.main()