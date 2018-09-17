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

import java.util.Objects;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Spatial;
import mol.datatypes.spatial.util.Rectangle;
import mol.util.GeneralHelper;
import mol.util.Vector2D;

/**
 * This class represents spatial objects of type 'Point'
 * 
 * @author Markus Fuessel
 */
public class Point extends GeneralType implements Spatial, Comparable<Point> {

   /**
    * Get the delta between start and end point on the x axis
    * 
    * @return the x axis delta
    */
   public static double getDeltaX(final Point startPoint, final Point endPoint) {
      return endPoint.getXValue() - startPoint.getXValue();
   }

   /**
    * Get the delta between start and end point on the y axis
    * 
    * @return the y axis delta
    */
   public static double getDeltaY(final Point startPoint, final Point endPoint) {
      return endPoint.getYValue() - startPoint.getYValue();
   }

   /**
    * x coordinate of the point
    */
   private final double xValue;

   /**
    * y coordinate of the point
    */
   private final double yValue;

   /**
    * Creates an undefined point object
    * 
    * @param xValue
    *           - value on the x axis
    * @param yValue
    *           - value on the y axis
    */
   public Point() {
      this.xValue = 0.0d;
      this.yValue = 0.0d;

   }

   /**
    * Creates an defined point object
    * 
    * @param xValue
    *           - value on the x axis
    * @param yValue
    *           - value on the y axis
    */
   public Point(final double xValue, final double yValue) {
      this.xValue = xValue;
      this.yValue = yValue;

      setDefined(true);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Spatial#isEmpty()
    */
   @Override
   public boolean isEmpty() {
      return !isDefined();
   }

   /**
    * Getter for the boundingBox. Trivial for a point
    * 
    * @return the boundingBox
    */
   @Override
   public Rectangle getBoundingBox() {
      return new Rectangle(xValue, xValue, yValue, yValue);
   }

   /**
    * Get the distance between this and the passed point.<br>
    * If this and the passed point are geographical points, then
    * useSphericalGeometry should set to true to get the geographical distance.
    * Otherwise euclidean distance is calculated and returned.
    * 
    * @param otherPoint
    *           - the other point to which the distance is to be calculated
    * 
    * @param useSphericalGeometry
    *           - if true spherical geometry is used to calculate the distance in
    *           metres, otherwise euclidean distance is used
    * 
    * @return euclidean or geographical distance, depends on parameter
    *         useSphericalGeometry
    */
   public double distance(final Point otherPoint, final boolean useSphericalGeometry) {

      if (useSphericalGeometry) {
         return GeneralHelper.distanceOrthodrome(xValue, yValue, otherPoint.xValue, otherPoint.yValue);
      } else {
         return GeneralHelper.distanceEuclidean(xValue, yValue, otherPoint.xValue, otherPoint.yValue);
      }

   }

   /**
    * Verify if this point and the passed one are almost equal. This means that the
    * two points are considered the same if they differ only slightly.
    * 
    * @param otherPoint
    *           - the reference point with which to compare.
    * 
    * @return true, the two points are equal, false otherwise
    * 
    * @see mol.util.GeneralHelper#almostEqual(double d1, double d2)
    */
   public boolean almostEqual(final Point otherPoint) {
      return GeneralHelper.almostEqual(this.xValue, otherPoint.xValue)
            && GeneralHelper.almostEqual(this.yValue, otherPoint.yValue);
   }

   /**
    * Compare this point with the passed one. Lexicographical order is used to
    * compare two points. First the x-coordinates of both points are compared. If
    * these are equal, the y-coordinates of both points are compared.
    * 
    * @param otherPoint
    *           - the other point to compare
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final Point otherPoint) {

      int doubleCompare;

      doubleCompare = Double.compare(xValue, otherPoint.getXValue());

      if (doubleCompare != 0) {
         return doubleCompare;
      }

      doubleCompare = Double.compare(yValue, otherPoint.getYValue());

      return doubleCompare;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {

      if (!(obj instanceof Point)) {
         return false;
      }

      Point otherPoint = (Point) obj;

      return (this.compareTo(otherPoint) == 0);

   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {

      return Objects.hash(Double.valueOf(xValue), Double.valueOf(yValue));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Point#getXValue()
    */
   public double getXValue() {
      return xValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Point#getYValue()
    */
   public double getYValue() {
      return yValue;
   }

   /**
    * Add a 'Vector2D' to this 'Point'
    * 
    * @param vector
    *           - a 'Vector2D'
    * 
    * @return a new 'Point'
    */
   public Point plus(Vector2D vector) {
      return new Point(xValue + vector.x, yValue + vector.y);
   }

   /**
    * Subtract a 'Point' from this 'Point' to get a delta vector
    * 
    * @param point
    * 
    * @return a delta 'Vector2D'
    */
   public Vector2D minus(Point point) {
      return new Vector2D(xValue - point.xValue, yValue - point.yValue);
   }
}
