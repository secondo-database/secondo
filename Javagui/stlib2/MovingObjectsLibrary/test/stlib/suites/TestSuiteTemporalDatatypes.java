package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.datatypes.moving.MovingBoolTest;
import stlib.datatypes.moving.MovingPointTest;
import stlib.datatypes.moving.MovingRegionTest;
import stlib.datatypes.unit.UnitBoolTest;
import stlib.datatypes.unit.UnitIntTest;
import stlib.datatypes.unit.spatial.UnitPointConstTest;
import stlib.datatypes.unit.spatial.UnitPointLinearTest;
import stlib.datatypes.unit.spatial.UnitRegionConstTest;
import stlib.datatypes.unit.spatial.UnitRegionLinearTest;
import stlib.datatypes.unit.spatial.util.MovableCycleTest;
import stlib.datatypes.unit.spatial.util.MovableFaceTest;
import stlib.datatypes.unit.spatial.util.MovableSegmentTest;

@RunWith(Suite.class)
@SuiteClasses({ UnitPointConstTest.class, UnitPointLinearTest.class, UnitIntTest.class, UnitBoolTest.class,
      MovingBoolTest.class, MovingPointTest.class, UnitRegionConstTest.class, UnitRegionLinearTest.class,
      MovingRegionTest.class, MovableSegmentTest.class, MovableCycleTest.class, MovableFaceTest.class })

public class TestSuiteTemporalDatatypes {

}
