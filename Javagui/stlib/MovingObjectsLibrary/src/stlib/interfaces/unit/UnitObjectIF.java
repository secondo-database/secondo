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
package stlib.interfaces.unit;

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.time.TimeInstantIF;

/**
 * Interface that should be provided by objects of type UnitObjectIF
 * 
 * @param <T>
 *           - specifies the type of the 'UnitObject'
 * 
 * @author Markus Fuessel
 */
public interface UnitObjectIF<T extends GeneralTypeIF> extends GeneralTypeIF, Comparable<UnitObjectIF<T>> {

   /**
    * Verify if this 'UnitObject' is completely before the passed one, regarding to
    * their defined time period.
    * 
    * @param otherUnitObject
    * 
    * @return true if the period of this unit is before the other units period,
    *         false otherwise
    */
   boolean before(UnitObjectIF<?> otherUnitObject);

   /**
    * Verify if the defined time period of this 'UnitObject' ends before the time
    * period of the passed unit
    * 
    * @param otherUnitObject
    * 
    * @return true if the period of this unit ends before the other units periods
    *         ends, false otherwise
    */
   boolean periodEndsWithin(UnitObjectIF<?> otherUnitObject);

   /**
    * Getter for the period of this 'UnitObject'
    * 
    * @return the period
    */
   PeriodIF getPeriod();

   /**
    * Set the period of this 'UnitObject'
    * 
    * @param period
    *           the period to set
    */
   void setPeriod(PeriodIF period);

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
   T getValue(TimeInstantIF instant);

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
   UnitObjectIF<T> atPeriod(PeriodIF period);

   /**
    * Verifies if the period of this unit contains the passed TimeInstant
    * 
    * @param instant
    * 
    * @return true if the passed instant lies within the period of this unit, false
    *         otherwise
    */
   boolean contains(TimeInstantIF instant);

   /**
    * Get the initial value of type {@code T} of this unit at begin of the unit
    * period
    * 
    * @return the initial {@code T} value
    */
   T getInitial();

   /**
    * Get the final value of type {@code T} of this unit at end of the unit period
    * 
    * @return the final {@code T} value
    */
   T getFinal();

   /**
    * Verify if the entire value of this {@code 'UnitObject<T>'} is equal to the
    * entire value of the passed {@code 'UnitObject<T>'}.
    * 
    * @param otherUnitObject
    * 
    * @return true, the values of this and the passed 'UnitObject' are equal, false
    *         otherwise
    */
   boolean equalValue(UnitObjectIF<T> otherUnitObject);

   /**
    * Verify if the final value of this {@code 'UnitObjectIF<T>'} is equal to the
    * initial value of the passed {@code 'UnitObjectIF<T>'}.
    * 
    * @param otherUnitObject
    * @return true if this final value is equal to initial value of other unit,
    *         false otherwise
    */
   boolean finalEqualToInitialValue(UnitObjectIF<T> otherUnitObject);

}