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
package stlib.datatypes.moving;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Arrays;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.TestUtil.TestUtilData;
import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingRegion;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.Region;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.spatial.UnitRegionConst;
import stlib.datatypes.unit.spatial.UnitRegionLinear;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.spatial.RegionIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.unit.spatial.UnitRegionIF;

/**
 * Tests for the 'MovingRegion' class
 * 
 * @author Markus Fuessel
 */
public class MovingRegionTest {

   private static Region region1;
   private static Region region2;
   private static Region region3;

   private MovingRegion continuosMovingRegion;
   private MovingRegion discreteMovingRegion;

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
   public void testMovingRegion_DefinedMovingRegion() {
      MovingRegion mregion = new MovingRegion(1);

      assertTrue(mregion.isDefined());
   }

   @Test
   public void testMovingRegion_Region() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));
      MovingRegion mregion = new MovingRegion(region);

      assertTrue(mregion.isDefined());
      assertEquals(1, mregion.getNoUnits());
   }

   @Test
   public void testMovingRegion_PassDefinedPeriodAndRegion_DefinedMovingRegion() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      MovingRegion mregion = new MovingRegion(
            new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false), region);

      assertTrue(mregion.isDefined());
      assertEquals(1, mregion.getNoUnits());
   }

   @Test
   public void testMovingRegion_PassUndefinedPeriod_UndefinedMovingRegion() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      MovingRegion mregion = new MovingRegion(new Period(), region);

      assertFalse(mregion.isDefined());
      assertEquals(0, mregion.getNoUnits());
   }

   @Test
   public void testAdd_PassDefinedUnitRegionConst_SuccessfulAppend() {

      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      UnitRegionIF uregion = new UnitRegionConst(
            new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false), region);

      int sizeBefore = discreteMovingRegion.getNoUnits();

      discreteMovingRegion.add(uregion);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_PassDefinedUnitRegionLinear_SuccessfulAppend() {

      UnitRegionLinear uregion = new UnitRegionLinear(
            new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false),
            TestUtilData.getMovableFaceDegenerate(0, 0, false));

      int sizeBefore = continuosMovingRegion.getNoUnits();

      boolean successful = continuosMovingRegion.add(uregion);

      int sizeAfter = continuosMovingRegion.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
      assertTrue(successful);
   }

   @Test
   public void testAdd_PassUndefinedUnitRegionConst_FailedAppend() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      UnitRegionIF uregion = new UnitRegionConst(new Period(), region);

      int sizeBefore = discreteMovingRegion.getNoUnits();

      boolean successful = discreteMovingRegion.add(uregion);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);
   }

   @Test
   public void testAdd_PassUndefinedUnitRegionLinear_FailedAppend() {

      UnitRegionLinear uregion = new UnitRegionLinear(new Period(), TestUtilData.getMovableFaceDegenerate(0, 0, false));

      int sizeBefore = continuosMovingRegion.getNoUnits();

      boolean successful = continuosMovingRegion.add(uregion);

      int sizeAfter = continuosMovingRegion.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);
   }

   @Test
   public void testAdd_PassDefinedPeriodAndRegion_SuccessfulAppend() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      int sizeBefore = discreteMovingRegion.getNoUnits();

      discreteMovingRegion.add(new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false), region);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_PassUndefinedPeriod_FailedAppend() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      int sizeBefore = discreteMovingRegion.getNoUnits();

      boolean successful = discreteMovingRegion.add(new Period(), region);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);
   }

   @Test
   public void testAdd_PassUndefinedRegion_FailedAppend() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      int sizeBefore = discreteMovingRegion.getNoUnits();

      boolean successful = discreteMovingRegion
            .add(new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false), region);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);
   }

   @Test
   public void testAdd_IntersectingPeriod_AppendFailed() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      int sizeBefore = discreteMovingRegion.getNoUnits();

      boolean successful = discreteMovingRegion
            .add(new Period("2018-01-15 00:00:00:000", "2018-02-20 00:00:00:000", true, false), region);

      int sizeAfter = discreteMovingRegion.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testGetProjectionBoundingBox_VerifyPBBAfterAddingUnit() {
      Point[] circle = { new Point(0.0d, 0.0d), new Point(-50.0d, -50.0d), new Point(-100.0d, 0.0d) };

      Region region = new Region(new ArrayList<>(Arrays.asList(circle)));

      RectangleIF objPBBBefore = discreteMovingRegion.getProjectionBoundingBox();
      RectangleIF expectedObjPBBAfter = objPBBBefore.merge(region.getBoundingBox());

      discreteMovingRegion.add(new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false), region);

      assertEquals(expectedObjPBBAfter, discreteMovingRegion.getProjectionBoundingBox());
   }

   @Test
   public void testGetProjectionBoundingBox_VerifyPBBAfterAddingUnitRegionLinear() {

      UnitRegionLinear uregion = new UnitRegionLinear(
            new Period("2018-01-30 00:00:00:000", "2018-02-30 00:00:00:000", true, false),
            TestUtilData.getMovableFaceDegenerate(50, 50, false));

      RectangleIF objPBBBefore = continuosMovingRegion.getProjectionBoundingBox();
      RectangleIF expectedObjPBBAfter = objPBBBefore.merge(uregion.getProjectionBoundingBox());

      continuosMovingRegion.add(uregion);

      assertEquals(expectedObjPBBAfter, continuosMovingRegion.getProjectionBoundingBox());
   }

   @Test
   public void testPresent_PassTimeInstant() {

      assertTrue(discreteMovingRegion.present(new TimeInstant("2018-01-20 00:00:00:000")));
      assertTrue(discreteMovingRegion.present(new TimeInstant("2018-01-25 00:00:00:000")));

      assertTrue(continuosMovingRegion.present(new TimeInstant("2018-01-20 00:00:00:000")));
      assertTrue(continuosMovingRegion.present(new TimeInstant("2018-01-25 00:00:00:000")));
   }

   @Test
   public void testAtInstant_DiskreteMovingRegion_ValidInstant() {

      IntimeIF<RegionIF> iregion = discreteMovingRegion.atInstant(new TimeInstant("2018-01-20 00:00:00:000"));

      assertEquals(region3.getBoundingBox(), iregion.getValue().getBoundingBox());
   }

   @Test
   public void testAtInstant_ContinuosMovingRegionValidInstant_UndefinedCauseOfDegenerate() {

      IntimeIF<RegionIF> iregion = continuosMovingRegion.atInstant(new TimeInstant("2018-01-29 23:59:59:999"));

      assertFalse(iregion.isDefined());
   }

   @Test
   public void testAtInstant_UndefinedInstant_ShouldReturnUndefinedIntimeRegion() {

      IntimeIF<RegionIF> iregionDMR = discreteMovingRegion.atInstant(new TimeInstant());
      IntimeIF<RegionIF> iregionCMR = continuosMovingRegion.atInstant(new TimeInstant());

      assertFalse(iregionDMR.isDefined());
      assertFalse(iregionCMR.isDefined());
   }

   @Test
   public void testGetValue_ValidInstant_ShouldReturnValidRegion() {
      RegionIF regionDMR = discreteMovingRegion.getValue(new TimeInstant("2018-01-20 00:00:00:000"));
      RegionIF regionCMR = continuosMovingRegion.getValue(new TimeInstant("2018-01-20 00:00:00:000"));

      assertTrue(regionDMR.isDefined());
      assertTrue(regionCMR.isDefined());
   }

   @Test
   public void testGetValue_UndefinedInstant_ShouldReturnUndefinedRegion() {
      RegionIF regionDMR = discreteMovingRegion.getValue(new TimeInstant());
      RegionIF regionCMR = continuosMovingRegion.getValue(new TimeInstant());

      assertFalse(regionDMR.isDefined());
      assertFalse(regionCMR.isDefined());
   }

   @Test
   public void testGetUnit_ValidPosition_ShouldReturnDefinedUnit() {

      UnitRegionIF uregionDMR = discreteMovingRegion.getUnit(discreteMovingRegion.getNoUnits() - 1);
      UnitRegionIF uregionCMR = continuosMovingRegion.getUnit(continuosMovingRegion.getNoUnits() - 1);

      assertTrue(uregionDMR.isDefined());
      assertTrue(uregionCMR.isDefined());

   }

   @Test
   public void testGetUnit_InvalidPosition_ShouldReturnUndefinedUnit() {

      UnitRegionIF uregionDMR = discreteMovingRegion.getUnit(discreteMovingRegion.getNoUnits());
      UnitRegionIF uregionCMR = continuosMovingRegion.getUnit(continuosMovingRegion.getNoUnits());

      assertFalse(uregionDMR.isDefined());
      assertFalse(uregionCMR.isDefined());

   }

   @Test
   public void testGetUnitPosition_ValidInstant_ShouldReturnValidUnitPosition() {
      TimeInstant instant = new TimeInstant("2018-01-20 00:00:00:000");

      int unitPosDMR = discreteMovingRegion.getUnitPosition(instant);
      int unitPosCMR = continuosMovingRegion.getUnitPosition(instant);

      assertEquals(2, unitPosDMR);
      assertEquals(2, unitPosCMR);
   }

   @Test
   public void testGetUnitPosition_InstantOutsideOfPeriods_ShouldReturnInvalidUnitPosition() {
      TimeInstant instant = new TimeInstant("2017-01-01 00:00:00:000");

      int unitPosDMR = discreteMovingRegion.getUnitPosition(instant);
      int unitPosCMR = continuosMovingRegion.getUnitPosition(instant);

      assertEquals(-1, unitPosDMR);
      assertEquals(-1, unitPosCMR);

   }

   @Test
   public void testGetUnitPosition_UndefinedInstant_ShouldReturnInvalidUnitPosition() {
      TimeInstant instant = new TimeInstant();

      int unitPosDMR = discreteMovingRegion.getUnitPosition(instant);
      int unitPosCMR = continuosMovingRegion.getUnitPosition(instant);

      assertEquals(-1, unitPosDMR);
      assertEquals(-1, unitPosCMR);

   }
}
