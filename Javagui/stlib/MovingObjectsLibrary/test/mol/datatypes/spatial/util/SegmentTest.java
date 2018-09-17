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
package mol.datatypes.spatial.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import mol.datatypes.spatial.Point;
import mol.util.GeneralHelper;

/**
 * Tests for class 'Segment'
 * 
 * @author Markus Fuessel
 *
 */
public class SegmentTest {

   @Test
   public void testSegment_WithoutValue_ShouldBeUndefinedAndEmpty() {

      Segment segment = new Segment();

      assertFalse(segment.isDefined());
      assertTrue(segment.isEmpty());

   }

   @Test
   public void testSegment_LowerPointGreaterPoint() {
      Point p0 = new Point(1.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);

      Segment segment = new Segment(p0, p1);

      assertEquals(p0, segment.getLeftPoint());
      assertEquals(p1, segment.getRightPoint());

   }

   @Test
   public void testSegment_GreaterPointLowerPoint() {
      Point p0 = new Point(2.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);

      Segment segment = new Segment(p0, p1);

      assertEquals(p1, segment.getLeftPoint());
      assertEquals(p0, segment.getRightPoint());
   }

   @Test
   public void testSegment_UndefinedPoint_SegmentShouldBeUndefined() {
      Point p0 = new Point();
      Point p1 = new Point(1.0d, 7.0d);

      Segment segment = new Segment(p0, p1);

      assertFalse(segment.isDefined());
   }

   @Test
   public void testSegment_DoubleValues() {
      double x0 = 1.0d;
      double y0 = 5.0d;
      double x1 = 1.5d;
      double y1 = 7.0d;

      Segment segment = new Segment(x0, y0, x1, y1);

      assertEquals(new Point(x0, y0), segment.getLeftPoint());
      assertEquals(new Point(x1, y1), segment.getRightPoint());

   }

   @Test
   public void testIntersect_IntersectingSegments_ShouldBeTrue() {
      Segment segment1 = new Segment(5.0d, 0.0d, 5.0d, 10.0d);
      Segment segment2 = new Segment(0.0d, 5.0d, 10.0d, 5.0d);

      assertTrue(segment1.intersect(segment2));
      assertTrue(segment2.intersect(segment1));
   }

   @Test
   public void testIntersect_IntersectLeftEndpoint_ShouldBeTrue() {
      Segment segment1 = new Segment(0.0d, 5.0d, 10.0d, 5.0d);
      Segment segment2 = new Segment(0.0d, 0.0d, 0.0d, 10.0d);

      assertTrue(segment1.intersect(segment2));
      assertTrue(segment2.intersect(segment1));
   }

   @Test
   public void testIntersect_IntersectRightEndpoint_ShouldBeFalse() {
      Segment segment1 = new Segment(0.0d, 5.0d, 10.0d, 5.0d);
      Segment segment2 = new Segment(10.0d, 0.0d, 10.0d, 10.0d);

      assertFalse(segment1.intersect(segment2));
      assertFalse(segment2.intersect(segment1));
   }

   @Test
   public void testIsVertical_VerticalSegment_ShouldBeTrue() {
      Segment segment = new Segment(0.0d, 0.0d, 0.0d, 10.0d);

      assertTrue(segment.isVertical());
   }

   @Test
   public void testIsVertical_NonVerticalSegment_ShouldBeFalse() {
      Segment segment = new Segment(0.0d, 0.0d, 0.0001d, 10.0d);

      assertFalse(segment.isVertical());
   }

   @Test
   public void testIsAlmostAPoint_SegmentWithVeryCloseEndPoints_ShouldBeTrue() {
      Segment segment = new Segment(0.0d, 0.0d, 0.0d + (GeneralHelper.getEpsilon() / 10),
            0.0d - (GeneralHelper.getEpsilon() / 10));

      assertTrue(segment.isAlmostAPoint());
   }

   @Test
   public void testIsAlmostAPoint_SegmentWithNotCloseEndPoints_ShouldBeFalse() {
      Segment segment = new Segment(0.0d, 0.0d, 0.0d + (GeneralHelper.getEpsilon() * 1.1),
            0.0d - (GeneralHelper.getEpsilon() * 1.1));

      assertFalse(segment.isAlmostAPoint());
   }

   @Test
   public void testGetBoundingBox() {
      Point p0 = new Point(1.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);
      Rectangle expectedPBB = p0.getBoundingBox().merge(p1.getBoundingBox());

      Segment segment = new Segment(p0, p1);

      assertEquals(expectedPBB, segment.getBoundingBox());
   }

   @Test
   public void testEquals_SegmentWithEqualCoordinates_ShouldBeTrue() {

      Segment segment1 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);
      Segment segment2 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);

      assertTrue(segment1.equals(segment2));
   }

   @Test
   public void testEquals_SegmentWithNonEqualCoordinates_ShouldBeFalse() {

      Segment segment1 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);
      Segment segment2 = new Segment(1.0d, 5.0d, 1.1d, 7.0d);

      assertFalse(segment1.equals(segment2));
   }

   @Test
   public void testEquals_SameObject_ShouldBeTrue() {
      Point p0 = new Point(1.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);
      Segment segment = new Segment(p0, p1);
      Object obj = segment;

      assertTrue(segment.equals(obj));
   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      Point p0 = new Point(1.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);
      Object obj = null;

      Segment segment = new Segment(p0, p1);

      assertFalse(segment.equals(obj));
   }

   @Test
   public void testEquals_UndefinedSegment_ShouldBeFalse() {

      Segment segment1 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);
      Segment segment2 = new Segment(new Point(), new Point());

      assertFalse(segment1.equals(segment2));
      assertFalse(segment2.equals(segment1));
   }

   @Test
   public void testHashCode_SegmentWithEqualCoordinates_HashCodeShoulBeEqual() {

      Segment segment1 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);
      Segment segment2 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);

      assertTrue(segment1.hashCode() == segment2.hashCode());
   }

   @Test
   public void testHashCode_SegmentWithNonEqualCoordinates_HashCodeShoulBeNonEqual() {

      Segment segment1 = new Segment(1.0d, 5.0d, 1.0d, 7.0d);
      Segment segment2 = new Segment(1.0d, 5.0d, 1.0d, 7.1d);

      assertFalse(segment1.hashCode() == segment2.hashCode());
   }

   @Test
   public void testLength_EuclideanSpace() {
      Point p0 = new Point(0.0d, 0.0d);
      Point p1 = new Point(10.0d, 0.0d);

      double expectedLength = 10.0d;

      Segment segment = new Segment(p0, p1);

      assertEquals(expectedLength, segment.length(false), 0.0d);
   }

   @Test
   public void testLength_SphericalSpace() {
      Point p0 = new Point(52.527115d, 13.415982d);
      Point p1 = new Point(52.550944d, 13.430201d);

      double expectedLength = 2821.70676d;

      Segment segment = new Segment(p0, p1);

      assertEquals(expectedLength, segment.length(true), 1.0d);
   }

}
