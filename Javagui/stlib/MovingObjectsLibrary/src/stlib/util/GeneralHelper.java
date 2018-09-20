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

import java.util.ArrayList;
import java.util.List;

import stlib.interfaces.spatial.PointIF;

/**
 * This class provides some useful methods for calculation and comparison
 * 
 * @author Markus Fuessel
 */
public final class GeneralHelper {

   /**
    * Epsilon value
    * 
    * Two floating point numbers are considered equal if their difference is less
    * than the static variable {@code epsilon}
    */
   private static double epsilon = 0.0000001;

   /**
    * Verifys if the passed values are almost equal. This means that the two values
    * are considered equal if they differ by less than the value epsilon.
    * 
    * @param d0
    *           - first value
    * @param d1
    *           - second value
    * 
    * @return true if both values almost equal, false otherwise
    */
   public static boolean almostEqual(final double d0, final double d1) {
      return Math.abs(d0 - d1) <= epsilon;
   }

   /**
    * Calculate the simple distance between two double values
    * 
    * @param d0
    *           - first double value
    * @param d1
    *           - second double value
    * 
    * @return the absolute distance
    */
   public static double distance(final double d0, final double d1) {
      return Math.abs(d0 - d1);
   }

   /**
    * Calculate the euclidean distance between the passed values which are
    * interpreted as coordinates of two points in the euclidean space
    * 
    * @param x0
    *           - x value of first point
    * @param y0
    *           - y value of first point
    * @param x1
    *           - x value of second point
    * @param y1
    *           - y value of second point
    * 
    * @return the euclidean distance
    */
   public static double distanceEuclidean(final double x0, final double y0, final double x1, final double y1) {
      return Math.sqrt(Math.pow(x1 - x0, 2) + Math.pow(y1 - y0, 2));
   }

   /**
    * Calculates the spherical distance between two geographical points, a
    * orthodrome, specified by passed latitude and longitude values with an
    * accuracy of 50 metres. The orthodrome is the shortest connection of two
    * points on a sphere surface. <br>
    * The values have to be passed in decimal degree e.g. 52.5183252d and
    * 13.4081446d<br>
    * <br>
    * The calculation is based on a WGS84 ellipsoid
    * 
    * @param lat0
    *           - latitude of first point
    * @param lon0
    *           - longitude of first point
    * @param lat1
    *           - latitude of second point
    * @param lon1
    *           - longitude of second point
    * 
    * @return the geographic distance in metres
    */
   public static double distanceOrthodrome(final double lat0, final double lon0, final double lat1, final double lon1) {
      double earthRadius = 6378137.0d; // in metres
      double earthFlattening = 1 / 298.257223563;

      double radF = Math.toRadians((lat0 + lat1) / 2);
      double radG = Math.toRadians((lat0 - lat1) / 2);
      double radl = Math.toRadians((lon0 - lon1) / 2);

      double sqSinG = Math.pow(Math.sin(radG), 2);
      double sqCosl = Math.pow(Math.cos(radl), 2);
      double sqCosF = Math.pow(Math.cos(radF), 2);
      double sqSinl = Math.pow(Math.sin(radl), 2);
      double sqCosG = Math.pow(Math.cos(radG), 2);
      double sqSinF = Math.pow(Math.sin(radF), 2);

      double S = sqSinG * sqCosl + sqCosF * sqSinl;
      double C = sqCosG * sqCosl + sqSinF * sqSinl;

      double w = Math.atan(Math.sqrt(S / C));

      double approxDistance = 2 * w * earthRadius;

      double T = Math.sqrt(S * C) / w;

      double H1 = (3 * T - 1) / (2 * C);

      double H2 = (3 * T + 1) / (2 * S);

      return approxDistance * (1 + earthFlattening * H1 * sqSinF * sqCosG - earthFlattening * H2 * sqCosF * sqSinG);

   }

   /**
    * Check if one moves counterclockwise when following the points p0 -> p1 -> p2
    * 
    * @param p0
    * @param p1
    * @param p2
    * 
    * @return 1 : move counter clockwise <br>
    *         -1 : move clockwise <br>
    *         0 :
    */
   public static int counterClockwisePath(final PointIF p0, final PointIF p1, final PointIF p2) {
      int ccw = 0;

      Vector2D deltaP0P1 = p1.minus(p0);
      Vector2D deltaP0P2 = p2.minus(p0);

      double crossProduct = deltaP0P1.cross(deltaP0P2);

      if (crossProduct > 0) {
         ccw = 1;

      } else if (crossProduct < 0) {
         ccw = -1;

      } else {

         if (deltaP0P1.x * deltaP0P2.x < 0 || deltaP0P1.y * deltaP0P2.y < 0) {
            ccw = -1;

         } else if ((deltaP0P1.product(deltaP0P1)) >= (deltaP0P2.product(deltaP0P2))) {
            ccw = 0;
         } else {
            ccw = 1;
         }
      }

      return ccw;
   }

