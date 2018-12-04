package unittests.mmdb.streamprocessing.objectnodes.projection;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

import mmdb.data.attributes.moving.AttributeMpoint;
import mmdb.data.attributes.spatial.AttributeLine;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.projection.Trajectory;
import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

public class TrajectoryTest {

   private AttributeMpoint mpoint;

   @Before
   public void setUp() throws Exception {
      mpoint = new AttributeMpoint();
      ListExpr list = ListExpr.threeElemList(TestUtilAttributes.getUpointList(), TestUtilAttributes.getUpointList(),
            TestUtilAttributes.getUpointList());

      mpoint.fromList(list);
   }

   @Test
   public void testTrajectory() throws TypeException, MemoryException {
      ObjectNode attrNode = ConstantNode.createConstantNode(mpoint, new AttributeMpoint());

      Trajectory trajectory = new Trajectory(attrNode);
      trajectory.typeCheck();

      assertEquals(AttributeLine.class, trajectory.getOutputType().getClass());

      AttributeLine line = (AttributeLine) trajectory.getResult();

      System.out.println(line.toList());

   }

}
