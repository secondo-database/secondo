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

package stlib.util;

import static org.junit.Assert.assertEquals;

import org.junit.Before;
import org.junit.Test;

import stlib.datatypes.spatial.Point;
import stlib.interfaces.spatial.PointIF;
import stlib.util.Vector2D;

/**
 * Tests for the 'UnitRegionConstTest' class
 * 
 * @author Markus Fuessel
 */
public class Vector2DTest {

   Vector2D vectorX4Y2;

   @Before
   public void setUp() throws Exception {
      vectorX4Y2 = new Vector2D(4, 2);
   }

   @Test
   public void testNeg() {
      Vector2D expectedVector = new Vector2D(-4, -2);

      assertEquals(expectedVector, vectorX4Y2.neg());
   }

   @Test
   public void testMinus() {
      Vector2D vector = new Vector2D(3, 5);

      Vector2D expectedVector = new Vector2D(1, -3);

      assertEquals(expectedVector, vectorX4Y2.minus(vector));
   }

   @Test
   public void testScale() {

      assertEquals(vectorX4Y2, vectorX4Y2.scale(1));
      assertEquals(new Vector2D(8, 4), vectorX4Y2.scale(2));
      assertEquals(new Vector2D(-8, -4), vectorX4Y2.scale(-2));
   }

   @Test
   public void testCross() {

      double x0 = 3;
      double y0 = 7;

      double x1 = 10;
      double y1 = -2;

      Vector2D vec0 = new Vector2D(x0, y0);
      Vector2D vec1 = new Vector2D(x1, y1);

      double expectedValue = x0 * y1 - x1 * y0;

      assertEquals(expectedValue, vec0.cross(vec1), 0.0d);

   }

   @Test
   public void testProduct() {
      double x0 = 3;
      double y0 = 7;

      double x1 = 10;
      double y1 = -2;

      Vector2D vec0 = new Vector2D(x0, y0);
      Vector2D vec1 = new Vector2D(x1, y1);

      double expectedValue = x0 * x1 + y0 * y1;

      assertEquals(expectedValue, vec0.product(vec1), 0.0d);
   }

   @Test
   public void testToPoint() {
      PointIF expectedPoint = new Point(4, 2);

      assertEquals(expectedPoint, vectorX4Y2.toPoint());
   }

   @Test
   public void testHashCode() {

      assertEquals(32506817, vectorX4Y2.hashCode());
   }

}
