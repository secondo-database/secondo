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
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

import mol.datatypes.spatial.util.Halfsegment;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.spatial.util.Segment;

/**
 * Tests for class 'Line'
 * 
 * @author Markus Fuessel
 */
public class LineTest {

   private Line emptyDefinedLine;

   @Before
   public void setUp() throws Exception {
      emptyDefinedLine = new Line(true);
   }

   @Test
   public void testLine_ShouldBeEmptyAndDefined() {
      Line line = new Line(true);

      assertTrue(line.isEmpty());
      assertTrue(line.isDefined());
      assertFalse(line.getBoundingBox().isDefined());
   }

   @Test
   public void testLine_PassListOfPoints_LineShouldBeDefined() {
      List<Point> points = new ArrayList<>();

      points.add(new Point(0.0d, 0.0d));
      points.add(new Point(5.0d, 5.0d));
      points.add(new Point(10.0d, 0.0d));

      Line line = new Line(points);

      assertTrue(line.isDefined());
      assertTrue(line.getHalfsegments().size() > 0);
   }

   @Test
   public void testLine_PassEmptyListOfPoints_FaceShouldBeUndefined() {
      List<Point> points = new ArrayList<>();

      Line line = new Line(points);

      assertFalse(line.isDefined());
      assertTrue(line.getHalfsegments().size() == 0);
   }

   @Test
   public void testAdd_Halfsegment() {
      Halfsegment halfsegment = new Halfsegment(1.0d, 1.0d, 5.0d, 5.0d, true);

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(halfsegment);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertFalse(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_UndefinedHalfsegment_ShouldFail() {
      Halfsegment halfsegment = new Halfsegment(new Point(1.0d, 1.0d), new Point(), true);

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(halfsegment);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertTrue(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_Segment() {
      Segment segment = new Segment(1.0d, 1.0d, 5.0d, 5.0d);

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(segment);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertFalse(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_UndefinedSegment_ShouldFail() {
      Segment segment = new Segment(new Point(1.0d, 1.0d), new Point());

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(segment);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertTrue(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_Points() {
      Point p0 = new Point(1.0d, 1.0d);
      Point p1 = new Point(5.0d, 5.0d);

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(p0, p1);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertFalse(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_UndefinedPoint_ShouldFail() {
      Point p0 = new Point(1.0d, 1.0d);
      Point p1 = new Point();

      boolean isEmptyBeforeAdd = emptyDefinedLine.isEmpty();

      emptyDefinedLine.add(p0, p1);

      boolean isEmptyAfterAdd = emptyDefinedLine.isEmpty();

      assertTrue(isEmptyBeforeAdd);
      assertTrue(isEmptyAfterAdd);
   }

   @Test
   public void testAdd_ShouldUpdateBoundingBox() {
      Halfsegment halfsegment1 = new Halfsegment(1.0d, 1.0d, 5.0d, 5.0d, true);
      Halfsegment halfsegment2 = new Halfsegment(5.0d, 5.0d, -5.0d, 5.0d, true);

      Rectangle expectedBBox1 = new Rectangle();
      Rectangle expectedBBox2 = new Rectangle(1.0d, 5.0d, 5.0d, 1.0d);
      Rectangle expectedBBox3 = new Rectangle(-5.0d, 5.0d, 5.0d, 1.0d);

      Rectangle bbox1 = emptyDefinedLine.getBoundingBox();

      emptyDefinedLine.add(halfsegment1);

      Rectangle bbox2 = emptyDefinedLine.getBoundingBox();

      emptyDefinedLine.add(halfsegment2);

      Rectangle bbox3 = emptyDefinedLine.getBoundingBox();

      assertEquals(expectedBBox1, bbox1);
      assertEquals(expectedBBox2, bbox2);
      assertEquals(expectedBBox3, bbox3);

   }

   @Test
   public void testClear_LineShouldBeEmptyAfterCall() {
      Segment segment = new Segment(1.0d, 1.0d, 5.0d, 5.0d);

      emptyDefinedLine.add(segment);

      boolean isEmptyBeforeClear = emptyDefinedLine.isEmpty();

      emptyDefinedLine.clear();

      boolean isEmptyAfterClear = emptyDefinedLine.isEmpty();

      assertFalse(isEmptyBeforeClear);
      assertTrue(isEmptyAfterClear);
      assertFalse(emptyDefinedLine.getBoundingBox().isDefined());
   }

   @Test
   public void testIsEmpty_NewLine_ShouldBeEmpty() {
      assertTrue(emptyDefinedLine.isEmpty());
   }

   @Test
   public void testIsEmpty_LineWithAddedSegments_ShouldNotBeEmpty() {
      Halfsegment halfsegment = new Halfsegment(1.0d, 1.0d, 5.0d, 5.0d, true);

      emptyDefinedLine.add(halfsegment);

      assertFalse(emptyDefinedLine.isEmpty());
   }

   @Test
   public void testGetBoundingBox_EmptyLineObject_BoundingBoxShouldBeUndefined() {
      Rectangle bbox = emptyDefinedLine.getBoundingBox();

      assertTrue(emptyDefinedLine.isEmpty());
      assertFalse(bbox.isDefined());
   }

   @Test
   public void testLength_EmptyLine_ShouldBeZero() {

      assertEquals(0.0d, emptyDefinedLine.length(false), 0.0d);
   }

   @Test
   public void testLength_UndefinedLine_ShouldBeZero() {

      Line line = new Line(false);

      assertFalse(line.isDefined());
      assertEquals(0.0d, line.length(false), 0.0d);
   }

   @Test
   public void testLength_EuclideanGeometry() {

      Point p0 = new Point(0.0d, 0.0d);
      Point p1 = new Point(10.0d, 0.0d);
      Point p2 = new Point(10.0d, 10.0d);
      Point p3 = new Point(0.0d, 10.0d);

      double expected = 40.0d;

      emptyDefinedLine.add(p0, p1);
      emptyDefinedLine.add(p1, p2);
      emptyDefinedLine.add(p2, p3);
      emptyDefinedLine.add(p3, p0);

      assertEquals(expected, emptyDefinedLine.length(false), 0.0d);
   }

   @Test
   public void testLength_SphericalGeometry() {

      Point p0 = new Point(52.527115d, 13.415982d);
      Point p1 = new Point(52.550944d, 13.430201d);
      Point p2 = new Point(52.553546d, 13.414743d);
      Point p3 = new Point(52.541120d, 13.412416d);

      double expected = 6878.31816d;

      emptyDefinedLine.add(p0, p1);
      emptyDefinedLine.add(p1, p2);
      emptyDefinedLine.add(p2, p3);
      emptyDefinedLine.add(p3, p0);

      assertEquals(expected, emptyDefinedLine.length(true), 1.0d);
   }
}
