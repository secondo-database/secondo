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
import mol.datatypes.spatial.util.Rectangle;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.util.RectangleIF;
import mol.util.GeneralHelper;
import mol.util.Vector2D;

/**
 * This class represents spatial objects of type 'Point'
 * 
 * @author Markus Fuessel
 */
public class Point extends GeneralType implements PointIF {

   /**
    * Get the delta between start and end point on the x axis
    * 
    * @return the x axis delta
    */
   public static double getDeltaX(final PointIF startPoint, final PointIF endPoint) {
      return endPoint.getXValue() - startPoint.getXValue();
   }

   /**
    * Get the delta between start and end point on the y axis
    * 
    * @return the y axis delta
    */
   public static double getDeltaY(final PointIF startPoint, final PointIF endPoint) {
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
   public RectangleIF getBoundingBox() {
      return new Rectangle(xValue, xValue, yValue, yValue);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#distance(mol.datatypes.spatial.Point,
    * boolean)
    */
   @Override
   public double distance(final PointIF otherPoint, final boolean useSphericalGeometry) {

      if (useSphericalGeometry) {
         return GeneralHelper.distanceOrthodrome(xValue, yValue, otherPoint.getXValue(), otherPoint.getYValue());
      } else {
         return GeneralHelper.distanceEuclidean(xValue, yValue, otherPoint.getXValue(), otherPoint.getYValue());
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#almostEqual(mol.datatypes.spatial.Point)
    */
   @Override
   public boolean almostEqual(final PointIF otherPoint) {
      return GeneralHelper.almostEqual(this.xValue, otherPoint.getXValue())
            && GeneralHelper.almostEqual(this.yValue, otherPoint.getYValue());
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
   public int compareTo(final PointIF otherPoint) {

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

      if (!(obj instanceof PointIF)) {
         return false;
      }

      PointIF otherPoint = (PointIF) obj;

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
   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#getXValue()
    */
   @Override
   public double getXValue() {
      return xValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#getYValue()
    */
   @Override
   public double getYValue() {
      return yValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#plus(mol.util.Vector2D)
    */
   @Override
   public PointIF plus(Vector2D vector) {
      return new Point(xValue + vector.x, yValue + vector.y);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.spatial.PointIF#minus(mol.datatypes.spatial.Point)
    */
   @Override
   public Vector2D minus(PointIF point) {
      return new Vector2D(xValue - point.getXValue(), yValue - point.getYValue());
   }
}
