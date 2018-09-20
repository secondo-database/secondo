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
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import stlib.datatypes.spatial.Point;
import stlib.datatypes.spatial.util.Halfsegment;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.RectangleIF;

/**
 * Tests for the 'Halfsegment' class
 * 
 * @author Markus Fuessel
 */
public class HalfsegmentTest {

   @Test
   public void testHalfsegment_WithoutValue_ShouldBeUndefinedAndEmpty() {

      Halfsegment hs = new Halfsegment();

      assertFalse(hs.isDefined());
      assertTrue(hs.isEmpty());

   }

   @Test
   public void testGetBoundingBox() {
      Point p0 = new Point(1.0d, 5.0d);
      Point p1 = new Point(1.0d, 7.0d);
      RectangleIF expectedPBB = p0.getBoundingBox().merge(p1.getBoundingBox());

      Halfsegment hs = new Halfsegment(p0, p1, true);

      assertEquals(expectedPBB, hs.getBoundingBox());
   }

   @Test
   public void testCompareTo_DifferentDominatingPoint() {

      Halfsegment lowerHS = new Halfsegment(0.0d, 0.0d, 10.0d, 10.0d, true);
      Halfsegment greaterHS = new Halfsegment(2.0d, 2.0d, 12.0d, 12.0d, true);

      assertEquals(-1, lowerHS.compareTo(greaterHS));
      assertEquals(1, greaterHS.compareTo(lowerHS));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_BothDPLeft() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.01d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.01d, -10.0d, true);

      assertEquals(1, hs1.compareTo(hs2));
      assertEquals(-1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualHalfsegments() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.01d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.01d, 10.0d, true);

      assertEquals(0, hs1.compareTo(hs2));
      assertEquals(0, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_BothDPLeft_HS1Vertical() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.00d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.01d, -10.0d, true);

      assertEquals(1, hs1.compareTo(hs2));
      assertEquals(-1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_BothDPRight() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, -0.01d, 10.0d, false);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, -0.01d, -10.0d, false);

      assertEquals(-1, hs1.compareTo(hs2));
      assertEquals(1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_BothDPRight_HS2Vertical() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, -0.01d, 10.0d, false);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.0d, -10.0d, false);

      assertEquals(-1, hs1.compareTo(hs2));
      assertEquals(1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_LeftAndRightDP() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.01d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, -0.01d, -10.0d, false);

      assertEquals(1, hs1.compareTo(hs2));
      assertEquals(-1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_LeftAndRightDP_HS1Vertical() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.00d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, -0.01d, -10.0d, false);

      assertEquals(1, hs1.compareTo(hs2));
      assertEquals(-1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_LeftAndRightDP_HS1HS2Vertical() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, 0.0d, 10.0d, true);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.0d, -10.0d, false);

      assertEquals(1, hs1.compareTo(hs2));
      assertEquals(-1, hs2.compareTo(hs1));
   }

   @Test
   public void testCompareTo_EqualDominatingPoint_RightAndLeftDP() {

      Halfsegment hs1 = new Halfsegment(0.0d, 0.0d, -0.01d, 10.0d, false);
      Halfsegment hs2 = new Halfsegment(0.0d, 0.0d, 0.01d, -10.0d, true);

      assertEquals(-1, hs1.compareTo(hs2));
      assertEquals(1, hs2.compareTo(hs1));
   }

   @Test
   public void testHashCode_SegmentWithEqualCoordinates_HashCodeShoulBeEqual() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);

      assertTrue(hs1.hashCode() == hs2.hashCode());
   }

   @Test
   public void testHashCode_SegmentWithNonEqualCoordinates_HashCodeShoulBeNonEqual() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.1d, true);

      assertFalse(hs1.hashCode() == hs2.hashCode());
   }

   @Test
   public void testEquals_SegmentWithEqualCoordinates_ShouldBeTrue() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);

      assertTrue(hs1.equals(hs2));
   }

   @Test
   public void testEquals_SegmentWithNonEqualCoordinates_ShouldBeFalse() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(1.0d, 5.0d, 1.1d, 7.0d, true);

      assertFalse(hs1.equals(hs2));
   }

   @Test
   public void testEquals_SegmentWithDifferenDominatingPoint_ShouldBeFalse() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, false);

      assertFalse(hs1.equals(hs2));
   }

   @Test
   public void testEquals_SameObject_ShouldBeTrue() {

      HalfsegmentIF hs = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);

      Object obj = hs;

      assertTrue(hs.equals(obj));
   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {

      HalfsegmentIF hs = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);

      Object obj = null;

      assertFalse(hs.equals(obj));
   }

   @Test
   public void testEquals_UndefinedSegment_ShouldBeFalse() {

      HalfsegmentIF hs1 = new Halfsegment(1.0d, 5.0d, 1.0d, 7.0d, true);
      HalfsegmentIF hs2 = new Halfsegment(new Point(), new Point(), true);

      assertFalse(hs1.equals(hs2));
      assertFalse(hs2.equals(hs1));
   }

}
