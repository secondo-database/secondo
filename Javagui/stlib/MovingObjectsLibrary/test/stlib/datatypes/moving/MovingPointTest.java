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
import java.util.List;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import stlib.datatypes.interval.Period;
import stlib.datatypes.intime.Intime;
import stlib.datatypes.moving.MovingPoint;
import stlib.datatypes.spatial.Point;
import stlib.datatypes.time.TimeInstant;
import stlib.datatypes.unit.spatial.UnitPointConst;
import stlib.datatypes.unit.spatial.UnitPointLinear;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Tests for the 'MovingPoint' class
 * 
 * @author Markus Fuessel
 */
public class MovingPointTest {

   private MovingPoint mpointContinuos;

   private MovingPoint mpointDiscrete;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Before
   public void setUp() throws Exception {
      mpointContinuos = new MovingPoint(0);
      mpointDiscrete = new MovingPoint(0);

      Point p1 = new Point(0.0d, 0.0d);
      Point p2 = new Point(10.0d, 10.0d);
      Point p3 = new Point(10.0d, 20.0d);
      Point p4 = new Point(20.0d, 5.0d);

      mpointContinuos.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), p1, p2);
      mpointContinuos.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), p2, p3);
      mpointContinuos.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), p3, p4);

      mpointDiscrete.add(new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false), p1, p1);
      mpointDiscrete.add(new Period("2018-01-10 00:00:00:000", "2018-01-20 00:00:00:000", true, false), p2, p2);
      mpointDiscrete.add(new Period("2018-01-20 00:00:00:000", "2018-01-30 00:00:00:000", true, false), p3, p3);
   }

   @Test
   public void testMovingPoint_WithListIntimePoints() {
      List<Intime<PointIF>> ipoints = new ArrayList<>();

      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-01 00:00:00:000"), new Point(0.0d, 0.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-10 00:00:00:000"), new Point(10.0d, 10.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-20 00:00:00:000"), new Point(10.0d, 20.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-30 00:00:00:000"), new Point(20.0d, 5.0d)));

      MovingPoint mpointIP = new MovingPoint(ipoints);

      assertTrue(mpointIP.isDefined());
      assertTrue(mpointIP.isClosed());
      assertEquals(3, mpointIP.getNoUnits());
   }

   @Test
   public void testMovingPoint_Point() {

      MovingPoint mpoint = new MovingPoint(new Point(5.0, 5.0));

      assertTrue(mpoint.isDefined());
      assertEquals(1, mpoint.getNoUnits());
   }

   @Test
   public void testAdd_RightAdjacentUnitPoint_SuccessfulAppend() {
      int sizeBefore = mpointContinuos.getNoUnits();

      UnitPointIF upoint = new UnitPointLinear(
            new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(20.0d, 5.0d),
            new Point(20.0d, 20.0d));

      mpointContinuos.add(upoint);

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_UnitPointWithLaterPeriod_SuccessfulAppend() {
      int sizeBefore = mpointContinuos.getNoUnits();

      UnitPointIF upoint = new UnitPointLinear(
            new Period("2018-02-15 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(20.0d, 5.0d),
            new Point(20.0d, 20.0d));

      mpointContinuos.add(upoint);

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_UnitPoint_FailedAppend() {
      int sizeBefore = mpointContinuos.getNoUnits();

      UnitPointIF upoint = new UnitPointLinear(
            new Period("2018-01-15 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(20.0d, 5.0d),
            new Point(20.0d, 20.0d));

      boolean successful = mpointContinuos.add(upoint);

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testAdd_SuccessfulAppend() {
      int sizeBefore = mpointContinuos.getNoUnits();

      mpointContinuos.add(new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false),
            new Point(20.0d, 5.0d), new Point(20.0d, 20.0d));

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore + 1, sizeAfter);
   }

   @Test
   public void testAdd_EqualStartAndEndPoint_SuccessfulAppendConstantPoint() {
      int sizeBefore = mpointContinuos.getNoUnits();

      mpointContinuos.add(new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false),
            new Point(20.0d, 20.0d), new Point(20.0d, 20.0d));

      int sizeAfter = mpointContinuos.getNoUnits();

      UnitPointIF upoint = mpointContinuos.getUnit(sizeAfter - 1);

      assertEquals(sizeBefore + 1, sizeAfter);
      assertTrue(upoint instanceof UnitPointConst);
   }

   @Test
   public void testAdd_ConstantPointsWithAdjacentTimeperiod_SuccessfulWithoutIncreaseNumberOfUnits() {
      int sizeBefore = mpointDiscrete.getNoUnits();

      UnitPointIF lastUP = mpointDiscrete.getUnit(sizeBefore - 1);

      boolean successful = mpointDiscrete.add(
            new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false), lastUP.getFinal(),
            lastUP.getFinal());

      int sizeAfter = mpointDiscrete.getNoUnits();

      assertTrue(successful);
      assertEquals(sizeBefore, sizeAfter);
   }

   @Test
   public void testAdd_IntersectingPeriod_AppendFailed() {
      int sizeBefore = mpointContinuos.getNoUnits();

      boolean successful = mpointContinuos.add(
            new Period("2018-01-15 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(20.0, 5.0),
            new Point(20.0, 20.0));

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testAdd_UndefinedValue_AppendFailed() {
      int sizeBefore = mpointContinuos.getNoUnits();

      boolean successful = mpointContinuos.add(new Period(), new Point(20.0, 5.0), new Point(20.0, 20.0));

      int sizeAfter = mpointContinuos.getNoUnits();

      assertEquals(sizeBefore, sizeAfter);
      assertFalse(successful);

   }

   @Test
   public void testAdd_UnitWithEqualFinalInitialValuePeriodIntersectOnBorder_Successful() {
      List<Intime<PointIF>> ipoints = new ArrayList<>();

      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-01 00:00:00:000"), new Point(0.0d, 0.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-10 00:00:00:000"), new Point(10.0d, 10.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-20 00:00:00:000"), new Point(10.0d, 20.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-30 00:00:00:000"), new Point(20.0d, 5.0d)));

      MovingPoint mpointIP = new MovingPoint(ipoints);

      UnitPointIF oldLastUnit = mpointIP.getUnit(mpointIP.getNoUnits() - 1);

      boolean isClosedBeforeAdd = oldLastUnit.getPeriod().isRightClosed();

      boolean successfulAdd = mpointIP.add(
            new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(20.0, 5.0),
            new Point(20.0, 20.0));

      boolean isClosedAfterAdd = oldLastUnit.getPeriod().isRightClosed();

      assertTrue(isClosedBeforeAdd);
      assertTrue(successfulAdd);
      assertFalse(isClosedAfterAdd);
      assertEquals(4, mpointIP.getNoUnits());
   }

   @Test
   public void testAdd_UnitWithNonEqualFinalInitialValuePeriodIntersectOnBorder_Fail() {
      List<Intime<PointIF>> ipoints = new ArrayList<>();

      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-01 00:00:00:000"), new Point(0.0d, 0.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-10 00:00:00:000"), new Point(10.0d, 10.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-20 00:00:00:000"), new Point(10.0d, 20.0d)));
      ipoints.add(new Intime<PointIF>(new TimeInstant("2018-01-30 00:00:00:000"), new Point(20.0d, 5.0d)));

      MovingPoint mpointIP = new MovingPoint(ipoints);

      UnitPointIF oldLastUnit = mpointIP.getUnit(mpointIP.getNoUnits() - 1);

      boolean isClosedBeforeAdd = oldLastUnit.getPeriod().isRightClosed();

      boolean successfulAdd = mpointIP.add(
            new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false), new Point(21.0, 5.0),
            new Point(20.0, 20.0));

      boolean isClosedAfterAdd = oldLastUnit.getPeriod().isRightClosed();

      assertTrue(isClosedBeforeAdd);
      assertFalse(successfulAdd);
      assertTrue(isClosedAfterAdd);
      assertEquals(3, mpointIP.getNoUnits());
   }

   @Test
   public void testGetObjectPBB_VerifyPBBAfterAddingUnit() {

      Point p1 = new Point(20.0d, 5.0d);
      Point p2 = new Point(20.0d, 20.0d);
      RectangleIF objPBBBefore = mpointContinuos.getProjectionBoundingBox();
      RectangleIF expectedObjPBBAfter = objPBBBefore.merge(p1.getBoundingBox()).merge(p2.getBoundingBox());

      mpointContinuos.add(new Period("2018-01-30 00:00:00:000", "2018-02-20 00:00:00:000", true, false), p1, p2);

      assertEquals(expectedObjPBBAfter, mpointContinuos.getProjectionBoundingBox());
   }

   @Test
   public void testGetValue_ValidInstant_ShouldReturnValidPoint() {
      PointIF point = mpointContinuos.getValue(new TimeInstant("2018-01-20 00:00:00:000"));

      assertEquals(new Point(10.0, 20.0), point);
   }

   @Test
   public void testGetValue_UndefinedInstant_ShouldReturnUndefinedPoint() {
      PointIF point = mpointContinuos.getValue(new TimeInstant());

      assertFalse(point.isDefined());
   }

   @Test
   public void testGetUnit_ValidPosition_ShouldReturnDefinedUnit() {

      UnitPointIF upoint = mpointContinuos.getUnit(mpointContinuos.getNoUnits() - 1);

      assertTrue(upoint.isDefined());

   }

   @Test
   public void testGetUnit_InvalidPosition_ShouldReturnUndefinedUnit() {

      UnitPointIF upoint = mpointContinuos.getUnit(mpointContinuos.getNoUnits());

      assertFalse(upoint.isDefined());

   }

   @Test
   public void testGetUnitPosition_ValidInstant_ShouldReturnValidUnitPosition() {
      TimeInstant instant = new TimeInstant("2018-01-20 00:00:00:000");

      int unitPos = mpointContinuos.getUnitPosition(instant);

      assertEquals(2, unitPos);

   }

   @Test
   public void testGetUnitPosition_InstantOutsideOfPeriods_ShouldReturnInvalidUnitPosition() {
      TimeInstant instant = new TimeInstant("2017-01-01 00:00:00:000");

      int unitPos = mpointContinuos.getUnitPosition(instant);

      assertEquals(-1, unitPos);

   }

   @Test
   public void testGetUnitPosition_UndefinedInstant_ShouldReturnInvalidUnitPosition() {
      TimeInstant instant = new TimeInstant();

      int unitPos = mpointContinuos.getUnitPosition(instant);

      assertEquals(-1, unitPos);

   }
}
