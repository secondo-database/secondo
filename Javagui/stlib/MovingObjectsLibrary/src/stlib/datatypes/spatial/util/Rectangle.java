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

import java.util.Objects;

import stlib.interfaces.spatial.util.RectangleIF;

/**
 * Class to represent rectangle objects, generally used for representing the
 * bounding box of spatial objects
 * 
 * @author Markus Fuessel
 *
 */
public class Rectangle implements RectangleIF {

   /**
    * The defined flag, indicates if a data type object is defined
    */
   private boolean defined;

   /**
    * Left value of the rectangle, defines left edge
    */
   private final double leftValue;

   /**
    * Right value of the rectangle, defines right edge
    */
   private final double rightValue;

   /**
    * Top value of the rectangle, defines upper edge
    */
   private final double topValue;

   /**
    * Bottom value of the rectangle, defines bottom edge
    */
   private final double bottomValue;

   /**
    * Constructor for undefined rectangle
    */
   public Rectangle() {
      this.leftValue = -1;
      this.rightValue = -1;
      this.topValue = -1;
      this.bottomValue = -1;

      setDefined(false);
   }

   /**
    * Constructor for a rectangle
    * 
    * @param leftValue
    *           - left edge oth the rectangle
    * @param rightValue
    *           - right edge of the rectangle
    * @param topValue
    *           - upper edge of the rectangle
    * @param bottomValue
    *           - bottom edge of the rectangle
    */
   public Rectangle(final double leftValue, final double rightValue, final double topValue, final double bottomValue) {
      this.leftValue = leftValue;
      this.rightValue = rightValue;
      this.topValue = topValue;
      this.bottomValue = bottomValue;

      setDefined(true);
   }

   /**
    * Copy constructor for a rectangle, creates a new rectangle based on the passed
    * rectangle
    * 
    * @param rect
    */
   private Rectangle(final RectangleIF rect) {
      this(rect.getLeftValue(), rect.getRightValue(), rect.getTopValue(), rect.getBottomValue());
      setDefined(rect.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.RectangleIF#copy()
    */
   @Override
   public RectangleIF copy() {
      return new Rectangle(this);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.spatial.util.RectangleIF#merge(stlib.interfaces.spatial.
    * util. RectangleIF)
    */
   @Override
   public RectangleIF merge(final RectangleIF rect) {

      if (this.isDefined() && rect.isDefined()) {
         double newLeftValue = Math.min(this.leftValue, rect.getLeftValue());
         double newRightValue = Math.max(this.rightValue, rect.getRightValue());
         double newTopValue = Math.max(this.topValue, rect.getTopValue());
         double newBottomValue = Math.min(this.bottomValue, rect.getBottomValue());

         return new Rectangle(newLeftValue, newRightValue, newTopValue, newBottomValue);

      } else if (this.isDefined()) {
         return this.copy();
      } else {
         return rect.copy();
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.spatial.util.RectangleIF#intersects(stlib.interfaces.spatial
    * .util. RectangleIF)
    */
   @Override
   public boolean intersects(final RectangleIF rect) {

      if (this.leftValue > rect.getRightValue() || this.rightValue < rect.getLeftValue()) {
         return false;
      }

      return !(this.bottomValue > rect.getTopValue() || this.topValue < rect.getBottomValue());

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

      return this.copy();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.RectangleIF#getLeftValue()
    */
   @Override
   public double getLeftValue() {
      return leftValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.RectangleIF#getRightValue()
    */
   @Override
   public double getRightValue() {
      return rightValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.RectangleIF#getTopValue()
    */
   @Override
   public double getTopValue() {
      return topValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.spatial.util.RectangleIF#getBottomValue()
    */
   @Override
   public double getBottomValue() {
      return bottomValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {
      return Objects.hash(Double.valueOf(leftValue), Double.valueOf(rightValue), Double.valueOf(topValue),
            Double.valueOf(bottomValue), Boolean.valueOf(isDefined()));
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(Object obj) {

      if (!(obj instanceof RectangleIF)) {
         return false;
      }

      if (this == obj) {
         return true;

      } else {

         RectangleIF otherRect = (RectangleIF) obj;

         return (this.leftValue == otherRect.getLeftValue() && this.rightValue == otherRect.getRightValue()
               && this.topValue == otherRect.getTopValue() && this.bottomValue == otherRect.getBottomValue());

      }
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
