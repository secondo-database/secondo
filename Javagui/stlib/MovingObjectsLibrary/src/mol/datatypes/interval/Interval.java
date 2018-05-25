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

package mol.datatypes.interval;

import java.util.Objects;

import mol.datatypes.GeneralType;
import mol.datatypes.IGeneralType;
import mol.datatypes.features.Orderable;

/**
 * Abstract Superclass for all interval subclasses
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 *           - specifies the type of the interval
 */
public abstract class Interval<T extends IGeneralType & Orderable<T>> extends GeneralType
      implements Orderable<Interval<T>> {

   /**
    * Check to ensure condition: lowerBound <= upperBound
    * 
    * @param lowerBound
    *           - the lower bound value
    * @param upperBound
    *           - the upper bound value
    */
   private static <T extends Comparable<T>> void assertLowerBoundLessOrEqualUpperBound(final T lowerBound,
                                                                                       final T upperBound) {

      if (lowerBound.compareTo(upperBound) > 0) {
         throw new IllegalArgumentException("'lowerBound' must be <= 'upperBound'");
      }
   }

   /**
    * Check to ensure condition: lowerBound == upperBound => leftClosed ==
    * rightClosed == true
    * 
    * @param lowerBound
    *           - the lower bound value
    * @param upperBound
    *           - the upper bound value
    * @param leftClosed
    *           - boolean for left closness
    * @param rightClosed
    *           - boolean for right closness
    */
   private static <T extends Comparable<T>> void assertClosedIntervalIfLowerBoundEqualUpperBound(final T lowerBound,
                                                                                                 final T upperBound,
                                                                                                 final boolean leftClosed,
                                                                                                 final boolean rightClosed) {

      if (lowerBound.compareTo(upperBound) == 0 && !(leftClosed && rightClosed)) {
         throw new IllegalArgumentException(
               "interval have to be leftclosed and rightclosed if 'lowerBound' == 'upperBound'");
      }
   }

   /**
    * Lower bound of the interval
    */
   private T lowerBound;

   /**
    * Upper bound of the interval
    */
   private T upperBound;

   /**
    * Is the interval left closed
    */
   private boolean leftClosed;

   /**
    * Is the interval right closed
    */
   private boolean rightClosed;

   /**
    * Base constructor for a 'Interval' object
    * 
    * @param lowerBound
    *           - T, the lower bound value
    * @param upperBound
    *           - T, the upper bound value
    * @param leftClosed
    *           - boolean for left closness
    * @param rightClosed
    *           - boolean for right closness
    */
   protected Interval(final T lowerBound, final T upperBound, final boolean leftClosed, final boolean rightClosed) {

      Objects.requireNonNull(lowerBound, "'lowerBound' must not be null");
      Objects.requireNonNull(upperBound, "'upperBound' must not be null");

      assertLowerBoundLessOrEqualUpperBound(lowerBound, upperBound);
      assertClosedIntervalIfLowerBoundEqualUpperBound(lowerBound, upperBound, leftClosed, rightClosed);

      this.lowerBound = lowerBound;
      this.upperBound = upperBound;
      this.leftClosed = leftClosed;
      this.rightClosed = rightClosed;

      setDefined(lowerBound.isDefined() && upperBound.isDefined());
   }

   /**
    * Verifies if the interval contains the passed value
    * 
    * @param value
    * 
    * @return true if the interval contains the passed value, false otherwise
    */
   public boolean contains(final T value) {
      if (value == null) {
         return false;
      }

      if (value.compareTo(lowerBound) < 0 || value.compareTo(upperBound) > 0) {
         return false;
      }

      if ((!leftClosed && value.compareTo(lowerBound) <= 0) || (!rightClosed && value.compareTo(lowerBound) >= 0)) {
         return false;
      }

      return true;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final Interval<T> otherInterval) {

      int comp = lowerBound.compareTo(otherInterval.lowerBound);

      if (comp != 0) {
         return comp;
      }

      comp = Boolean.compare(otherInterval.leftClosed, leftClosed);

      if (comp != 0) {
         return comp;
      }

      comp = upperBound.compareTo(otherInterval.upperBound);

      if (comp != 0) {
         return comp;
      }

      comp = Boolean.compare(rightClosed, otherInterval.rightClosed);

      return comp;

   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {

      if (obj == null) {
         return false;
      }

      if (!(obj instanceof Interval<?>) || !(this.getClass().equals(obj.getClass()))) {
         return false;
      }

      @SuppressWarnings("unchecked")
      Interval<T> otherInterval = (Interval<T>) obj;

      return (this.compareTo(otherInterval) == 0);
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#hashCode()
    */
   @Override
   public int hashCode() {

      return Objects.hash(lowerBound, upperBound, Boolean.valueOf(leftClosed), Boolean.valueOf(rightClosed));

   }

   /**
    * Check if two intervals are adjacent
    * 
    * @param otherInterval
    * @return true - intervals are adjacent, false - otherwise
    */
   @Override
   public boolean adjacent(final Interval<T> otherInterval) {

      return leftAdjacent(otherInterval) || rightAdjacent(otherInterval);
   }

   /**
    * Check if this interval is adjacent with its left boundary (lower bound) to
    * the passed interval
    * 
    * @param otherInterval
    * @return true - interval is left adjacent, false - otherwise
    */
   public boolean leftAdjacent(final Interval<T> otherInterval) {
      if (lowerBound.compareTo(otherInterval.upperBound) == 0 && (leftClosed != otherInterval.rightClosed)) {
         return true;
      }

      if (((lowerBound.compareTo(otherInterval.upperBound) > 0 && leftClosed && otherInterval.rightClosed)
            || (lowerBound.compareTo(otherInterval.upperBound) < 0 && !leftClosed && !otherInterval.rightClosed))
            && lowerBound.adjacent(otherInterval.upperBound)) {
         return true;
      }

      return false;
   }

   /**
    * Check if this interval is adjacent with its right boundary (upper bound) to
    * the passed interval
    * 
    * @param otherInterval
    * @return true - interval is right adjacent, false - otherwise
    */
   public boolean rightAdjacent(final Interval<T> otherInterval) {
      if (upperBound.compareTo(otherInterval.lowerBound) == 0 && (rightClosed != otherInterval.leftClosed)) {

         return true;
      }

      if (((upperBound.compareTo(otherInterval.lowerBound) < 0 && rightClosed && otherInterval.leftClosed)
            || (upperBound.compareTo(otherInterval.lowerBound) > 0 && !rightClosed && !otherInterval.leftClosed))
            && upperBound.adjacent(otherInterval.lowerBound)) {

         return true;
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#before(java.lang.Object)
    */
   @Override
   public boolean before(final Interval<T> otherInterval) {

      int compare = upperBound.compareTo(otherInterval.lowerBound);

      if (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.leftClosed))) {
         return true;
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.util.Orderable#after(java.lang.Object)
    */
   @Override
   public boolean after(final Interval<T> otherInterval) {
      int compare = lowerBound.compareTo(otherInterval.upperBound);

      if (compare > 0 || (compare == 0 && (!leftClosed || !otherInterval.rightClosed))) {
         return true;
      }

      return false;
   }

   /**
    * This method checks if this interval and the passed interval are disjoint
    * 
    * @param otherInterval
    * @return true - intervals are disjoint, false otherwise
    */
   public boolean disjoint(final Interval<T> otherInterval) {

      if (upperBound.compareTo(otherInterval.lowerBound) < 0 || lowerBound.compareTo(otherInterval.upperBound) > 0) {
         return true;
      }

      if ((upperBound.compareTo(otherInterval.lowerBound) == 0 && !(rightClosed && otherInterval.leftClosed))
            || (lowerBound.compareTo(otherInterval.upperBound) == 0 && !(leftClosed && otherInterval.rightClosed))) {
         return true;
      }

      return false;
   }

   /**
    * Check if the passed interval intersects this interval
    * 
    * @param otherInterval
    * @return true if the passed interval intersects with this interval, false
    *         otherwise
    */
   public boolean intersects(final Interval<T> otherInterval) {

      int comp = upperBound.compareTo(otherInterval.lowerBound);

      if (comp < 0 || (comp == 0 && !(rightClosed && otherInterval.leftClosed))) {
         return false;
      }

      comp = lowerBound.compareTo(otherInterval.upperBound);

      if (comp > 0 || (comp == 0 && !(rightClosed && otherInterval.leftClosed))) {
         return false;
      }

      return true;
   }

   /**
    * If this interval intersects with its lower bound (left side) by the passed
    * one
    * 
    * @param otherInterval
    * @return true if this interval is intersected left, false otherwise
    */
   public boolean intersectsLeft(final Interval<T> otherInterval) {

      return otherInterval.intersectsRight(this);
   }

   /**
    * If this interval intersects with its upper bound (right side) by the passed
    * one
    * 
    * @param otherInterval
    * @return true if this interval is intersected right, false otherwise
    */
   public boolean intersectsRight(final Interval<T> otherInterval) {

      int compare = lowerBound.compareTo(otherInterval.lowerBound);

      if (compare > 0 || (compare == 0 && (!leftClosed || otherInterval.leftClosed))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.lowerBound);

      if (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.leftClosed))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.upperBound);

      if (compare > 0 || (compare == 0 && (rightClosed || !otherInterval.rightClosed))) {
         return false;
      }

      return true;
   }

   /**
    * Creates a new interval object where this interval is merged with the passed
    * one
    * 
    * @param otherInterval
    *           - the other interval to merge with
    * @return new interval object
    */
   public Interval<T> merge(final Interval<T> otherInterval) {

      return mergeLeft(otherInterval).mergeRight(otherInterval);
   }

   /**
    * Creates a new interval object where this interval is merged on the left with
    * the passed interval
    * 
    * @param otherInterval
    * @return new interval object
    */
   public Interval<T> mergeLeft(final Interval<T> otherInterval) {
      Interval<T> mergedInterval = this.copy();

      int compare = mergedInterval.lowerBound.compareTo(otherInterval.lowerBound);

      if (compare == 0) {
         mergedInterval.leftClosed = mergedInterval.leftClosed || otherInterval.leftClosed;
      } else if (compare > 0) {
         mergedInterval.lowerBound = otherInterval.lowerBound;
         mergedInterval.leftClosed = otherInterval.leftClosed;
      }

      return mergedInterval;
   }

   /**
    * Creates a new interval object where this interval is merged on the right with
    * the passed interval
    * 
    * @param otherInterval
    * @return new interval object
    */
   public Interval<T> mergeRight(final Interval<T> otherInterval) {
      Interval<T> mergedInterval = this.copy();

      int compare = mergedInterval.upperBound.compareTo(otherInterval.upperBound);

      if (compare == 0) {
         mergedInterval.rightClosed = mergedInterval.rightClosed || otherInterval.rightClosed;
      } else if (compare < 0) {
         mergedInterval.upperBound = otherInterval.upperBound;
         mergedInterval.rightClosed = otherInterval.rightClosed;
      }

      return mergedInterval;
   }

   /**
    * Returns a copy of this {@code Interval<T>} object
    * 
    * @return {@code Interval<T>} object copy
    */
   public abstract Interval<T> copy();

   /**
    * Get lower bound of the interval
    * 
    * @return the lowerBound
    */
   public T getLowerBound() {
      return lowerBound;
   }

   /**
    * Get the upper bound of the interval
    * 
    * @return the upperBound
    */
   public T getUpperBound() {
      return upperBound;
   }

   /**
    * Is interval left closed
    * 
    * @return the leftClosed
    */
   public boolean isLeftClosed() {
      return leftClosed;
   }

   /**
    * Is interval right closed
    * 
    * @return the rightClosed
    */
   public boolean isRightClosed() {
      return rightClosed;
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#toString()
    */
   @Override
   public String toString() {

      String leftParenthesis = (leftClosed ? "[" : "(");
      String rightParenthesis = (rightClosed ? "]" : ")");

      return "Interval " + leftParenthesis + lowerBound.toString() + ", " + upperBound.toString() + rightParenthesis;
   }

}
