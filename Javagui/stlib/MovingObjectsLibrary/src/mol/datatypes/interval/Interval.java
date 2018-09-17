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
import mol.datatypes.features.Orderable;

/**
 * Abstract Superclass for all interval subclasses
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 *           - specifies the type of the interval
 */
public abstract class Interval<T extends GeneralType & Orderable<T>> extends GeneralType
      implements Comparable<Interval<T>> {

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
    * Constructor for an undefined 'Interval' object
    */
   protected Interval() {
   }

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

      this.lowerBound = lowerBound;
      this.upperBound = upperBound;
      this.leftClosed = leftClosed;
      this.rightClosed = rightClosed;

      setDefined(isValid());
   }

   /**
    * Verifies if the interval contains the passed value
    * 
    * @param value
    * 
    * @return true if the interval contains the passed value, false otherwise
    */
   public boolean contains(final T value) {

      return contains(value, false);
   }

   /**
    * Verifies if the interval contains the passed value.<br>
    * With {@code ignoreClosedFlags} set to true this interval is considered as
    * closed.
    * 
    * @param value
    * @param ignoreClosedFlags
    *           - the left closed and right closed flags are ignored if true
    * 
    * @return true if the interval contains the passed value, false otherwise
    */
   public boolean contains(final T value, final boolean ignoreClosedFlags) {
      if (value == null || !value.isDefined() || !isDefined()) {
         return false;
      }

      int compareToLowerBound = value.compareTo(lowerBound);
      int compareToUpperBound = value.compareTo(upperBound);

      if (compareToLowerBound < 0 || compareToUpperBound > 0) {
         return false;
      }

      return !((!(leftClosed || ignoreClosedFlags) && compareToLowerBound == 0)
            || (!(rightClosed || ignoreClosedFlags) && compareToUpperBound == 0));
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    */
   @Override
   public int compareTo(final Interval<T> otherInterval) {

      int compare = lowerBound.compareTo(otherInterval.lowerBound);

      if (compare != 0) {
         return compare;
      }

      compare = Boolean.compare(otherInterval.leftClosed, leftClosed);

      if (compare != 0) {
         return compare;
      }

      compare = upperBound.compareTo(otherInterval.upperBound);

      if (compare != 0) {
         return compare;
      }

      compare = Boolean.compare(rightClosed, otherInterval.rightClosed);

      return compare;

   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {

      if (!(obj instanceof Interval<?>)) {
         return false;
      }

      if (!(this.getClass().equals(obj.getClass()))) {
         return false;
      }

      @SuppressWarnings("unchecked")
      Interval<T> otherInterval = (Interval<T>) obj;

      return (this.isDefined() && otherInterval.isDefined() && this.compareTo(otherInterval) == 0);
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
   // @Override
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
      int compare = lowerBound.compareTo(otherInterval.upperBound);

      if (compare == 0 && (leftClosed != otherInterval.rightClosed)) {
         return true;
      }

      return (((compare > 0 && leftClosed && otherInterval.rightClosed)
            || (compare < 0 && !leftClosed && !otherInterval.rightClosed))
            && lowerBound.adjacent(otherInterval.upperBound));
   }

   /**
    * Check if this interval is adjacent with its right boundary (upper bound) to
    * the passed interval
    * 
    * @param otherInterval
    * @return true - interval is right adjacent, false - otherwise
    */
   public boolean rightAdjacent(final Interval<T> otherInterval) {
      int compare = upperBound.compareTo(otherInterval.lowerBound);

      if (compare == 0 && (rightClosed != otherInterval.leftClosed)) {

         return true;
      }

      return (((compare < 0 && rightClosed && otherInterval.leftClosed)
            || (compare > 0 && !rightClosed && !otherInterval.leftClosed))
            && upperBound.adjacent(otherInterval.lowerBound));
   }

   /**
    * Verify if this 'Interval' object is complete before the passed 'Interval'
    * object
    * 
    * @param otherInterval
    * @return true if this object is before the passed one, false otherwise
    */
   public boolean before(final Interval<T> otherInterval) {

      int compare = upperBound.compareTo(otherInterval.lowerBound);

      return (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.leftClosed)));

   }

   /**
    * Verify if this 'Interval' object is before the passed value
    * 
    * @param value
    * @return true if this object is before the passed value, false otherwise
    */
   public boolean before(final T value) {

      int compare = upperBound.compareTo(value);

      return (compare < 0 || (compare == 0 && !rightClosed));
   }

   /**
    * Verify if this 'Interval' object is after the passed 'Interval' object
    * 
    * @param otherInterval
    * @return true if this object is after the passed one, false otherwise
    */
   public boolean after(final Interval<T> otherInterval) {
      int compare = lowerBound.compareTo(otherInterval.upperBound);

      return (compare > 0 || (compare == 0 && (!leftClosed || !otherInterval.rightClosed)));
   }

   /**
    * Verify if this 'Interval' object is after the passed value
    * 
    * @param value
    * @return true if this object is after the passed value, false otherwise
    */
   public boolean after(final T value) {

      int compare = lowerBound.compareTo(value);

      return (compare > 0 || (compare == 0 && !leftClosed));
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

      return ((upperBound.compareTo(otherInterval.lowerBound) == 0 && !(rightClosed && otherInterval.leftClosed))
            || (lowerBound.compareTo(otherInterval.upperBound) == 0 && !(leftClosed && otherInterval.rightClosed)));
   }

   /**
    * Check if the passed interval intersects this interval
    * 
    * @param otherInterval
    * @return true if the passed interval intersects with this interval, false
    *         otherwise
    */
   public boolean intersects(final Interval<T> otherInterval) {

      int compare = upperBound.compareTo(otherInterval.lowerBound);

      if (compare < 0 || (compare == 0 && !(rightClosed && otherInterval.leftClosed))) {
         return false;
      }

      compare = lowerBound.compareTo(otherInterval.upperBound);

      return !(compare > 0 || (compare == 0 && !(leftClosed && otherInterval.rightClosed)));
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

      if (compare > 0 || (compare == 0 && (!leftClosed && otherInterval.leftClosed))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.lowerBound);

      if (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.leftClosed))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.upperBound);

      return !(compare > 0 || (compare == 0 && (rightClosed && !otherInterval.rightClosed)));
   }

   /**
    * Check if this interval intersects exactly on the right bound (upper bound)
    * with the passed interval.
    * 
    * @param otherInterval
    * 
    * @return true if this interval intersects exactly on the right bound (upper
    *         bound) with the passed interval, false otherwise
    */
   public boolean intersectsOnRightBound(final Interval<T> otherInterval) {
      int compare = upperBound.compareTo(otherInterval.lowerBound);

      return compare == 0 && rightClosed && otherInterval.leftClosed;
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
    * Creates the intersection of this and the passed interval as a new interval
    * object
    * 
    * @param otherInterval
    * @return new interval object
    */
   public Interval<T> intersection(final Interval<T> otherInterval) {

      Interval<T> intersectionInterval = this.copy();

      int compare = intersectionInterval.lowerBound.compareTo(otherInterval.lowerBound);

      if (compare == 0) {
         intersectionInterval.leftClosed = intersectionInterval.leftClosed && otherInterval.leftClosed;
      } else if (compare < 0) {
         intersectionInterval.lowerBound = otherInterval.lowerBound;
         intersectionInterval.leftClosed = otherInterval.leftClosed;
      }

      compare = intersectionInterval.upperBound.compareTo(otherInterval.upperBound);

      if (compare == 0) {
         intersectionInterval.rightClosed = intersectionInterval.rightClosed && otherInterval.rightClosed;
      } else if (compare > 0) {
         intersectionInterval.upperBound = otherInterval.upperBound;
         intersectionInterval.rightClosed = otherInterval.rightClosed;
      }

      intersectionInterval.setDefined(intersectionInterval.isValid());

      return intersectionInterval;
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

   /**
    * Set if interval is left closed
    * 
    * @param leftClosed
    */
   public void setLeftClosed(boolean leftClosed) {
      this.leftClosed = leftClosed;
   }

   /**
    * Set if interval is right closed
    * 
    * @param rightClosed
    */
   public void setRightClosed(boolean rightClosed) {
      this.rightClosed = rightClosed;
   }

   /**
    * Verify if this 'Interval' object is valid<br>
    * 
    * - lowerBound and upperBound are defined<br>
    * - lowerBound <= upperBound <br>
    * - is closed if lowerBound == upperBound <br>
    * 
    * @return true if this 'Interval' object is valid, false otherwise
    */
   public boolean isValid() {

      return lowerBound.isDefined() && upperBound.isDefined() && (lowerBound.compareTo(upperBound) < 0
            || (lowerBound.compareTo(upperBound) == 0 && leftClosed && rightClosed));
   }

}
