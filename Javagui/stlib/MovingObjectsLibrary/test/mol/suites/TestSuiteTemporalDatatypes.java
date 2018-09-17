package mol.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.datatypes.moving.MovingBoolTest;
import mol.datatypes.moving.MovingPointTest;
import mol.datatypes.moving.MovingRegionTest;
import mol.datatypes.unit.UnitBoolTest;
import mol.datatypes.unit.UnitIntTest;
import mol.datatypes.unit.spatial.UnitPointConstTest;
import mol.datatypes.unit.spatial.UnitPointLinearTest;
import mol.datatypes.unit.spatial.UnitRegionConstTest;
import mol.datatypes.unit.spatial.UnitRegionLinearTest;
import mol.datatypes.unit.spatial.util.MovableCycleTest;
import mol.datatypes.unit.spatial.util.MovableFaceTest;
import mol.datatypes.unit.spatial.util.MovableSegmentTest;

@RunWith(Suite.class)
@SuiteClasses({ UnitPointConstTest.class, UnitPointLinearTest.class, UnitIntTest.class, UnitBoolTest.class,
      MovingBoolTest.class, MovingPointTest.class, UnitRegionConstTest.class, UnitRegionLinearTest.class,
      MovingRegionTest.class, MovableSegmentTest.class, MovableCycleTest.class, MovableFaceTest.class })

public class TestSuiteTemporalDatatypes {

}
