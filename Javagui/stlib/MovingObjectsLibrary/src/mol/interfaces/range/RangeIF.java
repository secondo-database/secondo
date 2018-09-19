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
package mol.interfaces.range;

import java.util.Collection;

import mol.interfaces.GeneralTypeIF;
import mol.interfaces.features.Orderable;
import mol.interfaces.interval.IntervalIF;

/**
 * Interface that should be provided by objects of type RangeIF
 * 
 * @param <T>
 *           - specifies the type of the interval
 * 
 * @author Markus Fuessel
 */
public interface RangeIF<T extends GeneralTypeIF & Orderable<T>> extends GeneralTypeIF {

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
   boolean add(IntervalIF<T> interval);

   /**
    * Appends the passed interval to this 'RangeIF' object. If the passed interval
    * intersects the last interval of this range set or is adjacent to it, the last
    * interval and the passed one would be merged to a new last interval of this
    * range set
    * 
    * @param interval
    *           - the interval object to append
    * @return true if adding of the passed interval was successful, false otherwise
    */
   boolean mergeAdd(IntervalIF<T> interval);

   /**
    * Check if the passed interval intersects this range set
    * 
    * @param interval
    * @return true if the passed interval intersects at least with one interval of
    *         the range set
    */
   boolean intersects(IntervalIF<T> interval);

   /**
    * Check if the passed 'RangeIF' intersects this range set
    * 
    * @param other
    * @return true if the passed 'RangeIF' intersects at least with one interval of
    *         this range set
    */
   boolean intersects(RangeIF<T> other);

   /**
    * Verify if the passed interval is adjacent to this 'RangeIF' object.
    * 
    * @param interval
    * 
    * @return true if the passed interval is left or right adjacent, false
    *         otherwise
    */
   boolean adjacent(IntervalIF<T> interval);

   /**
    * Verify if this 'RangeIF' object is adjacent on the right to the passed
    * interval
    * 
    * @param interval
    * @return true if the passed interval is adjacent on the right, false otherwise
    */
   boolean rightAdjacent(IntervalIF<T> interval);

   /**
    * Verify if this 'Range' object is adjacent on the left to the passed interval
    * 
    * @param interval
    * 
    * @return true if the passed interval is adjacent on the left, false otherwise
    */
   boolean leftAdjacent(IntervalIF<T> interval);

   /**
    * Verify if this {@code 'RangeIF<T>'} object is before the passed
    * {@code 'Interval<T>'} object
    * 
    * @param interval
    * @return true if this object is before the passed one, false otherwise
    */
   boolean before(IntervalIF<T> interval);

   /**
    * Verify if this {@code 'RangeIF<T>'} object is after the passed
    * {@code 'Interval<T>'} object
    * 
    * @param interval
    * @return true if this object is after the passed one, false otherwise
    */
   boolean after(IntervalIF<T> interval);

   /**
    * Check whether the given value is covered by this range set.
    * 
    * @param value
    * 
    * @return true if value is covered by this range set, false otherwise
    */
   boolean contains(T value);

   /**
    * Getter for the minimum value of this range set
    * 
    * @return the minValue
    */
   T getMinValue();

   /**
    * Getter for the maximum value of this range set
    * 
    * @return the maxValue
    */
   T getMaxValue();

   /**
    * Getter for the number of intervals in this range set
    * 
    * @return number of intervals
    */
   int getNoComponents();

   /**
    * Returns the interval at the specified position in this range set
    * 
    * @param index
    *           index of the interval to return
    * 
    * @return interval at the specified position
    */
   public IntervalIF<T> get(final int index);

   /**
    * Check if this range set is empty
    * 
    * @return true if this range set is empty, false otherwise
    */
   boolean isEmpty();

}