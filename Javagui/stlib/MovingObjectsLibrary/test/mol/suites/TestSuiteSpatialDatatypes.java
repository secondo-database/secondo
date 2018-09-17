package mol.suites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import mol.datatypes.spatial.LineTest;
import mol.datatypes.spatial.PointTest;
import mol.datatypes.spatial.RegionTest;
import mol.datatypes.spatial.util.CycleTest;
import mol.datatypes.spatial.util.FaceTest;
import mol.datatypes.spatial.util.HalfsegmentTest;
import mol.datatypes.spatial.util.RectangleTest;
import mol.datatypes.spatial.util.SegmentTest;

@RunWith(Suite.class)
@SuiteClasses({ PointTest.class, RectangleTest.class, SegmentTest.class, HalfsegmentTest.class, LineTest.class,
      CycleTest.class, FaceTest.class, RegionTest.class })
public class TestSuiteSpatialDatatypes {

}
