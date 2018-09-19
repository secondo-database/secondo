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

package mol.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.List;

import org.junit.Test;

import mol.datatypes.spatial.Point;
import mol.interfaces.spatial.PointIF;

/**
 * Tests for 'GeneralHelper' class methods
 * 
 * @author Markus Fuessel
 */
public class GeneralHelperTest {

   @Test
   public void testAlmostEqual_EqualValues_ShouldBeEqual() {
      double d0 = 1.23d;
      double d1 = 1.23d;

      assertTrue(GeneralHelper.almostEqual(d0, d1));
   }

   @Test
   public void testAlmostEqual_NearlyEqualValues_ShouldBeEqual() {
      double smallerThanEpsilon = GeneralHelper.getEpsilon() / 10;

      double d0 = 1.23d;
      double d1 = 1.23d - smallerThanEpsilon;

      assertFalse(d0 == d1);
      assertTrue(GeneralHelper.almostEqual(d0, d1));

   }

   @Test
   public void testAlmostEqual_NonEqualValues_ShouldNotBeEqual() {
      double epsilon = GeneralHelper.getEpsilon();

      double d0 = 1.23d;
      double d1 = 1.23d - epsilon - (epsilon / 10);

      assertFalse(d0 == d1);
      assertFalse(GeneralHelper.almostEqual(d0, d1));

   }

   @Test
   public void testDistance() {
      double d0 = 1.23d;
      double d1 = 10.0d;
      double expected = d1 - d0;

      assertEquals(expected, GeneralHelper.distance(d0, d1), 0.0d);
   }

   @Test
   public void testDistanceEuclidean() {
      double x0 = 0.0d;
      double y0 = 0.0d;
      double x1 = 10.0d;
      double y1 = 0.0d;

      double expectedDistance = 10.0d;

      assertEquals(expectedDistance, GeneralHelper.distanceEuclidean(x0, y0, x1, y1), 0.0d);
   }

   @Test
   public void testDistanceOrthodrome() {

      double lat0 = 53.117500d;
      double lon0 = 13.504492d;
      double lat1 = 52.523403d;
      double lon1 = 13.411400d;

      double expectedDistance = 66410.013d;

      assertEquals(expectedDistance, GeneralHelper.distanceOrthodrome(lat0, lon0, lat1, lon1), 1.0d);
   }

   @Test
   public void testCounterClockwisePath_ClockwiseMovement_ShouldBeMinusOne() {
      Point p0 = new Point(0.0d, 0.0d);
      PointIF p1 = new Point(10.0d, 0.0d);
      PointIF p2 = new Point(0.0d, -10.0d);

      assertEquals(-1, GeneralHelper.counterClockwisePath(p0, p1, p2));

   }

   @Test
   public void testCounterClockwisePath_CounterClockwiseMovement_ShouldBePlusOne() {
      Point p0 = new Point(0.0d, 0.0d);
      PointIF p1 = new Point(10.0d, 0.0d);
      PointIF p2 = new Point(10.0d, 10.0d);

      assertEquals(1, GeneralHelper.counterClockwisePath(p0, p1, p2));

   }

   @Test
   public void testCounterClockwisePath_AllOnLineP2BetweenP0AndP1_ShouldBeZero() {
      Point p0 = new Point(0.0d, 0.0d);
      PointIF p1 = new Point(10.0d, 0.0d);
      PointIF p2 = new Point(5.0d, 0.0d);

      assertEquals(0, GeneralHelper.counterClockwisePath(p0, p1, p2));

   }

   @Test
   public void testCounterClockwisePath_AllOnLineP2BeforeP0_ShouldBeMinusOne() {
      Point p0 = new Point(0.0d, 0.0d);
      PointIF p1 = new Point(10.0d, 0.0d);
      PointIF p2 = new Point(-5.0d, 0.0d);

      assertEquals(-1, GeneralHelper.counterClockwisePath(p0, p1, p2));

   }

   @Test
   public void testCounterClockwisePath_AllOnLineP2AfterP1_ShouldBePlusOne() {
      Point p0 = new Point(0.0d, 0.0d);
      PointIF p1 = new Point(10.0d, 0.0d);
      PointIF p2 = new Point(15.0d, 0.0d);

      assertEquals(1, GeneralHelper.counterClockwisePath(p0, p1, p2));

   }

