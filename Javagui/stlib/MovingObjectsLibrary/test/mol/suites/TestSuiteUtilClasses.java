package mol.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.util.CrossPointScalarsTest;
import mol.util.GeneralHelperTest;
import mol.util.Vector2DTest;

@RunWith(Suite.class)
@SuiteClasses({ GeneralHelperTest.class, Vector2DTest.class, CrossPointScalarsTest.class })
public class TestSuiteUtilClasses {

}
