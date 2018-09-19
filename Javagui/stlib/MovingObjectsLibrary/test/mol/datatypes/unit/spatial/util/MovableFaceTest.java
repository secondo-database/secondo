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
package mol.datatypes.unit.spatial.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import mol.TestUtil.TestUtilData;
import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.time.TimeInstant;
import mol.interfaces.spatial.util.FaceIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * Tests for the 'MovableFace' class
 * 
 * @author Markus Fuessel
 */
public class MovableFaceTest {

   MovableFaceIF mFaceWithoutHoles;
   MovableFaceIF mFaceWithHoles;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      DateTimeFormatter format = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss:SSS");

      TimeInstant.setDefaultDateTimeFormat(format);
   }

   @Before
   public void setUp() throws Exception {

      mFaceWithoutHoles = TestUtilData.getMovableFace(0, 0);
      mFaceWithHoles = TestUtilData.getMovableFaceWithHoles(0, 0);
   }

   @Test
   public void testMovableFace_MovableCycleBoundary_ListMovableCycleHoles() {
      // a square
      Point iP0 = new Point(0.0d, 0.0d);
      Point iP1 = new Point(0.0d, 10.0d);
      Point iP2 = new Point(10.0d, 10.0d);
      Point iP3 = new Point(10.0d, 0.0d);

      // a triangle
      Point fP0 = new Point(0.0d, 0.0d);
      Point fP1 = new Point(10.0d, 10.0d);
      Point fP2 = new Point(10.0d, 0.0d);

      List<MovableSegmentIF> msegments = new ArrayList<>();

      msegments.add(new MovableSegment(iP0, iP1, fP0, fP0));
      msegments.add(new MovableSegment(iP1, iP1, fP0, fP1));
      msegments.add(new MovableSegment(iP1, iP2, fP1, fP1));
      msegments.add(new MovableSegment(iP2, iP3, fP1, fP2));
      msegments.add(new MovableSegment(iP3, iP0, fP2, fP0));

      MovableCycleIF mcycle = new MovableCycle(msegments);

      Point iH0P0 = new Point(2.0d, 1.0d);
      Point iH0P1 = new Point(4.0d, 4.0d);
      Point iH0P2 = new Point(4.0d, 1.0d);

      Point fH0P0 = new Point(2.0d, 1.0d);
      Point fH0P1 = new Point(4.0d, 4.0d);
      Point fH0P2 = new Point(4.0d, 1.0d);

      // hole1, a square, degenerate at the end
      Point iH1P0 = new Point(6.0d, 4.0d);
      Point iH1P1 = new Point(6.0d, 6.0d);
      Point iH1P2 = new Point(8.0d, 6.0d);
      Point iH1P3 = new Point(8.0d, 4.0d);

      Point fH1P0 = new Point(7.0d, 5.0d);

      List<MovableSegmentIF> mHoleSegments0 = new ArrayList<>();
      List<MovableSegmentIF> mHoleSegments1 = new ArrayList<>();

      mHoleSegments0.add(new MovableSegment(iH0P0, iH0P1, fH0P0, fH0P1));
      mHoleSegments0.add(new MovableSegment(iH0P1, iH0P2, fH0P1, fH0P2));
      mHoleSegments0.add(new MovableSegment(iH0P2, iH0P0, fH0P2, fH0P0));

      mHoleSegments1.add(new MovableSegment(iH1P0, iH1P1, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P1, iH1P2, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P2, iH1P3, fH1P0, fH1P0));
      mHoleSegments1.add(new MovableSegment(iH1P3, iH1P0, fH1P0, fH1P0));

      List<MovableCycleIF> mHoles = new ArrayList<>();

      mHoles.add(new MovableCycle(mHoleSegments0));
      mHoles.add(new MovableCycle(mHoleSegments1));

      MovableFaceIF mface = new MovableFace(mcycle, mHoles);

      int expectedMSegments = msegments.size() + mHoleSegments0.size() + mHoleSegments1.size();

      assertEquals(expectedMSegments, mface.getMovingSegments().size());

   }

   @Test
   public void testAdd_NumberOfMovingHolesIncrease() {
      int noMovingHolesBeforeAdd = mFaceWithoutHoles.getNoMovingHoles();

      // constant hole0, a triangle
      Point iH0P0 = new Point(2.0d, 1.0d);
      Point iH0P1 = new Point(4.0d, 4.0d);
      Point iH0P2 = new Point(4.0d, 1.0d);

      Point fH0P0 = new Point(2.0d, 1.0d);
      Point fH0P1 = new Point(4.0d, 4.0d);
      Point fH0P2 = new Point(4.0d, 1.0d);

      List<MovableSegmentIF> mHoleSegments = new ArrayList<>();

      mHoleSegments.add(new MovableSegment(iH0P0, iH0P1, fH0P0, fH0P1));
      mHoleSegments.add(new MovableSegment(iH0P1, iH0P2, fH0P1, fH0P2));
      mHoleSegments.add(new MovableSegment(iH0P2, iH0P0, fH0P2, fH0P0));

      MovableCycle movingHole = new MovableCycle(mHoleSegments);

      mFaceWithoutHoles.add(movingHole);

      int noMovingHolesAfterAdd = mFaceWithoutHoles.getNoMovingHoles();

      assertEquals(noMovingHolesBeforeAdd + 1, noMovingHolesAfterAdd);
   }

   @Test
   public void testGetValue_MovableFaceChangedPositionAndSize_ValidInstant() {

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = new TimeInstant("2018-01-05 12:00:00:000");

      FaceIF faceWithoutHoles = mFaceWithoutHoles.getValue(movementPeriod, instant);
      FaceIF faceWithHoles = mFaceWithHoles.getValue(movementPeriod, instant);

      assertTrue(faceWithoutHoles.isDefined());
      assertEquals(10, faceWithoutHoles.getBoundary().getHalfsegments().size());
      assertEquals(0, faceWithoutHoles.getNoHoles());

      assertTrue(faceWithHoles.isDefined());
      assertEquals(10, faceWithHoles.getBoundary().getHalfsegments().size());
      assertEquals(2, faceWithHoles.getNoHoles());
   }

   @Test
   public void testGetValue_MovableFaceChangedPositionAndSize_InvalidInstant() {

      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = new TimeInstant("2018-01-10 00:00:00:001");

      FaceIF faceWithoutHoles = mFaceWithoutHoles.getValue(movementPeriod, instant);
      FaceIF faceWithHoles = mFaceWithHoles.getValue(movementPeriod, instant);

      assertFalse(faceWithoutHoles.isDefined());

      assertFalse(faceWithHoles.isDefined());
   }

   @Test
   public void testGetValue_MovableFaceChangedPositionAndSize_InstantAtLowerEnd() {
      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = movementPeriod.getLowerBound();

      FaceIF faceWithoutHoles = mFaceWithoutHoles.getValue(movementPeriod, instant);
      FaceIF faceWithHoles = mFaceWithHoles.getValue(movementPeriod, instant);

      assertTrue(faceWithoutHoles.isDefined());
      assertEquals(8, faceWithoutHoles.getBoundary().getHalfsegments().size());
      assertEquals(0, faceWithoutHoles.getNoHoles());

      assertTrue(faceWithHoles.isDefined());
      assertEquals(8, faceWithHoles.getBoundary().getHalfsegments().size());
      assertEquals(2, faceWithHoles.getNoHoles());

   }

   @Test
   public void testGetValue_MovableCycleChangedPositionAndSize_InstantAtUpperEnd() {
      Period movementPeriod = new Period("2018-01-01 00:00:00:000", "2018-01-10 00:00:00:000", true, false);
      TimeInstantIF instant = movementPeriod.getUpperBound();

      FaceIF faceWithoutHoles = mFaceWithoutHoles.getValue(movementPeriod, instant);
      FaceIF faceWithHoles = mFaceWithHoles.getValue(movementPeriod, instant);

      assertTrue(faceWithoutHoles.isDefined());
      assertEquals(6, faceWithoutHoles.getBoundary().getHalfsegments().size());
      assertEquals(0, faceWithoutHoles.getNoHoles());

      assertTrue(faceWithHoles.isDefined());
      assertEquals(6, faceWithHoles.getBoundary().getHalfsegments().size());
      assertEquals(1, faceWithHoles.getNoHoles());

   }

   @Test
   public void testGetInitial() {

      FaceIF faceWithoutHoles = mFaceWithoutHoles.getInitial();
      FaceIF faceWithHoles = mFaceWithHoles.getInitial();

      assertTrue(faceWithoutHoles.isDefined());
      assertEquals(8, faceWithoutHoles.getBoundary().getHalfsegments().size());
      assertEquals(0, faceWithoutHoles.getHoles().size());

      assertTrue(faceWithHoles.isDefined());
      assertEquals(8, faceWithHoles.getBoundary().getHalfsegments().size());
      assertEquals(2, faceWithHoles.getHoles().size());

   }

   @Test
   public void testGetFinal() {
      FaceIF faceWithoutHoles = mFaceWithoutHoles.getFinal();
      FaceIF faceWithHoles = mFaceWithHoles.getFinal();

      assertTrue(faceWithoutHoles.isDefined());
      assertEquals(6, faceWithoutHoles.getBoundary().getHalfsegments().size());
      assertEquals(0, faceWithoutHoles.getHoles().size());

      assertTrue(faceWithHoles.isDefined());
      assertEquals(6, faceWithHoles.getBoundary().getHalfsegments().size());
      assertEquals(1, faceWithHoles.getHoles().size());
   }

   @Test
   public void testGetProjectionBoundingBox() {

      RectangleIF expectedPBB = new Rectangle(0.0d, 10.0d, 10.0d, 0.0d);
      RectangleIF currentPBB = mFaceWithoutHoles.getProjectionBoundingBox();

      assertTrue(currentPBB.isDefined());
      assertEquals(expectedPBB, currentPBB);
   }

   @Test
   public void testGetMovingSegments() {

      assertEquals(5, mFaceWithoutHoles.getMovingSegments().size());

      assertEquals(12, mFaceWithHoles.getMovingSegments().size());
   }

}
