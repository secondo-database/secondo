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

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import mol.datatypes.spatial.util.Face;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.spatial.util.RectangleIF;

/**
 * Tests for class 'Region'
 * 
 * @author Markus Fuessel
 */
public class RegionTest {

   @Test
   public void testRegion_CreateEmptyDefinedRegion() {
      Region region = new Region(true);

      assertTrue(region.isEmpty());
      assertTrue(region.isDefined());
   }

   @Test
   public void testRegion_CreateEmptyUndefinedRegion() {
      Region region = new Region(false);

      assertTrue(region.isEmpty());
      assertFalse(region.isDefined());
   }

   @Test
   public void testRegion_PassDefinedFace_ShouldCreateNonEmptyDefinedRegion() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      Region region = new Region(face);

      assertFalse(region.isEmpty());
      assertTrue(region.isDefined());
   }

   @Test
   public void testRegion_PassUndefinedFace_ShouldCreateEmptyUndefinedRegion() {
      List<PointIF> points = new ArrayList<>();

      Face face = new Face(points);

      Region region = new Region(face);

      assertTrue(region.isEmpty());
      assertFalse(region.isDefined());
   }

   @Test
   public void testRegion_PassListOfPoints_ShouldCreateNonEmptyDefinedRegion() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Region region = new Region(points);

      assertFalse(region.isEmpty());
      assertTrue(region.isDefined());
   }

   @Test
   public void testRegion_PassEmptyListOfPoints_ShouldCreateEmptyUndefinedRegion() {
      List<PointIF> points = new ArrayList<>();

      Region region = new Region(points);

      assertTrue(region.isEmpty());
      assertFalse(region.isDefined());
   }

   @Test
   public void testAdd_DefinedFace_ShouldBeAdded() {
      List<PointIF> pointsFirstFace = new ArrayList<>();

      pointsFirstFace.add(new Point(0.0d, 0.0d));
      pointsFirstFace.add(new Point(5.0d, 5.0d));
      pointsFirstFace.add(new Point(10.0d, 0.0d));

      Face face = new Face(pointsFirstFace);

      RegionIF region = new Region(face);

      int noComponentsBeforeAdd = region.getNoComponents();

      List<PointIF> pointsSecFace = new ArrayList<>();

      pointsSecFace.add(new Point(20.0d, 20.0d));
      pointsSecFace.add(new Point(25.0d, 25.0d));
      pointsSecFace.add(new Point(30.0d, 20.0d));

      face = new Face(pointsSecFace);

      region.add(face);

      int noComponentsAfterAdd = region.getNoComponents();

      assertEquals(1, noComponentsBeforeAdd);
      assertEquals(2, noComponentsAfterAdd);

   }

   @Test
   public void testAdd_UndefinedFace_ShouldNotBeAdded() {
      List<PointIF> pointsFirstFace = new ArrayList<>();

      pointsFirstFace.add(new Point(0.0d, 0.0d));
      pointsFirstFace.add(new Point(5.0d, 5.0d));
      pointsFirstFace.add(new Point(10.0d, 0.0d));

      Face face = new Face(pointsFirstFace);

      RegionIF region = new Region(face);

      int noComponentsBeforeAdd = region.getNoComponents();

      List<PointIF> pointsSecFace = new ArrayList<>();

      face = new Face(pointsSecFace);

      region.add(face);

      int noComponentsAfterAdd = region.getNoComponents();

      assertEquals(1, noComponentsBeforeAdd);
      assertEquals(1, noComponentsAfterAdd);
   }

   @Test
   public void testGetBoundingBox() {
      List<PointIF> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Face face = new Face(points);

      RectangleIF expectedBB = face.getBoundingBox();

      Region region = new Region(face);

      assertEquals(expectedBB, region.getBoundingBox());
   }

   @Test
   public void testGetBoundingBox_AddingDefinedFace_BBoxShouldChange() {
      List<PointIF> pointsFirstFace = new ArrayList<>();

      pointsFirstFace.add(new Point(0.0d, 0.0d));
      pointsFirstFace.add(new Point(5.0d, 5.0d));
      pointsFirstFace.add(new Point(10.0d, 0.0d));

      Face face = new Face(pointsFirstFace);

      Region region = new Region(face);

      RectangleIF bBoxBeforeAdd = region.getBoundingBox();

      List<PointIF> pointsSecFace = new ArrayList<>();

      pointsSecFace.add(new Point(20.0d, 20.0d));
      pointsSecFace.add(new Point(25.0d, 25.0d));
      pointsSecFace.add(new Point(30.0d, 20.0d));

      face = new Face(pointsSecFace);

      region.add(face);

      RectangleIF expectedBB = bBoxBeforeAdd.merge(face.getBoundingBox());
      RectangleIF bBoxAfterAdd = region.getBoundingBox();

      assertNotEquals(bBoxBeforeAdd, bBoxAfterAdd);
      assertEquals(expectedBB, bBoxAfterAdd);

   }

}
