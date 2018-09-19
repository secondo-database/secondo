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
package mol.operations.predicates;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.Region;
import mol.datatypes.spatial.util.Face;
import mol.interfaces.spatial.PointIF;

/**
 * Tests for 'Inside' class methods
 * 
 * @author Markus Fuessel
 */
public class InsideTest {

   /**
    * Boundary:<br>
    * ( (0, 0), (0, 50), (50, 50), (50, 0) )
    */
   private Region regionWithoutHole;

   /**
    * Boundary:<br>
    * ( (0, 0), (0, 50), (50, 50), (50, 0) )<br>
    * <br>
    * Holes:<br>
    * ( (20, 20), (20, 40), (40, 40), (40, 20) )
    */
   private Region regionWithHole;

   /**
    * Boundary:<br>
    * ( (0, 50), (50, 100), (100, 50), (50, 0) )
    */
   private Region regionWithoutHoleOnCorner;

   @Before
   public void setUp() throws Exception {
      List<PointIF> points = new ArrayList<>();
      points.add(new Point(0, 0));
      points.add(new Point(0, 50));
      points.add(new Point(50, 50));
      points.add(new Point(50, 0));

      regionWithoutHole = new Region(points);

      List<PointIF> holePoints = new ArrayList<>();
      holePoints.add(new Point(20, 20));
      holePoints.add(new Point(20, 40));
      holePoints.add(new Point(40, 40));
      holePoints.add(new Point(40, 20));

      Face face = new Face(points);
      face.add(holePoints);

      regionWithHole = new Region(face);

      List<PointIF> points2 = new ArrayList<>();
      points2.add(new Point(0, 50));
      points2.add(new Point(50, 100));
      points2.add(new Point(100, 50));
      points2.add(new Point(50, 0));

      regionWithoutHoleOnCorner = new Region(points2);
   }

   @Test
   public void testInside_PointInRegion_ShouldBeTrue() {
      Point point = new Point(10, 10);

      assertTrue(Inside.inside(point, regionWithoutHole));
   }

   @Test
   public void testInside_PointOutOfRegion_ShouldBeFalse() {
      Point point = new Point(100, 100);

      assertFalse(Inside.inside(point, regionWithoutHole));
   }

   @Test
   public void testInside_PointInHoleOfRegion_ShouldBeFalse() {
      Point point = new Point(30, 30);

      assertFalse(Inside.inside(point, regionWithHole));
   }

   @Test
   public void testInside_UndefinedPoint_ShouldBeFalse() {
      Point point = new Point();

      assertFalse(Inside.inside(point, regionWithHole));
   }

   @Test
   public void testInside_PointInsideOfRegionUnderCorner_ShouldBeTrue() {
      Point point0 = new Point(50, 1);
      Point point1 = new Point(50, 50);
      Point point2 = new Point(50, 99);

      assertTrue(Inside.inside(point0, regionWithoutHoleOnCorner));
      assertTrue(Inside.inside(point1, regionWithoutHoleOnCorner));
      assertTrue(Inside.inside(point2, regionWithoutHoleOnCorner));
   }

   @Test
   public void testInside_PointInsideOfRegionUnderCorner_ShouldBeFalse() {
      Point point0 = new Point(0, 1);
      Point point1 = new Point(0, 50);
      Point point2 = new Point(0, 99);

      Point point3 = new Point(100, 1);
      Point point4 = new Point(100, 50);
      Point point5 = new Point(100, 99);

      assertFalse(Inside.inside(point0, regionWithoutHoleOnCorner));
      assertFalse(Inside.inside(point1, regionWithoutHoleOnCorner));
      assertFalse(Inside.inside(point2, regionWithoutHoleOnCorner));

      assertFalse(Inside.inside(point3, regionWithoutHoleOnCorner));
      assertFalse(Inside.inside(point4, regionWithoutHoleOnCorner));
      assertFalse(Inside.inside(point5, regionWithoutHoleOnCorner));
   }
}
