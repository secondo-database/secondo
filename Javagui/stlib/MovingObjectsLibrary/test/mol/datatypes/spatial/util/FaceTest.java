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

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import mol.datatypes.spatial.Point;

/**
 * Tests for class 'Face'
 * 
 * @author Markus Fuessel
 */
public class FaceTest {

   @Test
   public void testFace_PassDefinedCycle_FaceShouldBeDefined() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(new Cycle(points));

      assertTrue(face.isDefined());
      assertEquals(0, face.getHoles().size());
      assertTrue(face.getBoundary().isDefined());

   }

   @Test
   public void testFace_PassListOfPoints_FaceShouldBeDefined() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      assertTrue(face.isDefined());
      assertEquals(0, face.getHoles().size());
      assertTrue(face.getBoundary().isDefined());
   }

   @Test
   public void testFace_PassEmptyListOfPoints_FaceShouldBeUndefined() {
      List<Point> points = new ArrayList<>();

      Face face = new Face(points);

      assertFalse(face.isDefined());
   }

   @Test
   public void testAdd_DefinedHoleCycle_ShouldBeAdded() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      List<Point> holePoints = new ArrayList<>();

      holePoints.add(new Point(2.0d, 2.0d));
      holePoints.add(new Point(3.0d, 3.0d));
      holePoints.add(new Point(4.0d, 2.0d));

      Cycle holeCycle = new Cycle(holePoints);

      face.add(holeCycle);

      assertEquals(1, face.getHoles().size());
   }

   @Test
   public void testAdd_UndefinedHoleCycle_ShouldNotBeAdded() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      List<Point> holePoints = new ArrayList<>();

      Cycle holeCycle = new Cycle(holePoints);

      boolean holeAddded = face.add(holeCycle);

      assertFalse(holeCycle.isDefined());
      assertFalse(holeAddded);
      assertEquals(0, face.getHoles().size());
   }

   @Test
   public void testGetBoundingBox() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Cycle cycle = new Cycle(points);
      Face face = new Face(cycle);

      Rectangle expectedBB = cycle.getBoundingBox();

      assertEquals(expectedBB, face.getBoundingBox());

   }

}
