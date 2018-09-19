//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, Department of Computer Science,
//Database Systems for New Applications.

//SECONDO is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.

//SECONDO is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with SECONDO; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
package mol.operations.predicates.lifted;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import mol.TestUtil.TestUtilData;
import mol.datatypes.interval.Period;
import mol.datatypes.intime.Intime;
import mol.datatypes.moving.MovingPoint;
import mol.datatypes.moving.MovingRegion;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Face;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.spatial.UnitPointLinear;
import mol.datatypes.unit.spatial.UnitRegionConst;
import mol.datatypes.unit.spatial.UnitRegionLinear;
import mol.datatypes.unit.spatial.util.MovableSegment;
import mol.datatypes.unit.spatial.util.MovableSegmentIF;
import mol.interfaces.moving.MovingBoolIF;
import mol.interfaces.moving.MovingObjectIF;
import mol.interfaces.moving.MovingPointIF;
import mol.interfaces.moving.MovingRegionIF;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.unit.spatial.UnitPointIF;
import mol.interfaces.unit.spatial.UnitRegionIF;

/**
 * Tests for 'InsidePointRegion' class methods
 * 
 * @author Markus Fuessel
 */
public class InsidePointRegionTest {

   /**
    * Boundary:<br>
    * ( (0, 0), (0, 50), (50, 50), (50, 0) )
    */
   private Region regionWithoutHole;

   /**
    * Boundary:<br>
    * ( (0, 0), (0, 50), (50, 50), (50, 0) )<br>
    * <br>
    * Holes:<br>
    * ( (20, 20), (20, 40), (40, 40), (40, 20) )
    */
   private Region regionWithHole;

   private static Region region1;
   private static Region region2;
   private static Region region3;

