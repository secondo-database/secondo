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
import mol.interfaces.GeneralTypeIF;
import mol.interfaces.features.Orderable;
import mol.interfaces.interval.IntervalIF;

/**
 * Abstract Superclass for all interval subclasses
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 *           - specifies the type of the interval
 */
public abstract class Interval<T extends GeneralTypeIF & Orderable<T>> extends GeneralType implements IntervalIF<T> {

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

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#contains(T)
    */
   @Override
   public boolean contains(final T value) {

      return contains(value, false);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#contains(T, boolean)
    */
   @Override
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
   public int compareTo(final IntervalIF<T> otherInterval) {

      int compare = lowerBound.compareTo(otherInterval.getLowerBound());

      if (compare != 0) {
         return compare;
      }

      compare = Boolean.compare(otherInterval.isLeftClosed(), leftClosed);

      if (compare != 0) {
         return compare;
      }

      compare = upperBound.compareTo(otherInterval.getUpperBound());

      if (compare != 0) {
         return compare;
      }

      compare = Boolean.compare(rightClosed, otherInterval.isRightClosed());

      return compare;

   }

   /*
    * (non-Javadoc)
    * 
    * @see java.lang.Object#equals(java.lang.Object)
    */
   @Override
   public boolean equals(final Object obj) {

      if (!(obj instanceof IntervalIF<?>)) {
         return false;
      }

      if (!(this.getClass().equals(obj.getClass()))) {
         return false;
      }

      @SuppressWarnings("unchecked")
      IntervalIF<T> otherInterval = (IntervalIF<T>) obj;

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

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#adjacent(mol.datatypes.interval.Interval)
    */
   // @Override
   @Override
   public boolean adjacent(final IntervalIF<T> otherInterval) {

      return leftAdjacent(otherInterval) || rightAdjacent(otherInterval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#leftAdjacent(mol.datatypes.interval.
    * Interval)
    */
   @Override
   public boolean leftAdjacent(final IntervalIF<T> otherInterval) {
      int compare = lowerBound.compareTo(otherInterval.getUpperBound());

      if (compare == 0 && (leftClosed != otherInterval.isRightClosed())) {
         return true;
      }

      return (((compare > 0 && leftClosed && otherInterval.isRightClosed())
            || (compare < 0 && !leftClosed && !otherInterval.isRightClosed()))
            && lowerBound.adjacent(otherInterval.getUpperBound()));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#rightAdjacent(mol.datatypes.interval.
    * Interval)
    */
   @Override
   public boolean rightAdjacent(final IntervalIF<T> otherInterval) {
      int compare = upperBound.compareTo(otherInterval.getLowerBound());

      if (compare == 0 && (rightClosed != otherInterval.isLeftClosed())) {

         return true;
      }

      return (((compare < 0 && rightClosed && otherInterval.isLeftClosed())
            || (compare > 0 && !rightClosed && !otherInterval.isLeftClosed()))
            && upperBound.adjacent(otherInterval.getLowerBound()));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#before(mol.datatypes.interval.Interval)
    */
   @Override
   public boolean before(final IntervalIF<T> otherInterval) {

      int compare = upperBound.compareTo(otherInterval.getLowerBound());

      return (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.isLeftClosed())));

   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#before(T)
    */
   @Override
   public boolean before(final T value) {

      int compare = upperBound.compareTo(value);

      return (compare < 0 || (compare == 0 && !rightClosed));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#after(mol.datatypes.interval.Interval)
    */
   @Override
   public boolean after(final IntervalIF<T> otherInterval) {
      int compare = lowerBound.compareTo(otherInterval.getUpperBound());

      return (compare > 0 || (compare == 0 && (!leftClosed || !otherInterval.isRightClosed())));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#after(T)
    */
   @Override
   public boolean after(final T value) {

      int compare = lowerBound.compareTo(value);

      return (compare > 0 || (compare == 0 && !leftClosed));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#disjoint(mol.datatypes.interval.Interval)
    */
   @Override
   public boolean disjoint(final IntervalIF<T> otherInterval) {

      if (upperBound.compareTo(otherInterval.getLowerBound()) < 0
            || lowerBound.compareTo(otherInterval.getUpperBound()) > 0) {
         return true;
      }

      return ((upperBound.compareTo(otherInterval.getLowerBound()) == 0
            && !(rightClosed && otherInterval.isLeftClosed()))
            || (lowerBound.compareTo(otherInterval.getUpperBound()) == 0
                  && !(leftClosed && otherInterval.isRightClosed())));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#intersects(mol.datatypes.interval.Interval)
    */
   @Override
   public boolean intersects(final IntervalIF<T> otherInterval) {

      int compare = upperBound.compareTo(otherInterval.getLowerBound());

      if (compare < 0 || (compare == 0 && !(rightClosed && otherInterval.isLeftClosed()))) {
         return false;
      }

      compare = lowerBound.compareTo(otherInterval.getUpperBound());

      return !(compare > 0 || (compare == 0 && !(leftClosed && otherInterval.isRightClosed())));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#intersectsLeft(mol.datatypes.interval.
    * IntervalIF)
    */
   @Override
   public boolean intersectsLeft(final IntervalIF<T> otherInterval) {

      return otherInterval.intersectsRight(this);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#intersectsRight(mol.datatypes.interval.
    * Interval)
    */
   @Override
   public boolean intersectsRight(final IntervalIF<T> otherInterval) {

      int compare = lowerBound.compareTo(otherInterval.getLowerBound());

      if (compare > 0 || (compare == 0 && (!leftClosed && otherInterval.isLeftClosed()))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.getLowerBound());

      if (compare < 0 || (compare == 0 && (!rightClosed || !otherInterval.isLeftClosed()))) {
         return false;
      }

      compare = upperBound.compareTo(otherInterval.getUpperBound());

      return !(compare > 0 || (compare == 0 && (rightClosed && !otherInterval.isRightClosed())));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#intersectsOnRightBound(mol.datatypes.
    * interval.Interval)
    */
   @Override
   public boolean intersectsOnRightBound(final IntervalIF<T> otherInterval) {
      int compare = upperBound.compareTo(otherInterval.getLowerBound());

      return compare == 0 && rightClosed && otherInterval.isLeftClosed();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#merge(mol.datatypes.interval.Interval)
    */
   @Override
   public IntervalIF<T> merge(final IntervalIF<T> otherInterval) {

      return mergeLeft(otherInterval).mergeRight(otherInterval);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#mergeLeft(mol.datatypes.interval.Interval)
    */
   @Override
   public IntervalIF<T> mergeLeft(final IntervalIF<T> otherInterval) {
      IntervalIF<T> mergedInterval = this.copy();

      int compare = mergedInterval.getLowerBound().compareTo(otherInterval.getLowerBound());

      if (compare == 0) {
         mergedInterval.setLeftClosed(leftClosed = mergedInterval.isLeftClosed() || otherInterval.isLeftClosed());
      } else if (compare > 0) {
         mergedInterval.setLowerBound(otherInterval.getLowerBound());
         mergedInterval.setLeftClosed(otherInterval.isLeftClosed());
      }

      return mergedInterval;
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.interval.IntervalIF#mergeRight(mol.datatypes.interval.Interval)
    */
   @Override
   public IntervalIF<T> mergeRight(final IntervalIF<T> otherInterval) {
      IntervalIF<T> mergedInterval = this.copy();

      int compare = mergedInterval.getUpperBound().compareTo(otherInterval.getUpperBound());

      if (compare == 0) {
         mergedInterval.setRightClosed(mergedInterval.isRightClosed() || otherInterval.isRightClosed());
      } else if (compare < 0) {
         mergedInterval.setUpperBound(otherInterval.getUpperBound());
         mergedInterval.setRightClosed(otherInterval.isRightClosed());
      }

      return mergedInterval;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#intersection(mol.datatypes.interval.
    * Interval)
    */
   @Override
   public IntervalIF<T> intersection(final IntervalIF<T> otherInterval) {

      IntervalIF<T> intersectionInterval = this.copy();

      int compare = intersectionInterval.getLowerBound().compareTo(otherInterval.getLowerBound());

      if (compare == 0) {
         intersectionInterval.setLeftClosed(intersectionInterval.isLeftClosed() && otherInterval.isLeftClosed());
      } else if (compare < 0) {
         intersectionInterval.setLowerBound(otherInterval.getLowerBound());
         intersectionInterval.setLeftClosed(otherInterval.isLeftClosed());
      }

      compare = intersectionInterval.getUpperBound().compareTo(otherInterval.getUpperBound());

      if (compare == 0) {
         intersectionInterval.setRightClosed(intersectionInterval.isRightClosed() && otherInterval.isRightClosed());
      } else if (compare > 0) {
         intersectionInterval.setUpperBound(otherInterval.getUpperBound());
         intersectionInterval.setRightClosed(otherInterval.isRightClosed());
      }

      intersectionInterval.setDefined(intersectionInterval.isValid());

      return intersectionInterval;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#getLowerBound()
    */
   @Override
   public T getLowerBound() {
      return lowerBound;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#getUpperBound()
    */
   @Override
   public T getUpperBound() {
      return upperBound;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#isLeftClosed()
    */
   @Override
   public boolean isLeftClosed() {
      return leftClosed;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#isRightClosed()
    */
   @Override
   public boolean isRightClosed() {
      return rightClosed;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#setLowerBound()
    */
   @Override
   public void setLowerBound(T lowerBound) {
      this.lowerBound = lowerBound;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#setUpperBound()
    */
   @Override
   public void setUpperBound(T upperBound) {
      this.upperBound = upperBound;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#setLeftClosed(boolean)
    */
   @Override
   public void setLeftClosed(boolean leftClosed) {
      this.leftClosed = leftClosed;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#setRightClosed(boolean)
    */
   @Override
   public void setRightClosed(boolean rightClosed) {
      this.rightClosed = rightClosed;
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.interval.IntervalIF#isValid()
    */
   @Override
   public boolean isValid() {

      return lowerBound.isDefined() && upperBound.isDefined() && (lowerBound.compareTo(upperBound) < 0
            || (lowerBound.compareTo(upperBound) == 0 && leftClosed && rightClosed));
   }

}
