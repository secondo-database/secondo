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
import java.util.Objects;

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
   protected Range(int size) {
      this.intervals = new ArrayList<>(size);
      this.setDefined(true);
   }

   /**
    * Copy constructor for a 'Range' object <br>
    * Creates an 'Range' object as a copy from a other passed 'Range' object
    * 
    * @param originalRange
    */
   protected Range(final Range<T> originalRange) {
      Objects.requireNonNull(originalRange, "range must not be null");

      this.intervals = new ArrayList<>(originalRange.getNoComponents());
      intervals.addAll(originalRange.getIntervals());

      this.setDefined(originalRange.isDefined());
   }

   /**
    * Append the passed interval object to this range set.<br>
    * The passed interval have to be disjoint and non adjacent to the current range
    * set.
    * 
    * @param interval
    *           - the interval object to append
    * @return true (as specified by {@link Collection#add})
    */
   public boolean add(Interval<T> interval) {

      return intervals.add(interval);
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
   public boolean mergeAdd(Interval<T> interval) {
      if (isEmpty()) {
         return intervals.add(interval);

      } else {

         Interval<T> lastInterval = last();

         if (lastInterval.intersectsRight(interval) || lastInterval.rightAdjacent(interval)) {

            Interval<T> newLastInterval = lastInterval.mergeRight(interval);

            intervals.set(getNoComponents() - 1, newLastInterval);

            return true;

         } else if (lastInterval.before(interval)) {

            return intervals.add(interval);
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
   public boolean intersects(Interval<T> interval) {

      T ivLowerBound = interval.getLowerBound();
      T ivUpperBound = interval.getUpperBound();

      if (ivUpperBound.compareTo(getMinValue()) < 0 || ivLowerBound.compareTo(getMaxValue()) > 0) {
         return false;
      }

      return true;
   }

   /**
    * Check if the passed interval is adjacent to this 'Range' object.
    * 
    * @param interval
    * @return
    */
   public boolean adjacent(Interval<T> interval) {

      return first().leftAdjacent(interval) || last().rightAdjacent(interval);

   }

   /**
    * Getter for the list of intervals
    * 
    * @return the intervals
    */
   private List<Interval<T>> getIntervals() {
      return intervals;
   }

   /**
    * Getter for the minimum value of this range set
    * 
    * @return the minValue
    */
   public T getMinValue() {

      if (isEmpty()) {
         return getUndefinedValue();
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
         return getUndefinedValue();
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
   protected Interval<T> get(int index) {
      return intervals.get(index);
   }

   /**
    * Check if this range set is empty
    * 
    * @return true if this range set is empty, false otherwise
    */
   public boolean isEmpty() {
      return intervals.size() == 0;
   }

   /**
    * Returns an undefined object of type {@code <T>} <br>
    * Must be implemented by the specific subclass
    * 
    * @return undefined object of type {@code <T>}
    */
   protected abstract T getUndefinedValue();
}
