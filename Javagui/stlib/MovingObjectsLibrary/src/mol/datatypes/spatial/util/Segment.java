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

import mol.datatypes.features.Spatial;
import mol.datatypes.spatial.Point;
import mol.util.GeneralHelper;

/**
 * Class to represent segment objects, generally used for representing the
 * spatial line objects
 * 
 * @author Markus Fuessel
 */
public class Segment implements Spatial {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Left endpoint of this segment
    */
   private final Point leftPoint;

   /**
    * Right endpoint of this segment
    */
   private final Point rightPoint;

   /**
    * Constructor for a empty and undefined 'Segment' object.<br>
    * Should not normally be used
    */
   public Segment() {
      leftPoint = new Point();
      rightPoint = new Point();

      setDefined(false);
   }

   /**
    * Constructor for an 'Segment' object<br>
    * The passed points will be assigned to the internal leftPoint and rightPoint
    * by the following rule:<br>
    * leftPoint = min(p0, p1)<br>
    * rightPoint = max(p0, p1)
    * 
    * @param p0
    *           - first endpoint
    * @param p1
    *           - second endpoint
    */
   public Segment(final Point p0, final Point p1) {
      if (p0.compareTo(p1) <= 0) {
         this.leftPoint = p0;
         this.rightPoint = p1;
      } else {
         this.leftPoint = p1;
         this.rightPoint = p0;
      }

      setDefined(p0.isDefined() && p1.isDefined());
   }

   /**
    * Constructor for an 'Segment' object<br>
    * The passed coordinates will be assigned to the internal leftPoint and
    * rightPoint by the following rule:<br>
    * leftPoint = min(Point(x0, y0), Point(x1, y1))<br>
    * rightPoint = max(Point(x0, y0), Point(x1, y1))
    * 
    * @param x0
    *           - x value of first endpoint
    * @param y0
    *           - y value of first endpoint
    * @param x1
    *           - x value of second endpoint
    * @param y1
    *           - x value of second endpoint
    */
   public Segment(final double x0, final double y0, final double x1, final double y1) {
      this(new Point(x0, y0), new Point(x1, y1));
   }

   /**
    * Verify if this 'Segment' intersects with the passed other 'Segment'.<br>
    * - Intersection thru left endpoint is a valid intersection<br>
    * - Intersection thru right endpoint is not a valid intersection
    * 
    * @param other
    *           - the other 'Segment'
    * 
    * @return true - both 'Segment' objects intersect each other, false - otherwise
    */
   public boolean intersect(final Segment other) {

      int ccwLpRpOLp = GeneralHelper.counterClockwisePath(leftPoint, rightPoint, other.leftPoint);
      int ccwLpRpORp = GeneralHelper.counterClockwisePath(leftPoint, rightPoint, other.rightPoint);

      if (((ccwLpRpOLp == 0 && ccwLpRpORp != 0) || ccwLpRpOLp * ccwLpRpORp < 0)) {

         int ccwOLpORpLp = GeneralHelper.counterClockwisePath(other.leftPoint, other.rightPoint, leftPoint);
         int ccwOLpORpRp = GeneralHelper.counterClockwisePath(other.leftPoint, other.rightPoint, rightPoint);

         return ((ccwOLpORpLp == 0 && ccwOLpORpRp != 0) || ccwOLpORpLp * ccwOLpORpRp < 0);
      }

      return false;
   }

   /**
    * Is the alignment of this segment is vertical
    * 
    * @return true - alignment is vertical, false otherwise
    */
   public boolean isVertical() {

      double x0 = leftPoint.getXValue();
      double x1 = rightPoint.getXValue();

      return Double.compare(x0, x1) == 0;
   }

   /**
    * Verify if the end points of this segment are very close to each other
    * 
    * @return true if the end points are very close to each other, false otherwise
    */
   public boolean isAlmostAPoint() {
      return leftPoint.almostEqual(rightPoint);
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

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.Spatial#getBoundingBox()
    */
   @Override
   public Rectangle getBoundingBox() {
      return leftPoint.getBoundingBox().merge(rightPoint.getBoundingBox());
   }

   /**
    * Get the length of this 'Segment'
    * 
    * @param useSphericalGeometry
    *           - if true spherical geometry is used to calculate the length in
    *           metres, otherwise euclidean distance is used
    * 
    * @return euclidean or geographical length, depends on parameter
    *         useSphericalGeometry
    */
   public double length(final boolean useSphericalGeometry) {
      return leftPoint.distance(rightPoint, useSphericalGeometry);
   }

   /**
    * Get the lower endpoint
    * 
    * @return the leftPoint
    */
   public Point getLeftPoint() {
      return leftPoint;
   }

   /**
    * Get the greater endpoint
    * 
    * @return the rightPoint
    */
   public Point getRightPoint() {
      return rightPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      final int prime = 31;
      int result = 1;
      result = prime * result + ((leftPoint == null) ? 0 : leftPoint.hashCode());
      result = prime * result + ((rightPoint == null) ? 0 : rightPoint.hashCode());
      return result;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(Object obj) {
      if (this == obj) {
         return true;
      }

      if (!(obj instanceof Segment)) {
         return false;
      }

      Segment otherSegment = (Segment) obj;

      if (!isDefined() || !otherSegment.isDefined()) {
         return false;
      }

      return leftPoint.equals(otherSegment.leftPoint) && rightPoint.equals(otherSegment.rightPoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.IGeneralType#isDefined()
    */
   public boolean isDefined() {
      return defined;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.IGeneralType#setDefined(boolean)
    */
   public void setDefined(final boolean defined) {
      this.defined = defined;
   }

}
