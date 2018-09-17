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

import org.junit.BeforeClass;
import org.junit.Test;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Segment;
import mol.datatypes.time.TimeInstant;

/**
 * Tests for the 'MovableSegment' class
 * 
 * @author Markus Fuessel
 */
public class MovableSegmentTest {

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Test
   public void testGetValue_MovableSegmentWithUnchangedPositionAndSize_ValidInstant() {
      Point initStartPoint = new Point(0.0d, 0.0d);
      Point initEndPoint = new Point(10.0d, 0.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 0.0d);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstant instant = new TimeInstant("2018-01-05 00:00:00:000");

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(initStartPoint, initEndPoint);

      Segment segment = msegment.getValue(movementPeriod, instant);

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

   @Test
   public void testGetValue_MovableSegmentWithUnchangedPositionAndSize_InvalidInstant() {
      Point initStartPoint = new Point(0.0d, 0.0d);
      Point initEndPoint = new Point(10.0d, 0.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 0.0d);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstant instant = new TimeInstant("2018-01-11 00:00:00:000");

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment segment = msegment.getValue(movementPeriod, instant);

      assertFalse(segment.isDefined());

   }

   @Test
   public void testGetValue_MovableSegmentChangedPositionAndSize_ValidInstant() {
      Point initStartPoint = new Point(0.0d, 10.0d);
      Point initEndPoint = new Point(0.0d, 10.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 10.0d);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstant instant = movementPeriod.getLowerBound().plusMillis(movementPeriod.getDurationInMilliseconds() / 2);

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(0.0d, 5.0d, 5.0d, 10.0d);

      Segment segment = msegment.getValue(movementPeriod, instant);

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

   @Test
   public void testGetValue_MovableSegmentChangedPositionAndSize_InstantAtLowerEnd() {
      Point initStartPoint = new Point(0.0d, 10.0d);
      Point initEndPoint = new Point(0.0d, 10.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 10.0d);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstant instant = movementPeriod.getLowerBound();

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(initStartPoint, initEndPoint);

      Segment segment = msegment.getValue(movementPeriod, instant);

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

   @Test
   public void testGetValue_MovableSegmentChangedPositionAndSize_InstantAtUpperEnd() {
      Point initStartPoint = new Point(0.0d, 10.0d);
      Point initEndPoint = new Point(0.0d, 10.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 10.0d);

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstant instant = movementPeriod.getUpperBound();

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(finalStartPoint, finalEndPoint);

      Segment segment = msegment.getValue(movementPeriod, instant);

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

   @Test
   public void testGetInitial_GetValidSegment() {
      Point initStartPoint = new Point(0.0d, 10.0d);
      Point initEndPoint = new Point(0.0d, 10.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 10.0d);

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(initStartPoint, initEndPoint);
      Segment segment = msegment.getInitial();

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

   @Test
   public void testGetFinal_GetValidSegment() {
      Point initStartPoint = new Point(0.0d, 10.0d);
      Point initEndPoint = new Point(0.0d, 10.0d);
      Point finalStartPoint = new Point(0.0d, 0.0d);
      Point finalEndPoint = new Point(10.0d, 10.0d);

      MovableSegment msegment = new MovableSegment(initStartPoint, initEndPoint, finalStartPoint, finalEndPoint);

      Segment expectedSegment = new Segment(finalStartPoint, finalEndPoint);
      Segment segment = msegment.getFinal();

      assertTrue(segment.isDefined());
      assertEquals(expectedSegment, segment);

   }

}
