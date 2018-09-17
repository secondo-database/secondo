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

package mol.datatypes.range;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import mol.datatypes.GeneralType;
import mol.datatypes.features.Orderable;
import mol.datatypes.interval.Interval;

/**
 * Base class for 'Range' objects.
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 */
public abstract class Range<T extends GeneralType & Orderable<T>> extends GeneralType {

   /**
    * Set of {@code 'Interval<T>'} objects
    */
   private final List<Interval<T>> intervals;

   /**
    * Simple constructor to create an empty 'Range' object with the specified
    * initial capacity
    * 
    * @param size
    *           - initial capacity of this 'Range' object
    */
   protected Range(final int size) {
      this.intervals = new ArrayList<>(size);
      this.setDefined(true);
   }

   /**
    * Append the passed interval object to this range set.<br>
    * The passed interval have to be disjoint and non adjacent to the current range
    * set.<br>
    * Only defined interval objects will be added
    * 
    * @param interval
    *           - the interval object to append
    * @return true (as specified by {@link Collection#add})
    */
   public boolean add(final Interval<T> interval) {
      if (interval.isDefined()) {
         return intervals.add(interval);
      }

      return false;
   }

   /**
    * Appends the passed interval to this 'Range' object. If the passed interval
    * intersects the last interval of this range set or is adjacent to it, the last
    * interval and the passed one would be merged to a new last interval of this
    * range set
    * 
    * @param interval
    *           - the interval object to append
    * @return true if adding of the passed interval was successful, false otherwise
    */
   public boolean mergeAdd(final Interval<T> interval) {
      if (isEmpty()) {
         return add(interval);

      } else {

         Interval<T> lastInterval = last();

         if (lastInterval.intersectsRight(interval) || lastInterval.rightAdjacent(interval)) {

            Interval<T> newLastInterval = lastInterval.mergeRight(interval);

            intervals.set(getNoComponents() - 1, newLastInterval);

            return true;

         } else if (lastInterval.before(interval)) {

            return add(interval);
         }
      }

      return false;
   }

   /**
    * Check if the passed interval intersects this range set
    * 
    * @param interval
    * @return true if the passed interval intersects at least with one interval of
    *         the range set
    */
   public boolean intersects(final Interval<T> interval) {

      if (!this.isDefined() || !interval.isDefined()) {
         return false;
      }

      Interval<T> centerIV;
      int centerPos;

      int firstPos = 0;
      int lastPos = getNoComponents() - 1;

      while (firstPos <= lastPos) {

         centerPos = (firstPos + lastPos) / 2;

         centerIV = this.get(centerPos);

         if (centerIV.intersects(interval)) {
            return true;

         } else if (centerIV.after(interval)) {
            lastPos = centerPos - 1;

         } else if (centerIV.before(interval)) {
            firstPos = centerPos + 1;
         }

      }

      return false;

   }

   /**
    * Check if the passed 'Range' intersects this range set
    * 
    * @param other
    * @return true if the passed 'Range' intersects at least with one interval of
    *         this range set
    */
   public boolean intersects(final Range<T> other) {

      if (!this.isDefined() || !other.isDefined()) {
         return false;
      }

      for (int i = 0; i < other.getNoComponents(); i++) {
         Interval<T> currentOtherInterval = other.get(i);

         if (this.intersects(currentOtherInterval)) {
            return true;
         }
      }

      return false;

   }

   /**
    * Verify if the passed interval is adjacent to this 'Range' object.
    * 
    * @param interval
    * 
    * @return true if the passed interval is left or right adjacent, false
    *         otherwise
    */
   public boolean adjacent(final Interval<T> interval) {

      return leftAdjacent(interval) || rightAdjacent(interval);

   }

   /**
    * Verify if this 'Range' object is adjacent on the right to the passed interval
    * 
    * @param interval
    * @return true if the passed interval is adjacent on the right, false otherwise
    */
   public boolean rightAdjacent(final Interval<T> interval) {
      return last().rightAdjacent(interval);
   }

   /**
    * Verify if this 'Range' object is adjacent on the left to the passed interval
    * 
    * @param interval
    * 
    * @return true if the passed interval is adjacent on the left, false otherwise
    */
   public boolean leftAdjacent(final Interval<T> interval) {
      return first().leftAdjacent(interval);
   }

   /**
    * Verify if this {@code 'Range<T>'} object is before the passed
    * {@code 'Interval<T>'} object
    * 
    * @param interval
    * @return true if this object is before the passed one, false otherwise
    */
   public boolean before(final Interval<T> interval) {

      return last().before(interval);

   }

   /**
    * Verify if this {@code 'Range<T>'} object is after the passed
    * {@code 'Interval<T>'} object
    * 
    * @param interval
    * @return true if this object is after the passed one, false otherwise
    */
   public boolean after(final Interval<T> interval) {
      return first().after(interval);

   }

   /**
    * Check whether the given value is covered by this range set.
    * 
    * @param value
    * 
    * @return true if value is covered by this range set, false otherwise
    */
   public boolean contains(final T value) {

      Interval<T> centerInterval;
      int centerIdx;

      int firstIdx = 0;
      int lastIdx = this.intervals.size() - 1;

      while (firstIdx <= lastIdx) {

         centerIdx = (firstIdx + lastIdx) / 2;

         centerInterval = this.intervals.get(centerIdx);

         if (centerInterval.contains(value)) {
            return true;

         } else if (value.compareTo(centerInterval.getLowerBound()) <= 0) {
            lastIdx = centerIdx - 1;

         } else if (value.compareTo(centerInterval.getUpperBound()) >= 0) {
            firstIdx = centerIdx + 1;
         }

      }
      return false;
   }

   /**
    * Getter for the minimum value of this range set
    * 
    * @return the minValue
    */
   public T getMinValue() {

      if (isEmpty()) {
         return getUndefinedObject();
      } else {
         Interval<T> firstInterval = first();
         return firstInterval.getLowerBound();
      }

   }

   /**
    * Getter for the maximum value of this range set
    * 
    * @return the maxValue
    */
   public T getMaxValue() {
      if (isEmpty()) {
         return getUndefinedObject();
      } else {
         Interval<T> lastInterval = last();
         return lastInterval.getUpperBound();
      }

   }

   /**
    * Getter for the first (lowest) interval in this range set
    * 
    * @return the first interval
    */
   protected Interval<T> first() {
      return intervals.get(0);
   }

   /**
    * Getter for the last (greatest) interval in this range set
    * 
    * @return the last interval
    */
   protected Interval<T> last() {
      return intervals.get(getNoComponents() - 1);
   }

   /**
    * Getter for the number of intervals in this range set
    * 
    * @return number of intervals
    */
   public int getNoComponents() {
      return intervals.size();
   }

   /**
    * Returns the interval at the specified position in this range set
    * 
    * @param index
    *           index of the interval to return
    * @return interval at the specified position
    * @throws IndexOutOfBoundsException
    *            if the index is out of range
    *            (<tt>index &lt; 0 || index &gt;= getNoComponents()</tt>) *
    */
   protected Interval<T> get(final int index) {
      return intervals.get(index);
   }

   /**
    * Check if this range set is empty
    * 
    * @return true if this range set is empty, false otherwise
    */
   public boolean isEmpty() {
      return intervals.isEmpty();
   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#toString()
    */
   @Override
   public String toString() {
      return "Range [intervals=" + intervals + "]";
   }

   /**
    * Returns an undefined object of type {@code <T>}. <br>
    * Must be implemented by the specific subclass.
    * 
    * @return undefined object of type {@code <T>}
    */
   protected abstract T getUndefinedObject();
}
