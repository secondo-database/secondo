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
package mol.datatypes.unit.spatial;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import mol.TestUtil.TestUtilData;
import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.datatypes.unit.spatial.util.MovableFaceIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.time.TimeInstantIF;
import mol.interfaces.unit.spatial.UnitRegionIF;

/**
 * Tests for the 'UnitRegionLinearTest' class
 * 
 * @author Markus Fuessel
 */
public class UnitRegionLinearTest {

   UnitRegionLinear linearURegion;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Before
   public void setUp() throws Exception {

      List<MovableFaceIF> movingFaces = new ArrayList<>();
      movingFaces.add(TestUtilData.getMovableFace(0, 0));

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);

      linearURegion = new UnitRegionLinear(period, movingFaces);
   }

   @Test
   public void testAdd_NoOfMovingFacesIncrease_PBBChanges() {
      int noMovingFacesBeforeAdd = linearURegion.getNoMovingFaces();

      MovableFaceIF mface = TestUtilData.getMovableFace(50, 50);

      RectangleIF expectedPBB = linearURegion.getProjectionBoundingBox().merge(mface.getProjectionBoundingBox());

      linearURegion.add(mface);

      int noMovingFacesAfterAdd = linearURegion.getNoMovingFaces();

      assertEquals(noMovingFacesBeforeAdd + 1, noMovingFacesAfterAdd);
      assertEquals(expectedPBB, linearURegion.getProjectionBoundingBox());
   }

   @Test
   public void testGetValue_AtValidTimeInstant_ShouldReturnDefinedRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);

      List<MovableFaceIF> movingFaces = new ArrayList<>();
      movingFaces.add(TestUtilData.getMovableFace(0, 0));
      movingFaces.add(TestUtilData.getMovableFaceWithHoles(50, 50));
      movingFaces.add(TestUtilData.getMovableFaceDegenerate(-50, -50, true));

      UnitRegionIF uregion = new UnitRegionLinear(period, movingFaces);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant3 = new TimeInstant("2018-01-09 23:59:59:999");

      RegionIF region1 = uregion.getValue(instant1);
      RegionIF region2 = uregion.getValue(instant2);
      RegionIF region3 = uregion.getValue(instant3);

      assertTrue(region1.isDefined());
      assertEquals(3, region1.getNoComponents());

      assertTrue(region2.isDefined());
      assertEquals(3, region2.getNoComponents());

      assertTrue(region3.isDefined());
      assertEquals(2, region3.getNoComponents());

   }

   @Test
   public void testGetValue_AtInvalidInstant_ShouldReturnUndefinedRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);

      List<MovableFaceIF> movingFaces = new ArrayList<>();
      movingFaces.add(TestUtilData.getMovableFace(0, 0));
      movingFaces.add(TestUtilData.getMovableFaceWithHoles(50, 50));
      movingFaces.add(TestUtilData.getMovableFaceDegenerate(-50, -50, true));

      UnitRegionIF uregion = new UnitRegionLinear(period, movingFaces);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-09 23:59:59:999");
      TimeInstantIF instant3 = new TimeInstant("2017-01-01 00:00:00:000");
      TimeInstantIF instant4 = new TimeInstant();

      RegionIF region1 = uregion.getValue(instant1);
      RegionIF region2 = uregion.getValue(instant2);
      RegionIF region3 = uregion.getValue(instant3);
      RegionIF region4 = uregion.getValue(instant4);

      assertFalse(region1.isDefined());
      assertFalse(region2.isDefined());
      assertFalse(region3.isDefined());
      assertFalse(region4.isDefined());
   }

   @Test
   public void testGetInitial() {

      linearURegion.add(TestUtilData.getMovableFaceDegenerate(50, 50, true));
      linearURegion.add(TestUtilData.getMovableFaceDegenerate(-50, -50, false));

      RegionIF region = linearURegion.getInitial();

      assertTrue(region.isDefined());
      assertEquals(2, region.getNoComponents());
   }

   @Test
   public void testGetInitial_DegeneratedRegionAtBeginning() {

      List<MovableFaceIF> movingFaces = new ArrayList<>();
      movingFaces.add(TestUtilData.getMovableFaceDegenerate(0, 0, false));

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);

      UnitRegionLinear uregion = new UnitRegionLinear(period, movingFaces);

      RegionIF region = uregion.getInitial();

      assertFalse(region.isDefined());
   }

   @Test
   public void testGetFinal() {
      linearURegion.add(TestUtilData.getMovableFaceDegenerate(50, 50, true));
      linearURegion.add(TestUtilData.getMovableFaceDegenerate(-50, -50, false));
      Region region = linearURegion.getFinal();

      assertTrue(region.isDefined());
      assertEquals(2, region.getNoComponents());
   }

   @Test
   public void testGetFinal_DegeneratedRegionAtEnd() {

      List<MovableFaceIF> movingFaces = new ArrayList<>();
      movingFaces.add(TestUtilData.getMovableFaceDegenerate(0, 0, true));

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);

      UnitRegionLinear uregion = new UnitRegionLinear(period, movingFaces);

      Region region = uregion.getFinal();

      assertFalse(region.isDefined());
   }

   @Test
   public void testGetMovingSegments() {

      int noMovingSegmentsBeforeAdd = linearURegion.getMovingSegments().size();

      MovableFaceIF mface = TestUtilData.getMovableFace(50, 50);

      linearURegion.add(mface);

      int noMovingSegmentsAfterAdd = linearURegion.getMovingSegments().size();

      assertEquals(5, noMovingSegmentsBeforeAdd);
      assertEquals(10, noMovingSegmentsAfterAdd);

   }

   @Test
   public void testGetProjectionBoundingBox() {
      RectangleIF expectedRectangle = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);

      assertEquals(expectedRectangle, linearURegion.getProjectionBoundingBox());
   }
}
