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

package stlib.datatypes.spatial.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.util.Cycle;
import stlib.datatypes.spatial.util.Rectangle;
import stlib.datatypes.spatial.util.Segment;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.spatial.util.SegmentIF;

/**
 * Tests for class 'Cycle'
 * 
 * @author Markus Fuessel
 */
public class CycleTest {

   @Test
   public void testCycle_ShouldBeEmptyAndDefined() {
      Cycle cycle = new Cycle(true);

      assertTrue(cycle.isEmpty());
      assertTrue(cycle.isDefined());
      assertFalse(cycle.getBoundingBox().isDefined());
   }

   @Test
   public void testCycle_WithMoreThenTwoPoints_ShouldBeNotEmptyAndDefined() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Cycle cycle = new Cycle(points);

      assertFalse(cycle.isEmpty());
      assertTrue(cycle.isDefined());
   }

   @Test
   public void testCycle_WithLessThenThreePoints_ShouldBeEmptyAndUndefined() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(10.0d, 0.0d));

      Cycle cycle = new Cycle(points);

      assertTrue(cycle.isEmpty());
      assertFalse(cycle.isDefined());
   }

   @Test
   public void testSetCycleBySegmentList_WithMoreThenTwoSegments_ShouldBeNotEmptyAndDefined() {
      List<SegmentIF> segments = new ArrayList<>();

      segments.add(new Segment(0.0d, 0.0d, 5.0d, 5.0d));
      segments.add(new Segment(5.0d, 5.0d, 10.0d, 0.0d));
      segments.add(new Segment(10.0d, 0.0d, 0.0d, 0.0d));

      Cycle cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      assertFalse(cycle.isEmpty());
      assertTrue(cycle.isDefined());
   }

   @Test
   public void testSetCycleBySegmentList_WithLessThenThreeSegments_ShouldBeEmptyAndUndefined() {
      List<SegmentIF> segments = new ArrayList<>();

      segments.add(new Segment(0.0d, 0.0d, 5.0d, 5.0d));
      segments.add(new Segment(5.0d, 5.0d, 10.0d, 0.0d));

      Cycle cycle = new Cycle(false);

      cycle.setCycleBySegmentList(segments);

      assertTrue(cycle.isEmpty());
      assertFalse(cycle.isDefined());
   }

   @Test
   public void testSetCycleBySegmentList_ExistingSegmentsShouldBeReplaced() {
      List<SegmentIF> segments = new ArrayList<>();

      segments.add(new Segment(0.0d, 0.0d, 5.0d, 5.0d));
      segments.add(new Segment(5.0d, 5.0d, 10.0d, 0.0d));
      segments.add(new Segment(10.0d, 0.0d, 0.0d, 0.0d));

      Cycle cycle = new Cycle(false);
      cycle.setCycleBySegmentList(segments);

      RectangleIF bbBeforeNewSet = cycle.getBoundingBox();
      int sizeBeforeNewSet = cycle.getHalfsegments().size();

      segments.clear();
      segments.add(new Segment(0.0d, 0.0d, 0.0d, 15.0d));
      segments.add(new Segment(0.0d, 15.0d, 15.0d, 15.0d));
      segments.add(new Segment(15.0d, 15.0d, 15.0d, 0.0d));
      segments.add(new Segment(15.0d, 0.0d, 0.0d, 0.0d));

      cycle.setCycleBySegmentList(segments);

      RectangleIF bbAfterNewSet = cycle.getBoundingBox();
      int sizeAfterNewSet = cycle.getHalfsegments().size();

      assertFalse(cycle.isEmpty());
      assertTrue(cycle.isDefined());
      assertNotEquals(bbBeforeNewSet, bbAfterNewSet);
      assertNotEquals(sizeBeforeNewSet, sizeAfterNewSet);
   }

   @Test
   public void testGetBoundingBox() {

      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Cycle cycle = new Cycle(points);
      RectangleIF expectedBB = new Rectangle();

      for (PointIF point : points) {
         expectedBB = expectedBB.merge(point.getBoundingBox());
      }

      assertEquals(expectedBB, cycle.getBoundingBox());
   }

   @Test
   public void testIsEmpty_EmptyCycle_ShouldBeEmpty() {
      List<PointIF> points = new ArrayList<>();

      Cycle cycle = new Cycle(points);

      assertTrue(cycle.isEmpty());
   }

   @Test
   public void testLength_EmptyCycle_ShouldBeZero() {
      List<PointIF> points = new ArrayList<>();

      Cycle cycle = new Cycle(points);

      assertTrue(cycle.isEmpty());
      assertEquals(0.0d, cycle.length(false), 0.0d);
   }

   @Test
   public void testLength_EuclideanGeometry() {

      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(10.0d, 0.0d));
      points.add(new Point(10.0d, 10.0d));
      points.add(new Point(0.0d, 10.0d));

      Cycle cycle = new Cycle(points);

      double expected = 40.0d;

      assertEquals(expected, cycle.length(false), 0.0d);
   }

   @Test
   public void testLength_SphericalGeometry() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(52.527115d, 13.415982d));
      points.add(new Point(52.550944d, 13.430201d));
      points.add(new Point(52.553546d, 13.414743d));
      points.add(new Point(52.541120d, 13.412416d));

      Cycle cycle = new Cycle(points);

      double expected = 6878.31816d;

      assertEquals(expected, cycle.length(true), 1.0d);
   }

}
