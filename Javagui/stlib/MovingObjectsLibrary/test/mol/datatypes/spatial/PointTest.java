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
package mol.datatypes.spatial;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import mol.datatypes.spatial.util.Rectangle;
import mol.util.GeneralHelper;
import mol.util.Vector2D;

/**
 * Tests for class 'Point'
 * 
 * @author Markus Fuessel
 */
public class PointTest {

   @Test
   public void testPointConstructor_UndefinedObject() {
      Point undefinedPoint = new Point();

      assertFalse(undefinedPoint.isDefined());
   }

   @Test
   public void testAlmostEqual_EqualPoints_ShouldBeEqual() {
      Point p1 = new Point(1.0d, 2.0d);
      Point p2 = new Point(1.0d, 2.0d);

      assertTrue(p1.almostEqual(p2));
   }

   @Test
   public void testAlmostEqual_NearlyEqualPoints_ShouldBeEqual() {
      double epsilon = GeneralHelper.getEpsilon() / 10;

      Point p1 = new Point(1.0d, 2.0d);
      Point p2 = new Point(1.0d - epsilon, 2.0d);

      assertFalse(p1.equals(p2));
      assertTrue(p1.almostEqual(p2));

   }

   @Test
   public void testAlmostEqual_NonEqualPoints_ShouldNotBeEqual() {
      double epsilon = GeneralHelper.getEpsilon();

      Point p1 = new Point(1.0d, 2.0d);
      Point p2 = new Point(1.0d + epsilon, 2.0d);

      assertFalse(p1.equals(p2));
      assertFalse(p1.almostEqual(p2));

   }

   @Test
   public void testCompareTo_TwoEqualPoints_ShouldBeZero() {
      Point p1 = new Point(1.0d, 2.0d);
      Point p2 = new Point(1.0d, 2.0d);

      int expected = 0;
      int result = p1.compareTo(p2);

      assertEquals(expected, result);
   }

   @Test
   public void testCompareTo_LowerVsGreaterPoint_ShouldBeMinusOne() {
      Point p1 = new Point(1.0d, 2.0d);
      Point p2 = new Point(1.1d, 2.0d);
      Point p3 = new Point(1.0d, 2.1d);

      int expected = -1;

      assertEquals(expected, p1.compareTo(p2));
      assertEquals(expected, p1.compareTo(p3));
   }

   @Test
   public void testCompareTo_GreaterVsLowerPoint_ShouldBePlusOne() {
      Point p1 = new Point(1.1d, 2.0d);
      Point p2 = new Point(1.0d, 2.0d);
      Point p3 = new Point(1.0d, 2.1d);

      int expected = 1;

      assertEquals(expected, p1.compareTo(p2));
      assertEquals(expected, p3.compareTo(p2));
   }

   @Test
   public void testEquals_EqualPoints_ShouldBeTrue() {

      Point p1 = new Point(1.2d, 2.3d);
      Point p2 = new Point(1.2d, 2.3d);

      assertTrue(p1.equals(p2));

   }

   @Test
   public void testEquals_NonEqualPoints_ShouldBeFalse() {

      Point p1 = new Point(1.2d, 2.3d);
      Point p2 = new Point(1.2d, 2.4d);

      assertFalse(p1.equals(p2));

   }

   @Test
   public void testEquals_NullObject_ShouldBeFalse() {
      Point p = new Point(1.2d, 2.3d);
      Object object = null;

      assertFalse(p.equals(object));

   }

   @Test
   public void testHashCode_EqualPoints_HashCodeShouldBeIndentical() {

      Point p1 = new Point(1.2d, 2.3d);
      Point p2 = new Point(1.2d, 2.3d);

      assertEquals(p1.hashCode(), p2.hashCode());

   }

   @Test
   public void testHashCode_NonEqualPoints_HashCodeShouldBeDifferent() {

      Point p1 = new Point(1.2d, 2.3d);
      Point p2 = new Point(1.2d, 2.4d);

      assertNotEquals(p1.hashCode(), p2.hashCode());

   }

   @Test
   public void testGetBoundingBox() {
      double xValue = 4.0d;
      double yValue = 2.0d;

      Point p1 = new Point(xValue, yValue);
      Rectangle mbb = p1.getBoundingBox();

      assertEquals(xValue, mbb.getLeftValue(), 0.0d);
      assertEquals(xValue, mbb.getRightValue(), 0.0d);
      assertEquals(yValue, mbb.getTopValue(), 0.0d);
      assertEquals(yValue, mbb.getBottomValue(), 0.0d);
   }

   @Test
   public void testDistance_EuclideanDistance() {

      Point p1 = new Point(0.0d, 0.0d);
      Point p2 = new Point(10.0d, 0.0d);

      double expected = 10.0d;

      assertEquals(expected, p1.distance(p2, false), 0.0d);

   }

   @Test
   public void testDistance_UseSphericalGeometry() {

      Point p1 = new Point(51.362328d, 7.463116d); // 51.362328, 7.463116
      Point p2 = new Point(52.523403d, 13.411400d);

      double expected = 428803.559d;

      assertEquals(expected, p1.distance(p2, true), 1.0d);

   }

   @Test
   public void testPlus_AddVectorToPoint() {

      Point initialPoint = new Point(4.0d, 2.0d);
      Vector2D vector = new Vector2D(5.0d, -7.0d);

      Point expectedPoint = new Point(9.0d, -5.0d);

      assertEquals(expectedPoint, initialPoint.plus(vector));
   }

   @Test
   public void testMinus_SubtractPoint_GetVector() {

      Point initialPoint = new Point(7.0d, 6.0d);
      Point finalPoint = new Point(10.0d, 15.0d);
      Vector2D vector = finalPoint.minus(initialPoint);

      assertEquals(3.0d, vector.x, 0.0);
      assertEquals(9.0d, vector.y, 0.0);
   }
}