   /**
    * Determines the position of the first point in relation to the other two.<br>
    * If all three points are on a line, a value between 0 and 1 indicates the
    * ratio between the other two points.
    * 
    * @param point
    *           - the point whose position is to be determined
    * 
    * @param startPoint
    * @param endPoint
    * 
    * @return a value between 0 and 1 indicates the ratio between the other two
    *         points
    * 
    * @see http://twistedoakstudios.com/blog/Post2194_determining-exactly-ifwhenwhere-a-moving-line-intersected-a-moving-point
    */
   public static double determinePositionRatio(PointIF point, PointIF startPoint, PointIF endPoint) {
      Vector2D deltaPointStart = point.minus(startPoint);
      Vector2D deltaEndStart = endPoint.minus(startPoint);

      return (deltaPointStart.product(deltaEndStart)) / (deltaEndStart.product(deltaEndStart));
   }

   /**
    * This method determines the intersections of a moving line with a fixed point
    * in the form of time and positions scalars. Depending on how the line moves,
    * more than one intersection point is possible.
    * 
    * @param fixedPoint
    *           - the fixed point
    * 
    * @param initStartPoint
    *           - initial start point of the moving line
    * 
    * @param finalStartPoint
    *           - - final start point of the moving line
    * 
    * @param initEndPoint
    *           - initial end point of the moving line
    * 
    * @param finalEndPoint
    *           - final end point of the moving line
    * 
    * @return a List of 'CrossPointScalars' data objects with the determined
    *         scalars
    * 
    * @see http://twistedoakstudios.com/blog/Post2194_determining-exactly-ifwhenwhere-a-moving-line-intersected-a-moving-point
    */
   public static List<CrossPointScalars> intersection(PointIF fixedPoint, PointIF initStartPoint,
                                                      PointIF finalStartPoint, PointIF initEndPoint,
                                                      PointIF finalEndPoint) {

      List<CrossPointScalars> timePositions = new ArrayList<>();

      Vector2D startDelta = finalStartPoint.minus(initStartPoint);
      Vector2D endDelta = finalEndPoint.minus(initEndPoint);

      Vector2D a = fixedPoint.minus(initStartPoint);
      Vector2D b = startDelta.neg();
      Vector2D c = initEndPoint.minus(initStartPoint);
      Vector2D d = endDelta.minus(startDelta);

      List<Double> results = quadraticRoots(b.cross(d), a.cross(d) + b.cross(c), a.cross(c));

      for (int i = 0; i < results.size(); i++) {
         double t = results.get(i).doubleValue();

         if (t >= 0 && t <= 1) {

            PointIF startPoint = initStartPoint.plus(startDelta.scale(t));
            PointIF endPoint = initEndPoint.plus(endDelta.scale(t));

            double s = determinePositionRatio(fixedPoint, startPoint, endPoint);

            if (s >= 0 && s <= 1) {
               timePositions.add(new CrossPointScalars(t, s));
            }
         }
      }

      return timePositions;
   }

   /**
    * Enumerates the real solutions to the formula a*x^2 + b*x + c = 0.<br>
    * Handles degenerate cases.<br>
    * If a=b=c=0 then only zero is enumerated, even though technically all real
    * numbers are solutions.
    * 
    * @param a
    *           - coefficient a*x^2
    * @param b
    *           - coefficient b*x
    * @param c
    *           - coefficient c
    * 
    * @return solutions as list of Double values
    * 
    * @see http://twistedoakstudios.com/blog/Post2194_determining-exactly-ifwhenwhere-a-moving-line-intersected-a-moving-point
    */
   public static List<Double> quadraticRoots(double a, double b, double c) {
      List<Double> results = new ArrayList<>();

      // degenerate? (0x^2 + bx + c == 0)
      if (a == 0) {
         // double-degenerate? (0x^2 + 0x + c == 0)
         if (b == 0) {
            // triple-degenerate? (0x^2 + 0x + 0 == 0)
            if (c == 0) {
               // every other real number is also a solution, but hopefully one example will be
               // fine
               results.add(Double.valueOf(0));
            }
            return results;
         }

         results.add(Double.valueOf(-c / b));
         return results;
      }

      // ax^2 + bx + c == 0
      // x = (-b +- sqrt(b^2 - 4ac)) / 2a

      double d = b * b - 4 * a * c;
      if (d < 0) { // no real roots
         return results;
      }

      double s0 = -b / (2 * a);
      double sd = Math.sqrt(d) / (2 * a);

      results.add(Double.valueOf(s0 - sd));

      if (sd == 0) {
         return results; // unique root
      }

      results.add(Double.valueOf(s0 + sd));

      return results;
   }

   /**
    * Get the epsilon value
    * 
    * @return the epsilon
    */
   public static double getEpsilon() {
      return epsilon;
   }

   /**
    * Set the epsilon value
    * 
    * @param epsilon
    *           the epsilon to set
    */
   public static void setEpsilon(double epsilon) {
      GeneralHelper.epsilon = epsilon;
   }

   /**
    * Only helper class, no instances allowed
    */
   private GeneralHelper() {

   }
}
