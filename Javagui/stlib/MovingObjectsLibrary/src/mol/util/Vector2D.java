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

import mol.datatypes.spatial.Point;
import mol.interfaces.spatial.PointIF;

/**
 * Class to represent 'Vector2D' objects.<br>
 * A simple two dimensional vector, used for calculations.
 * 
 * @author Markus Fuessel
 */
public class Vector2D {

   /**
    * x value, public for easy use
    */
   public final double x;

   /**
    * y value, public for easy use
    */
   public final double y;

   /**
    * Constructs the vector
    * 
    * @param x
    * 
    * @param y
    */
   public Vector2D(double x, double y) {
      this.x = x;
      this.y = y;
   }

   /**
    * Negate this vector
    * 
    * @return new negated vector
    */
   public Vector2D neg() {
      return new Vector2D(-x, -y);
   }

   /**
    * Subtract a other vector from this vector
    * 
    * @param other
    *           - the other vector
    * 
    * @return new vector
    */
   public Vector2D minus(Vector2D other) {
      return new Vector2D(x - other.x, y - other.y);
   }

   /**
    * Multiply this vector by the passed scalar
    * 
    * @param scalar
    *           - the scalar to multiply with
    * 
    * @return new vector
    */
   public Vector2D scale(double scalar) {
      return new Vector2D(x * scalar, y * scalar);
   }

   /**
    * cross product between this and the passed vector
    * 
    * @param other
    *           - the other vector
    * 
    * @return cross product
    */
   public double cross(Vector2D other) {
      return x * other.y - y * other.x;
   }

   /**
    * Scalar product between this and the passed vector
    * 
    * @param other
    *           - the other vector
    * 
    * @return scalar product
    */
   public double product(Vector2D other) {
      return x * other.x + y * other.y;
   }

   /**
    * Create a 'Point' from this vector
    * 
    * @return new 'Point'
    */
   public PointIF toPoint() {
      return new Point(x, y);
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
      long temp;
      temp = Double.doubleToLongBits(x);
      result = prime * result + (int) (temp ^ (temp >>> 32));
      temp = Double.doubleToLongBits(y);
      result = prime * result + (int) (temp ^ (temp >>> 32));
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

      if (!(obj instanceof Vector2D)) {
         return false;
      }

      Vector2D other = (Vector2D) obj;
      if (Double.doubleToLongBits(x) != Double.doubleToLongBits(other.x)) {
         return false;
      }

      return (Double.doubleToLongBits(y) == Double.doubleToLongBits(other.y));
   }

}
