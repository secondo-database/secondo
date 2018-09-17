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

package mol.datatypes.unit;

import java.util.Objects;

import mol.datatypes.GeneralType;
import mol.datatypes.interval.Period;
import mol.datatypes.time.TimeInstant;

/**
 * Abstract base class for all 'UnitObject' subclasses
 * 
 * @param <T>
 *           specifies the type of the 'UnitObject'
 * 
 * @author Markus Fuessel
 * 
 */
public abstract class UnitObject<T extends GeneralType> extends GeneralType implements Comparable<UnitObject<T>> {

   /**
    * Time period for which the current unitobject is valid
    */
   private Period period;

   /**
    * Constructor for an undefined 'UnitObject' object<br>
    * Required for subclasses
    */
   protected UnitObject() {
      this.period = new Period();
   }

   /**
    * Base constructor for a 'UnitObject' object<br>
    * Required for subclasses
    * 
    * @param period
    *           - valid time period for this unit
    */
   protected UnitObject(final Period period) {
      this.period = Objects.requireNonNull(period, "period must not be null");
      setDefined(period.isDefined());
   }

   /**
    * Unit objects where ordered/compared by their period<br>
    * Note: this class has a natural ordering that is inconsistent with equals.
    *
    * @see java.lang.Comparable#compareTo(java.lang.Object)
    * 
    * @param otherUnitObject
    *           - the other unit object to compare
    * @return a negative, zero, or a positive integer as the period of this object
    *         is less than, equal to, or greater than the period of the specified
    *         object.
    */
   @Override
   public int compareTo(final UnitObject<T> otherUnitObject) {

      return period.compareTo(otherUnitObject.getPeriod());
   }

   /**
    * Verify if this 'UnitObject' is completely before the passed one, regarding to
    * their defined time period.
    * 
    * @param otherUnitObject
    * 
    * @return true if the period of this unit is before the other units period,
    *         false otherwise
    */
   public boolean before(final UnitObject<?> otherUnitObject) {
      return period.before(otherUnitObject.getPeriod());
   }

   /**
    * Verify if the defined time period of this 'UnitObject' ends before the time
    * period of the passed unit
    * 
    * @param otherUnitObject
    * 
    * @return true if the period of this unit ends before the other units periods
    *         ends, false otherwise
    */
   public boolean periodEndsWithin(final UnitObject<?> otherUnitObject) {

      TimeInstant upperBound = period.getUpperBound();

      TimeInstant otherLowerBound = otherUnitObject.getPeriod().getLowerBound();
      TimeInstant otherUpperBound = otherUnitObject.getPeriod().getUpperBound();

      return upperBound.compareTo(otherLowerBound) >= 0 && upperBound.compareTo(otherUpperBound) < 0;
   }

   /**
    * Getter for the period of this 'UnitObject'
    * 
    * @return the period
    */
   public Period getPeriod() {
      return period;
   }

   /**
    * Set the period of this 'UnitObject'
    * 
    * @param period
    *           the period to set
    */
   public void setPeriod(Period period) {
      this.period = period;
   }

   /**
    * This method returns a object of type {@code T} which is valid at the given
    * time instant, if this time instant lies inside the defined period of this
    * unitobject
    * <p>
    * otherwise the returned object is undefined
    * 
    * @param instant
    *           the time instant
    * 
    * @return object of type {@code T}
    */
   public abstract T getValue(final TimeInstant instant);

   /**
    * This method reduces this 'UnitObject' by the passed time period. <br>
    * The passed time period musst intersect with the period of this unit object,
    * otherwise the returned object is undefined
    * 
    * @param period
    *           the time period
    * 
    * @return object of type {@code UnitObject<T>}
    */
   public abstract UnitObject<T> atPeriod(final Period period);

   /**
    * Verifies if the period of this unit contains the passed TimeInstant
    * 
    * @param instant
    * 
    * @return true if the passed instant lies within the period of this unit, false
    *         otherwise
    */
   public boolean contains(final TimeInstant instant) {
      return period.contains(instant);
   }

   /**
    * Get the initial value of type {@code T} of this unit at begin of the unit
    * period
    * 
    * @return the initial {@code T} value
    */
   public abstract T getInitial();

   /**
    * Get the final value of type {@code T} of this unit at end of the unit period
    * 
    * @return the final {@code T} value
    */
   public abstract T getFinal();

   /**
    * Verify if the entire value of this {@code 'UnitObject<T>'} is equal to the
    * entire value of the passed {@code 'UnitObject<T>'}.
    * 
    * @param otherUnitObject
    * 
    * @return true, the values of this and the passed 'UnitObject' are equal, false
    *         otherwise
    */
   public abstract boolean equalValue(final UnitObject<T> otherUnitObject);

   /**
    * Verify if the final value of this {@code 'UnitObject<T>'} is equal to the
    * initial value of the passed {@code 'UnitObject<T>'}.
    * 
    * @param otherUnitObject
    * @return true if this final value is equal to initial value of other unit,
    *         false otherwise
    */
   public abstract boolean finalEqualToInitialValue(final UnitObject<T> otherUnitObject);
}
