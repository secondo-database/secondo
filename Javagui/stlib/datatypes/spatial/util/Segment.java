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

package stlib.datatypes.spatial.util;

import stlib.datatypes.spatial.Point;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.spatial.util.RectangleIF;
import stlib.interfaces.spatial.util.SegmentIF;
import stlib.util.GeneralHelper;

/**
 * Class to represent segment objects, generally used for representing the
 * spatial line objects
 * 
 * @author Markus Fuessel
 */
public class Segment implements SegmentIF {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Left endpoint of this segment
    */
   private final PointIF leftPoint;

   /**
    * Right endpoint of this segment
    */
   private final PointIF rightPoint;

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
   public Segment(final PointIF p0, final PointIF p1) {
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

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.spatial.util.SegmentIF#intersect(stlib.interfaces.spatial.
    * util. SegmentIF)
    */
   @Override
   public boolean intersect(final SegmentIF other) {

      int ccwLpRpOLp = GeneralHelper.counterClockwisePath(leftPoint, rightPoint, other.getLeftPoint());
      int ccwLpRpORp = GeneralHelper.counterClockwisePath(leftPoint, rightPoint, other.getRightPoint());

      if (((ccwLpRpOLp == 0 && ccwLpRpORp != 0) || ccwLpRpOLp * ccwLpRpORp < 0)) {

         int ccwOLpORpLp = GeneralHelper.counterClockwisePath(other.getLeftPoint(), other.getRightPoint(), leftPoint);
         int ccwOLpORpRp = GeneralHelper.counterClockwisePath(other.getLeftPoint(), other.getRightPoint(), rightPoint);

         return ((ccwOLpORpLp == 0 && ccwOLpORpRp != 0) || ccwOLpORpLp * ccwOLpORpRp < 0);
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.SegmentIF#isVertical()
    */
   @Override
   public boolean isVertical() {

      double x0 = leftPoint.getXValue();
      double x1 = rightPoint.getXValue();

      return Double.compare(x0, x1) == 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.SegmentIF#isAlmostAPoint()
    */
   @Override
   public boolean isAlmostAPoint() {
      return leftPoint.almostEqual(rightPoint);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.Spatial#isEmpty()
    */
   @Override
   public boolean isEmpty() {
      return !isDefined();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.features.Spatial#getBoundingBox()
    */
   @Override
   public RectangleIF getBoundingBox() {
      return leftPoint.getBoundingBox().merge(rightPoint.getBoundingBox());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.SegmentIF#length(boolean)
    */
   @Override
   public double length(final boolean useSphericalGeometry) {
      return leftPoint.distance(rightPoint, useSphericalGeometry);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.SegmentIF#getLeftPoint()
    */
   @Override
   public PointIF getLeftPoint() {
      return leftPoint;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.SegmentIF#getRightPoint()
    */
   @Override
   public PointIF getRightPoint() {
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

      if (!(obj instanceof SegmentIF)) {
         return false;
      }

      SegmentIF otherSegment = (SegmentIF) obj;

      if (!isDefined() || !otherSegment.isDefined()) {
         return false;
      }

      return leftPoint.equals(otherSegment.getLeftPoint()) && rightPoint.equals(otherSegment.getRightPoint());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.IGeneralType#isDefined()
    */
   public boolean isDefined() {
      return defined;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.IGeneralType#setDefined(boolean)
    */
   public void setDefined(final boolean defined) {
      this.defined = defined;
   }

}