   @Test
   public void testDeterminePositionRatio_BetweenPoints() {

      Point startPoint = new Point(5.0d, 2.0d);
      PointIF endPoint = new Point(15.0d, 2.0d);

      PointIF p0 = new Point(5.0d, 2.0d);
      PointIF p1 = new Point(10.0d, 2.0d);
      PointIF p2 = new Point(15.0d, 2.0d);

      assertEquals(0.0d, GeneralHelper.determinePositionRatio(p0, startPoint, endPoint), 0.0);
      assertEquals(0.5d, GeneralHelper.determinePositionRatio(p1, startPoint, endPoint), 0.0);
      assertEquals(1.0d, GeneralHelper.determinePositionRatio(p2, startPoint, endPoint), 0.0);

   }

   @Test
   public void testDeterminePositionRatio_OutsidePoints() {

      Point startPoint = new Point(5.0d, 2.0d);
      PointIF endPoint = new Point(15.0d, 2.0d);

      PointIF p0 = new Point(4.99999999d, 2.0d);
      PointIF p1 = new Point(15.00000001d, 2.0d);

      assertTrue(GeneralHelper.determinePositionRatio(p0, startPoint, endPoint) < 0.0);
      assertTrue(GeneralHelper.determinePositionRatio(p1, startPoint, endPoint) > 1.0);

   }

   @Test
   public void testQuadraticRoots_NoSolution() {

      List<Double> results1 = GeneralHelper.quadraticRoots(0, 0, 1);
      List<Double> results2 = GeneralHelper.quadraticRoots(1, 0, 1);

      assertEquals(0, results1.size());
      assertEquals(0, results2.size());
   }

   @Test
   public void testQuadraticRoots_OneSolution() {

      List<Double> results1 = GeneralHelper.quadraticRoots(1, 0, 0);
      List<Double> results2 = GeneralHelper.quadraticRoots(0, 1, 0);
      List<Double> results3 = GeneralHelper.quadraticRoots(0, 1, 1);
      List<Double> results4 = GeneralHelper.quadraticRoots(1, 2, 1);

      assertEquals(1, results1.size());
      assertEquals(0, results1.get(0).doubleValue(), 0.0d);

      assertEquals(1, results2.size());
      assertEquals(0, results2.get(0).doubleValue(), 0.0d);

      assertEquals(1, results3.size());
      assertEquals(-1, results3.get(0).doubleValue(), 0.0d);

      assertEquals(1, results4.size());
      assertEquals(-1, results4.get(0).doubleValue(), 0.0d);

   }

   @Test
   public void testQuadraticRoots_TwoSolution() {

      List<Double> results1 = GeneralHelper.quadraticRoots(1, 1, 0);
      List<Double> results2 = GeneralHelper.quadraticRoots(1, 0, -1);

      assertEquals(2, results1.size());
      assertEquals(-1, results1.get(0).doubleValue(), 0.0d);
      assertEquals(0, results1.get(1).doubleValue(), 0.0d);

      assertEquals(2, results2.size());
      assertEquals(-1, results2.get(0).doubleValue(), 0.0d);
      assertEquals(1, results2.get(1).doubleValue(), 0.0d);

   }

   @Test
   public void testIntersection_OneIntersection() {

      PointIF fixedPoint = new Point(1.0, 1.0);
      Point initStartPoint = new Point(0.0, 0.0);
      PointIF finalStartPoint = new Point(2.0, 0.0);
      Point initEndPoint = new Point(0.0, 2.0);
      PointIF finalEndPoint = new Point(2.0, 2.0);

      List<CrossPointScalars> timePositions = GeneralHelper.intersection(fixedPoint, initStartPoint, finalStartPoint,
            initEndPoint, finalEndPoint);

      assertEquals(1, timePositions.size());
      assertEquals(0.5, timePositions.get(0).positionScalar, 0.0);
      assertEquals(0.5, timePositions.get(0).timeScalar, 0.0);

   }

   @Test
   public void testIntersection_NoIntersection() {

      PointIF fixedPoint = new Point(5.0, 5.0);
      Point initStartPoint = new Point(0.0, 0.0);
      PointIF finalStartPoint = new Point(2.0, 0.0);
      Point initEndPoint = new Point(0.0, 2.0);
      PointIF finalEndPoint = new Point(2.0, 2.0);

      List<CrossPointScalars> timePositions = GeneralHelper.intersection(fixedPoint, initStartPoint, finalStartPoint,
            initEndPoint, finalEndPoint);

      assertEquals(0, timePositions.size());

   }
}
