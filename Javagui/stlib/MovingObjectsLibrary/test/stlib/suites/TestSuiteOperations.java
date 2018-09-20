package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.operations.predicates.InsideTest;
import stlib.operations.predicates.lifted.InsidePointRegionTest;

@RunWith(Suite.class)
@SuiteClasses({ InsideTest.class, InsidePointRegionTest.class })
public class TestSuiteOperations {

}
