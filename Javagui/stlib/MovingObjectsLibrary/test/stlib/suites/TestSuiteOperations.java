package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.operations.interaction.AtInstantTest;
import stlib.operations.interaction.PassesTest;
import stlib.operations.interaction.PresentTest;
import stlib.operations.predicates.InsideTest;
import stlib.operations.predicates.lifted.InsidePointRegionTest;
import stlib.operations.projection.TrajectoryTest;

@RunWith(Suite.class)
@SuiteClasses({ AtInstantTest.class, PassesTest.class, PresentTest.class, InsideTest.class, InsidePointRegionTest.class,
      TrajectoryTest.class })
public class TestSuiteOperations {

}