   private MovingRegion continuosMovingRegion;
   private MovingRegionIF discreteMovingRegion;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);

      Point[] circle1 = { new Point(0.0d, 0.0d), new Point(5.0d, 5.0d), new Point(10.0d, 0.0d) };
      Point[] circle2 = { new Point(10.0d, 10.0d), new Point(30.0d, 30.0d), new Point(40.0d, 10.0d) };
      Point[] circle3 = { new Point(0.0d, 0.0d), new Point(50.0d, 50.0d), new Point(100.0d, 0.0d) };

      region1 = new Region(new ArrayList<>(Arrays.asList(circle1)));
      region2 = new Region(new ArrayList<>(Arrays.asList(circle2)));
      region3 = new Region(new ArrayList<>(Arrays.asList(circle3)));
   }

   @Before
   public void setUp() throws Exception {

      List<PointIF> points = new ArrayList<>();
      points.add(new Point(0, 0));
      points.add(new Point(0, 50));
      points.add(new Point(50, 50));
      points.add(new Point(50, 0));

      regionWithoutHole = new Region(points);

      List<PointIF> holePoints = new ArrayList<>();
      holePoints.add(new Point(20, 20));
      holePoints.add(new Point(20, 40));
      holePoints.add(new Point(40, 40));
      holePoints.add(new Point(40, 20));

      Face face = new Face(points);
      face.add(holePoints);

      regionWithHole = new Region(face);

      discreteMovingRegion = new MovingRegion(0);

      discreteMovingRegion.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), region1);
      discreteMovingRegion.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), region2);
      discreteMovingRegion.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), region3);

      continuosMovingRegion = new MovingRegion(0);

      UnitRegionLinear uregion1 = new UnitRegionLinear(
            new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false));
      UnitRegionLinear uregion2 = new UnitRegionLinear(
            new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false));
      UnitRegionLinear uregion3 = new UnitRegionLinear(
            new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false));

      uregion1.add(TestUtilData.getMovableFaceDegenerate(0, 0, true));
      uregion2.add(TestUtilData.getMovableFaceDegenerate(0, 0, false));
      uregion3.add(TestUtilData.getMovableFaceDegenerate(0, 0, true));

      continuosMovingRegion.add(uregion1);
      continuosMovingRegion.add(uregion2);
      continuosMovingRegion.add(uregion3);
   }

   @Test
   public void testInsidePointRegion_PointMovingRegion() {

      InsidePointRegion insidePMROp = new InsidePointRegion(new Point(10.0d, 10.0d), continuosMovingRegion);

      MovingObjectIF<?, ?> mobject1 = insidePMROp.getMobject1();
      MovingObjectIF<?, ?> mobject2 = insidePMROp.getMobject2();

      assertTrue(mobject1 instanceof MovingPointIF);
      assertTrue(mobject2 instanceof MovingRegionIF);
   }

   @Test
   public void testInsidePointRegion_MovingPointRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      InsidePointRegion insidePMROp = new InsidePointRegion(mpoint, regionWithHole);

      MovingObjectIF<?, ?> mobject1 = insidePMROp.getMobject1();
      MovingObjectIF<?, ?> mobject2 = insidePMROp.getMobject2();

      assertTrue(mobject1 instanceof MovingPointIF);
      assertTrue(mobject2 instanceof MovingRegionIF);
   }

   @Test
   public void testInsidePointRegion_MovingPointMovingRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      InsidePointRegion insidePMROp = new InsidePointRegion(mpoint, continuosMovingRegion);

      MovingObjectIF<?, ?> mobject1 = insidePMROp.getMobject1();
      MovingObjectIF<?, ?> mobject2 = insidePMROp.getMobject2();

      assertTrue(mobject1 instanceof MovingPointIF);
      assertTrue(mobject2 instanceof MovingRegionIF);
   }

   @Test
   public void testAdditionalChecksSuccessful_BoundingBoxesIntersects_ShouldBeTrue() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, -10.0d);
      Point pointEnd = new Point(100.0d, 100.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      InsidePointRegion insidePMROp = new InsidePointRegion(mpoint, regionWithHole);

      assertTrue(insidePMROp.additionalChecksSuccessful());
   }

   @Test
   public void testAdditionalChecksSuccessful_BoundingBoxesDontIntersects_ShouldBeFalse() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 300.0d);
      Point pointEnd = new Point(100.0d, 300.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      InsidePointRegion insidePMROp = new InsidePointRegion(mpoint, regionWithHole);

      assertFalse(insidePMROp.additionalChecksSuccessful());
   }

   @Test
   public void testGetUnitResult_DifferentPeriodsUPointIntersectsURegionWithoutHole() {
      Period periodPoint = new Period("2018-01-01 00:00:00:000", "2018-01-09 00:00:00:000", true, true);
      Period periodRegion = new Period("2018-01-03 00:00:00:000", "2018-01-10 00:00:00:000", true, true);

      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      UnitPointIF upoint = new UnitPointLinear(periodPoint, pointStart, pointEnd);
      UnitRegionIF uregion = new UnitRegionConst(periodRegion, regionWithoutHole);

      InsidePointRegion insidePR = new InsidePointRegion(new MovingPoint(new Point(0, 0)), regionWithoutHole);

      MovingBoolIF mbool = insidePR.getUnitResult(upoint, uregion);

      assertTrue(mbool.isDefined());
      assertEquals(periodPoint.getUpperBound(), mbool.getPeriods().getMaxValue());
      assertEquals(periodRegion.getLowerBound(), mbool.getPeriods().getMinValue());
   }

   @Test
   public void testIntersectionPoints_UPointIntersectsURegionWithHole() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      UnitPointIF upoint = new UnitPointLinear(period, pointStart, pointEnd);
      UnitRegionIF uregion = new UnitRegionConst(period, regionWithHole);

      List<Intime<PointIF>> iPoints = InsidePointRegion.intersectionPoints(upoint, uregion);

      assertEquals(4, iPoints.size());
   }

   @Test
   public void testIntersectionPoints_UPointDontIntersectsURegionWithHole() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 300.0d);
      Point pointEnd = new Point(100.0d, 300.0d);

      UnitPointIF upoint = new UnitPointLinear(period, pointStart, pointEnd);
      UnitRegionIF uregion = new UnitRegionConst(period, regionWithHole);

      List<Intime<PointIF>> iPoints = InsidePointRegion.intersectionPoints(upoint, uregion);

      assertTrue(iPoints.isEmpty());
   }

   @Test
   public void testIntersectionPoint_UPointIntersectsMovingSegment() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      UnitPointIF upoint = new UnitPointLinear(period, pointStart, pointEnd);

      Point initStartPoint = new Point(50.0, 0.0);
      Point finalStartPoint = new Point(30.0, 0.0);
      Point initEndPoint = new Point(50.0, 50.0);
      Point finalEndPoint = new Point(30.0, 50.0);

      MovableSegmentIF msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Intime<PointIF> iPoint = InsidePointRegion.intersectionPoint(upoint, msegment);

      assertTrue(iPoint.isDefined());
   }

   @Test
   public void testIntersectionPoint_UPointDontIntersectsURegionWithHole() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 300.0d);
      Point pointEnd = new Point(100.0d, 300.0d);

      UnitPointIF upoint = new UnitPointLinear(period, pointStart, pointEnd);
      UnitRegionIF uregion = new UnitRegionConst(period, regionWithHole);

      List<Intime<PointIF>> iPoints = InsidePointRegion.intersectionPoints(upoint, uregion);

      assertTrue(iPoints.isEmpty());
   }

   @Test
   public void testGetResult_MovingPointIntersectsRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, true);
      Point pointStart = new Point(-10.0d, 30.0d);
      Point pointEnd = new Point(100.0d, 30.0d);

      MovingPoint mpoint = new MovingPoint(0);
      mpoint.add(new UnitPointLinear(period, pointStart, pointEnd));

      MovingBoolIF mbool = new InsidePointRegion(mpoint, regionWithHole).getResult();

      assertTrue(mbool.isDefined());
   }

}
