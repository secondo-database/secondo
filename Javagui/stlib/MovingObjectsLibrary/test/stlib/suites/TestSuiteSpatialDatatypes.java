package stlib.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import stlib.datatypes.spatial.LineTest;
import stlib.datatypes.spatial.PointTest;
import stlib.datatypes.spatial.RegionTest;
import stlib.datatypes.spatial.util.CycleTest;
import stlib.datatypes.spatial.util.FaceTest;
import stlib.datatypes.spatial.util.HalfsegmentTest;
import stlib.datatypes.spatial.util.RectangleTest;
import stlib.datatypes.spatial.util.SegmentTest;

@RunWith(Suite.class)
@SuiteClasses({ PointTest.class, RectangleTest.class, SegmentTest.class, HalfsegmentTest.class, LineTest.class,
      CycleTest.class, FaceTest.class, RegionTest.class })
public class TestSuiteSpatialDatatypes {

}
