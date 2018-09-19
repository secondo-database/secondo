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

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Cycle;
import mol.datatypes.spatial.util.Face;
import mol.datatypes.time.TimeInstant;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.time.TimeInstantIF;
import mol.interfaces.unit.spatial.UnitRegionIF;

/**
 * Tests for the 'UnitRegionConstTest' class
 * 
 * @author Markus Fuessel
 */
public class UnitRegionConstTest {

   Region simpleRegion;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Before
   public void setUp() throws Exception {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      simpleRegion = new Region(face);
   }

   @Test
   public void testUnitRegionConst_DefinedPeriodAndRegion_DefinedUnitRegion() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);

      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

      assertTrue(uregion.isDefined());
   }

   @Test
   public void testUnitRegionConst_UndefinedPeriodOrRegion_UndefinedUnitRegion() {

      Period definedPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      Period undefinedPeriod = new Period();
      Region undefinedRegion = new Region(false);

      UnitRegionIF uregion1 = new UnitRegionConst(undefinedPeriod, simpleRegion);
      UnitRegionIF uregion2 = new UnitRegionConst(definedPeriod, undefinedRegion);

      assertFalse(uregion1.isDefined());
      assertFalse(uregion2.isDefined());
   }

   @Test
   public void testGetValue_AtValidTimeInstant_ShouldReturnDefinedRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", true, true);
      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

      TimeInstantIF instant1 = new TimeInstant("2018-01-01 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant3 = new TimeInstant("2018-01-09 23:59:59:999");

      assertTrue(uregion.getValue(instant1).isDefined());
      assertTrue(uregion.getValue(instant2).isDefined());
      assertTrue(uregion.getValue(instant3).isDefined());

   }

   @Test
   public void testGetValue_AtInvalidInstant_ShouldReturnUndefinedRegion() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-09 23:59:59:999", false, false);
      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

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

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

      RegionIF region = uregion.getInitial();

      assertTrue(region.isDefined());
      assertEquals(1, region.getNoComponents());
   }

   @Test
   public void testGetFinal() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

      RegionIF region = uregion.getFinal();

      assertTrue(region.isDefined());
      assertEquals(1, region.getNoComponents());
   }

   @Test
   public void testGetProjectionBoundingBox() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      UnitRegionIF uregion = new UnitRegionConst(period, simpleRegion);

      RectangleIF expectedRectangle = simpleRegion.getBoundingBox();

      assertEquals(expectedRectangle, uregion.getProjectionBoundingBox());
   }

   @Test
   public void testAtPeriod_IntersectingPeriod() {

      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      UnitRegionIF oldURegion = new UnitRegionConst(period, simpleRegion);

      Period newPeriod = new Period("2018-01-05 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitRegionIF newURegion = oldURegion.atPeriod(newPeriod);

      assertEquals(period.intersection(newPeriod), newURegion.getPeriod());
      assertTrue(newURegion.isDefined());

   }

   @Test
   public void testAtPeriod_NoIntersectingPeriod() {
      Period period = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      UnitRegionIF oldURegion = new UnitRegionConst(period, simpleRegion);

      Period newPeriod = new Period("2018-01-15 12:00:00:000", "2018-01-20 00:00:00:000", true, true);

      UnitRegionIF newURegion = oldURegion.atPeriod(newPeriod);

      assertFalse(newURegion.isDefined());

   }

   @Test
   public void testGetMovingSegments() {

      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      UnitRegionConst uregion1 = new UnitRegionConst(new Region(face));

      List<PointIF> holePoints = new ArrayList<>();

      holePoints.add(new Point(2.0d, 2.0d));
      holePoints.add(new Point(3.0d, 3.0d));
      holePoints.add(new Point(3.0d, 2.0d));

      face = new Face(points);

      face.add(new Cycle(holePoints));

      UnitRegionConst uregion2 = new UnitRegionConst(new Region(face));

      assertEquals(3, uregion1.getMovingSegments().size());
      assertEquals(6, uregion2.getMovingSegments().size());
   }

   @Test
   public void testGetNoMovingFaces() {
      UnitRegionConst uregion1 = new UnitRegionConst(simpleRegion);

      int noFacesBeforeAdd = uregion1.getNoMovingFaces();

      List<PointIF> points = new ArrayList<>();

      points.add(new Point(20.0d, 20.0d));
      points.add(new Point(40.0d, 40.0d));
      points.add(new Point(40.0d, 20.0d));

      Face face = new Face(points);

      simpleRegion.add(face);

      UnitRegionConst uregion2 = new UnitRegionConst(simpleRegion);

      int noFacesAfterAdd = uregion2.getNoMovingFaces();

      assertEquals(1, noFacesBeforeAdd);
      assertEquals(2, noFacesAfterAdd);
   }
}
