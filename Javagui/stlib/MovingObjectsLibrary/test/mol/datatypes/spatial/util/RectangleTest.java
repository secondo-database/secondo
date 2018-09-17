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
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

/**
 * Tests for the 'Rectangle' class
 * 
 * @author Markus Fuessel
 *
 */
public class RectangleTest {

   @Test
   public void testRectangleConstructor_UndefinedObject() {
      Rectangle undefinedRect = new Rectangle();

      assertFalse(undefinedRect.isDefined());
   }

   @Test
   public void testRectangle() {
      double leftValue = 1.2d;
      double rightValue = 2.3d;
      double topValue = 3.4d;
      double bottomValue = 0.5d;

      Rectangle rect = new Rectangle(leftValue, rightValue, topValue, bottomValue);

      assertEquals(leftValue, rect.getLeftValue(), 0.0f);
      assertEquals(rightValue, rect.getRightValue(), 0.0f);
      assertEquals(topValue, rect.getTopValue(), 0.0f);
      assertEquals(bottomValue, rect.getBottomValue(), 0.0f);
   }

   @Test
   public void testCopy() {
      double leftValue = 1.2d;
      double rightValue = 2.3d;
      double topValue = 3.4d;
      double bottomValue = 0.5d;

      Rectangle rect = new Rectangle(leftValue, rightValue, topValue, bottomValue);

      Rectangle copyRect = rect.copy();

      assertEquals(rect, copyRect);
   }

   @Test
   public void testMerge() {

      Rectangle rect1 = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);
      Rectangle rect2 = new Rectangle(-1.2d, 5.11d, 4.0001d, -6.999d);

      Rectangle mergedRect = rect1.merge(rect2);

      assertEquals(-1.2d, mergedRect.getLeftValue(), 0.0f);
      assertEquals(5.12d, mergedRect.getRightValue(), 0.0f);
      assertEquals(4.0001d, mergedRect.getTopValue(), 0.0f);
      assertEquals(-7.0d, mergedRect.getBottomValue(), 0.0f);
   }

   @Test
   public void testMerge_UndefinedRectanglePassed_RectangleShouldBeUnchanged() {

      Rectangle rect1 = new Rectangle(1.1d, 5.12d, 4.0d, 7.0d);
      Rectangle rect2 = new Rectangle();

      Rectangle mergedRect = rect1.merge(rect2);

      assertEquals(rect1, mergedRect);
   }

   @Test
   public void testMerge_UndefinedRectangleWithDefinedRectangle_ResultShouldBePassedRectangle() {

      Rectangle rect1 = new Rectangle();
      Rectangle rect2 = new Rectangle(1.1d, 5.12d, 4.0d, 7.0d);

      Rectangle mergedRect = rect1.merge(rect2);

      assertEquals(rect2, mergedRect);
   }

   @Test
   public void testIntersects_IntersectingRectangle() {

      Rectangle rect1 = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      Rectangle rect2 = new Rectangle(5.0d, 15.0d, 15.0d, 5.0d);

      assertTrue(rect1.intersects(rect2));
      assertTrue(rect2.intersects(rect1));
      assertTrue(rect1.intersects(rect1));
   }

   @Test
   public void testIntersects_IntersectingRectangleInside() {

      Rectangle rect1 = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      Rectangle rect2 = new Rectangle(3.0d, 7.0d, 7.0d, 3.0d);

      assertTrue(rect1.intersects(rect2));
      assertTrue(rect2.intersects(rect1));
   }

   @Test
   public void testIntersects_IntersectingRectangleAtCorner() {

      Rectangle rect1 = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      Rectangle rect2 = new Rectangle(10.0d, 20.0d, 0.0d, -10.0d);

      assertTrue(rect1.intersects(rect2));
      assertTrue(rect2.intersects(rect1));
   }

   @Test
   public void testIntersects_IntersectingRectangleOutside() {

      Rectangle rect1 = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      Rectangle rect2 = new Rectangle(10.1d, 20.0d, 7.0d, 3.0d);

      assertFalse(rect1.intersects(rect2));
      assertFalse(rect2.intersects(rect1));
   }

   @Test
   public void testGetBoundingBox_ResultShouldEqualToRectangle() {

      Rectangle rect = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);

      Rectangle bb = rect.getBoundingBox();

      assertEquals(rect, bb);
   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      Rectangle rect = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);
      Object object = null;

      assertFalse(rect.equals(object));

   }

   @Test
   public void testEquals_SameObject_ShouldBeTrue() {
      Rectangle rect = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);

      Object object = rect;

      assertTrue(rect.equals(object));

   }

   @Test
   public void testHashCode_ObjectsWithSameValue_HashCodeShouldBeEqual() {
      Rectangle rect1 = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);
      Rectangle rect2 = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);

      assertEquals(rect1.hashCode(), rect2.hashCode());

   }

   @Test
   public void testHashCode_ObjectsWithDifferentValue_HashCodeShouldBeNotEqual() {
      Rectangle rect1 = new Rectangle(-1.0d, 5.12d, 4.0d, -7.0d);
      Rectangle rect2 = new Rectangle(-1.1d, 5.12d, 4.0d, -7.0d);

      assertNotEquals(rect1.hashCode(), rect2.hashCode());

   }
}
