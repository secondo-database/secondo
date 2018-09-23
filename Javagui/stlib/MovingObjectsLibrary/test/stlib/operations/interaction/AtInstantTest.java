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
package stlib.operations.interaction;

import static org.junit.Assert.*;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Arrays;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.TestUtil.TestUtilData;
import stlib.datatypes.base.BaseBool;
import stlib.datatypes.interval.Period;
import stlib.datatypes.moving.MovingBool;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.moving.MovingRegion;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.Region;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.spatial.UnitRegionLinear;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.intime.IntimeIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.RegionIF;

/**
 * Tests for 'AtInstant' class methods
 * 
 * @author Markus Fuessel
 */
public class AtInstantTest {

   /**
    * [2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000), TRUE<br>
    * [2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000), FALSE<br>
    * [2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000), TRUE
    */
   private MovingBool mbool;

   private MovingPoint mpointContinuos;

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
      mbool = new MovingBool(0);

      mbool.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), true);

      mbool.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), false);

      mbool.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), true);

      mpointContinuos = new MovingPoint(0);

      Point p1 = new Point(0.0d, 0.0d);
      Point p2 = new Point(10.0d, 10.0d);
      Point p3 = new Point(10.0d, 20.0d);
      Point p4 = new Point(20.0d, 5.0d);

      mpointContinuos.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), p1, p2);
      mpointContinuos.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), p2, p3);
      mpointContinuos.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), p3, p4);

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
   public void testAtInstant_OnMBool_DefinedInstantInsideDefinedPeriod_ShouldReturnDefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = AtInstant.execute(mbool, new TimeInstant("2018-01-20 00:00:00:000"));

      assertTrue(ibool.isDefined());
      assertEquals(new BaseBool(true), ibool.getValue());
   }

   @Test
   public void testAtInstant_OnMBool_DefinedInstantOutsideDefinedPeriod_ShouldReturnUndefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool1 = AtInstant.execute(mbool, new TimeInstant("2017-12-31 23:59:59:999"));
      IntimeIF<BaseBoolIF> ibool2 = AtInstant.execute(mbool, new TimeInstant("2018-02-30 00:00:00:000"));

      assertFalse(ibool1.isDefined());
      assertFalse(ibool2.isDefined());
   }

   @Test
   public void testAtInstant_OnMBool_UndefinedInstant_ShouldReturnUndefinedIntimeBaseBool() {

      IntimeIF<BaseBoolIF> ibool = AtInstant.execute(mbool, new TimeInstant());

      assertFalse(ibool.isDefined());
   }

   @Test
   public void testAtInstant_OnMPoint_ValidInstant() {

      IntimeIF<PointIF> ipoint = AtInstant.execute(mpointContinuos, new TimeInstant("2018-01-20 00:00:00:000"));

      assertEquals(new Point(10.0, 20.0), ipoint.getValue());
   }

   @Test
   public void testAtInstant_OnMPoint_UndefinedInstant_ShouldReturnUndefinedIntimePoint() {

      IntimeIF<PointIF> ipoint = AtInstant.execute(mpointContinuos, new TimeInstant());

      assertFalse(ipoint.isDefined());
   }

   @Test
   public void testAtInstant_OnMRegion_DiskreteMovingRegion_ValidInstant() {

      IntimeIF<RegionIF> iregion = AtInstant.execute(discreteMovingRegion, new TimeInstant("2018-01-20 00:00:00:000"));

      assertEquals(region3.getBoundingBox(), iregion.getValue().getBoundingBox());
   }

   @Test
   public void testAtInstant_OnMRegion_ContinuosMovingRegionValidInstant_UndefinedCauseOfDegenerate() {

      IntimeIF<RegionIF> iregion = AtInstant.execute(continuosMovingRegion, new TimeInstant("2018-01-29 23:59:59:999"));

      assertFalse(iregion.isDefined());
   }

   @Test
   public void testAtInstant_OnMRegion_UndefinedInstant_ShouldReturnUndefinedIntimeRegion() {

      IntimeIF<RegionIF> iregionDMR = AtInstant.execute(discreteMovingRegion, new TimeInstant());
      IntimeIF<RegionIF> iregionCMR = AtInstant.execute(continuosMovingRegion, new TimeInstant());

      assertFalse(iregionDMR.isDefined());
      assertFalse(iregionCMR.isDefined());
   }

}
