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
package stlib.interfaces.interval;

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.features.Orderable;

/**
 * Interface that should be provided by objects of type IntervalIF
 * 
 * @param <T>
 *           - specifies the type of the interval
 * 
 * @author Markus Fuessel
 */
public interface IntervalIF<T extends GeneralTypeIF & Orderable<T>> extends GeneralTypeIF, Comparable<IntervalIF<T>> {

   /**
    * Verifies if the interval contains the passed value
    * 
    * @param value
    * 
    * @return true if the interval contains the passed value, false otherwise
    */
   boolean contains(T value);

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
   boolean contains(T value, boolean ignoreClosedFlags);

   /**
    * Check if two intervals are adjacent
    * 
    * @param otherInterval
    * @return true - intervals are adjacent, false - otherwise
    */
   // @Override
   boolean adjacent(IntervalIF<T> otherInterval);

   /**
    * Check if this interval is adjacent with its left boundary (lower bound) to
    * the passed interval
    * 
    * @param otherInterval
    * @return true - interval is left adjacent, false - otherwise
    */
   boolean leftAdjacent(IntervalIF<T> otherInterval);

   /**
    * Check if this interval is adjacent with its right boundary (upper bound) to
    * the passed interval
    * 
    * @param otherInterval
    * @return true - interval is right adjacent, false - otherwise
    */
   boolean rightAdjacent(IntervalIF<T> otherInterval);

   /**
    * Verify if this 'Interval' object is complete before the passed 'Interval'
    * object
    * 
    * @param otherInterval
    * @return true if this object is before the passed one, false otherwise
    */
   boolean before(IntervalIF<T> otherInterval);

   /**
    * Verify if this 'Interval' object is before the passed value
    * 
    * @param value
    * @return true if this object is before the passed value, false otherwise
    */
   boolean before(T value);

   /**
    * Verify if this 'Interval' object is after the passed 'Interval' object
    * 
    * @param otherInterval
    * @return true if this object is after the passed one, false otherwise
    */
   boolean after(IntervalIF<T> otherInterval);

   /**
    * Verify if this 'Interval' object is after the passed value
    * 
    * @param value
    * @return true if this object is after the passed value, false otherwise
    */
   boolean after(T value);

   /**
    * This method checks if this interval and the passed interval are disjoint
    * 
    * @param otherInterval
    * @return true - intervals are disjoint, false otherwise
    */
   boolean disjoint(IntervalIF<T> otherInterval);

   /**
    * Check if the passed interval intersects this interval
    * 
    * @param otherInterval
    * @return true if the passed interval intersects with this interval, false
    *         otherwise
    */
   boolean intersects(IntervalIF<T> otherInterval);

   /**
    * If this interval intersects with its lower bound (left side) by the passed
    * one
    * 
    * @param otherInterval
    * @return true if this interval is intersected left, false otherwise
    */
   boolean intersectsLeft(IntervalIF<T> otherInterval);

   /**
    * If this interval intersects with its upper bound (right side) by the passed
    * one
    * 
    * @param otherInterval
    * @return true if this interval is intersected right, false otherwise
    */
   boolean intersectsRight(IntervalIF<T> otherInterval);

   /**
    * Check if this interval intersects exactly on the right bound (upper bound)
    * with the passed interval.
    * 
    * @param otherInterval
    * 
    * @return true if this interval intersects exactly on the right bound (upper
    *         bound) with the passed interval, false otherwise
    */
   boolean intersectsOnRightBound(IntervalIF<T> otherInterval);

   /**
    * Creates a new interval object where this interval is merged with the passed
    * one
    * 
    * @param otherInterval
    *           - the other interval to merge with
    * @return new interval object
    */
   IntervalIF<T> merge(IntervalIF<T> otherInterval);

   /**
    * Creates a new interval object where this interval is merged on the left with
    * the passed interval
    * 
    * @param otherInterval
    * @return new interval object
    */
   IntervalIF<T> mergeLeft(IntervalIF<T> otherInterval);

   /**
    * Creates a new interval object where this interval is merged on the right with
    * the passed interval
    * 
    * @param otherInterval
    * @return new interval object
    */
   IntervalIF<T> mergeRight(IntervalIF<T> otherInterval);

   /**
    * Creates the intersection of this and the passed interval as a new interval
    * object
    * 
    * @param otherInterval
    * @return new interval object
    */
   IntervalIF<T> intersection(IntervalIF<T> otherInterval);

   /**
    * Returns a copy of this {@code Interval<T>} object
    * 
    * @return {@code Interval<T>} object copy
    */
   IntervalIF<T> copy();

   /**
    * Get lower bound of the interval
    * 
    * @return the lowerBound
    */
   T getLowerBound();

   /**
    * Get the upper bound of the interval
    * 
    * @return the upperBound
    */
   T getUpperBound();

   /**
    * Is interval left closed
    * 
    * @return the leftClosed
    */
   boolean isLeftClosed();

   /**
    * Is interval right closed
    * 
    * @return the rightClosed
    */
   boolean isRightClosed();

   /**
    * Set the lower bound
    * 
    * @param lowerBound
    */
   public void setLowerBound(T lowerBound);

   /**
    * Set the upper bound
    * 
    * @param upperBound
    */
   public void setUpperBound(T upperBound);

   /**
    * Set if interval is left closed
    * 
    * @param leftClosed
    */
   void setLeftClosed(boolean leftClosed);

   /**
    * Set if interval is right closed
    * 
    * @param rightClosed
    */
   void setRightClosed(boolean rightClosed);

   /**
    * Verify if this 'Interval' object is valid<br>
    * 
    * - lowerBound and upperBound are defined<br>
    * - lowerBound <= upperBound <br>
    * - is closed if lowerBound == upperBound <br>
    * 
    * @return true if this 'Interval' object is valid, false otherwise
    */
   boolean isValid();

}