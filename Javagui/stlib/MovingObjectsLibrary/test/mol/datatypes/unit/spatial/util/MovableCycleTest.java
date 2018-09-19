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

package mol.datatypes.unit.spatial.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.interfaces.spatial.util.CycleIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * Tests for the 'MovableCycle' class
 * 
 * @author Markus Fuessel
 */
public class MovableCycleTest {

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Test
   public void testGetValue_MovableCycleWithUnchangedPositionAndSize_ValidInstant() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, iP0, iP1));
      msegments.add(new MovableSegment(iP1, iP2, iP1, iP2));
      msegments.add(new MovableSegment(iP2, iP3, iP2, iP3));
      msegments.add(new MovableSegment(iP3, iP0, iP3, iP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = new TimeInstant("2018-01-05 00:00:00:000");

      CycleIF cycle = mcycle.getValue(movementPeriod, instant);

      assertTrue(cycle.isDefined());
      assertEquals(8, cycle.getHalfsegments().size());
   }

   @Test
   public void testGetValue_MovableCycleWithUnchangedPositionAndSize_InvalidInstant() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, iP0, iP1));
      msegments.add(new MovableSegment(iP1, iP2, iP1, iP2));
      msegments.add(new MovableSegment(iP2, iP3, iP2, iP3));
      msegments.add(new MovableSegment(iP3, iP0, iP3, iP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = new TimeInstant("2018-01-11 00:00:00:000");

      CycleIF cycle = mcycle.getValue(movementPeriod, instant);

      assertFalse(cycle.isDefined());
   }

   @Test
   public void testGetValue_MovableCycleChangedPositionAndSize_ValidInstant() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = new TimeInstant("2018-01-05 12:00:00:000");

      CycleIF cycle = mcycle.getValue(movementPeriod, instant);

      assertTrue(cycle.isDefined());
      assertEquals(10, cycle.getHalfsegments().size());

   }

   @Test
   public void testGetValue_MovableCycleChangedPositionAndSize_InstantAtLowerEnd() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = movementPeriod.getLowerBound();

      CycleIF cycle = mcycle.getValue(movementPeriod, instant);

      assertTrue(cycle.isDefined());
      assertEquals(8, cycle.getHalfsegments().size());

   }

   @Test
   public void testGetValue_MovableCycleChangedPositionAndSize_InstantAtUpperEnd() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = movementPeriod.getUpperBound();

      CycleIF cycle = mcycle.getValue(movementPeriod, instant);

      assertTrue(cycle.isDefined());
      assertEquals(6, cycle.getHalfsegments().size());

   }

   @Test
   public void testGetValue_MovableCycleChangedPositionAndSize_UndefinedCycleAtEndOfMovement() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(5.0d, 5.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP2, fP0, fP0));
      msegments.add(new MovableSegment(iP2, iP3, fP0, fP0));
      msegments.add(new MovableSegment(iP3, iP0, fP0, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant1 = new TimeInstant("2018-01-05 00:00:00:000");
      TimeInstantIF instant2 = new TimeInstant("2018-01-08 00:00:00:000");
      TimeInstantIF instant3 = movementPeriod.getUpperBound();

      CycleIF cycle1 = mcycle.getValue(movementPeriod, instant1);
      CycleIF cycle2 = mcycle.getValue(movementPeriod, instant2);
      CycleIF cycle3 = mcycle.getValue(movementPeriod, instant3);

      assertTrue(cycle1.isDefined());
      assertTrue(cycle2.isDefined());
      assertFalse(cycle3.isDefined());
   }

   @Test
   public void testGetInitial() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      CycleIF cycle = mcycle.getInitial();

      assertTrue(cycle.isDefined());
      assertEquals(8, cycle.getHalfsegments().size());

   }

   @Test
   public void testGetFinal() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      CycleIF cycle = mcycle.getFinal();

      assertTrue(cycle.isDefined());
      assertEquals(6, cycle.getHalfsegments().size());

   }

   @Test
   public void testGetProjectionBoundingBox_MovableCycleChangedPositionAndSize() {
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycle mcycle = new MovableCycle(msegments);

      RectangleIF expectedPBB = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      RectangleIF currentPBB = mcycle.getProjectionBoundingBox();

      assertTrue(currentPBB.isDefined());
      assertEquals(expectedPBB, currentPBB);

   }
}
