package mol.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.operations.predicates.InsideTest;
import mol.operations.predicates.lifted.InsidePointRegionTest;

@RunWith(Suite.class)
@SuiteClasses({ InsideTest.class, InsidePointRegionTest.class })
public class TestSuiteOperations {

}
