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

package stlib.datatypes.range;

import java.util.ArrayList;
import java.util.List;

import stlib.datatypes.GeneralType;
import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.features.Orderable;
import stlib.interfaces.interval.IntervalIF;
import stlib.interfaces.range.RangeIF;

/**
 * Base class for 'Range' objects.
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 */
public abstract class Range<T extends GeneralTypeIF & Orderable<T>> extends GeneralType implements RangeIF<T> {

   /**
    * Set of {@code 'IntervalIF<T>'} objects
    */
   private final List<IntervalIF<T>> intervals;

   /**
    * Simple constructor to create an empty 'RangeIF' object with the specified
    * initial capacity
    * 
    * @param size
    *           - initial capacity of this 'RangeIF' object
    */
   protected Range(final int size) {
      this.intervals = new ArrayList<>(size);
      this.setDefined(true);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#add(stlib.interfaces.interval.IntervalIF)
    */
   @Override
   public boolean add(final IntervalIF<T> interval) {
      if (interval.isDefined()) {
         return intervals.add(interval);
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.range.RangeIF#mergeAdd(stlib.interfaces.interval.IntervalIF)
    */
   @Override
   public boolean mergeAdd(final IntervalIF<T> interval) {
      if (isEmpty()) {
         return add(interval);

      } else {

         IntervalIF<T> lastInterval = last();

         if (lastInterval.intersectsRight(interval) || lastInterval.rightAdjacent(interval)) {

            IntervalIF<T> newLastInterval = lastInterval.mergeRight(interval);

            intervals.set(getNoComponents() - 1, newLastInterval);

            return true;

         } else if (lastInterval.before(interval)) {

            return add(interval);
         }
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#intersects(stlib.interfaces.interval.
    * IntervalIF)
    */
   @Override
   public boolean intersects(final IntervalIF<T> interval) {

      if (!this.isDefined() || !interval.isDefined()) {
         return false;
      }

      IntervalIF<T> centerIV;
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

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#intersects(stlib.datatypes.range.RangeIF)
    */
   @Override
   public boolean intersects(final RangeIF<T> other) {

      if (!this.isDefined() || !other.isDefined()) {
         return false;
      }

      for (int i = 0; i < other.getNoComponents(); i++) {
         IntervalIF<T> currentOtherInterval = other.get(i);

         if (this.intersects(currentOtherInterval)) {
            return true;
         }
      }

      return false;

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.range.RangeIF#adjacent(stlib.interfaces.interval.IntervalIF)
    */
   @Override
   public boolean adjacent(final IntervalIF<T> interval) {

      return leftAdjacent(interval) || rightAdjacent(interval);

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#rightAdjacent(stlib.interfaces.interval.
    * IntervalIF)
    */
   @Override
   public boolean rightAdjacent(final IntervalIF<T> interval) {
      return last().rightAdjacent(interval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#leftAdjacent(stlib.interfaces.interval.
    * IntervalIF)
    */
   @Override
   public boolean leftAdjacent(final IntervalIF<T> interval) {
      return first().leftAdjacent(interval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.range.RangeIF#before(stlib.interfaces.interval.IntervalIF)
    */
   @Override
   public boolean before(final IntervalIF<T> interval) {

      return last().before(interval);

   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.range.RangeIF#after(stlib.interfaces.interval.IntervalIF)
    */
   @Override
   public boolean after(final IntervalIF<T> interval) {
      return first().after(interval);

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#contains(T)
    */
   @Override
   public boolean contains(final T value) {

      IntervalIF<T> centerInterval;
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

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#getMinValue()
    */
   @Override
   public T getMinValue() {

      if (isEmpty()) {
         return getUndefinedObject();
      } else {
         IntervalIF<T> firstInterval = first();
         return firstInterval.getLowerBound();
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#getMaxValue()
    */
   @Override
   public T getMaxValue() {
      if (isEmpty()) {
         return getUndefinedObject();
      } else {
         IntervalIF<T> lastInterval = last();
         return lastInterval.getUpperBound();
      }

   }

   /**
    * Getter for the first (lowest) interval in this range set
    * 
    * @return the first interval
    */
   protected IntervalIF<T> first() {
      return intervals.get(0);
   }

   /**
    * Getter for the last (greatest) interval in this range set
    * 
    * @return the last interval
    */
   protected IntervalIF<T> last() {
      return intervals.get(getNoComponents() - 1);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#getNoComponents()
    */
   @Override
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
   public IntervalIF<T> get(final int index) {
      return intervals.get(index);
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.range.RangeIF#isEmpty()
    */
   @Override
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
