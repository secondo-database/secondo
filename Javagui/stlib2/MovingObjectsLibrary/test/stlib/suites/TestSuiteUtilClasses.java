package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.util.CrossPointScalarsTest;
import stlib.util.GeneralHelperTest;
import stlib.util.Vector2DTest;

@RunWith(Suite.class)
@SuiteClasses({ GeneralHelperTest.class, Vector2DTest.class, CrossPointScalarsTest.class })
public class TestSuiteUtilClasses {

}
