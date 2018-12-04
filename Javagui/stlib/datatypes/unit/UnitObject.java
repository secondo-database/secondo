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

package stlib.datatypes.unit;

import java.util.Objects;

import stlib.datatypes.GeneralType;
import stlib.datatypes.interval.Period;
import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectIF;

/**
 * Abstract base class for all 'UnitObject' subclasses
 * 
 * @param <T>
 *           specifies the type of the 'UnitObject'
 * 
 * @author Markus Fuessel
 * 
 */
public abstract class UnitObject<T extends GeneralTypeIF> extends GeneralType implements UnitObjectIF<T> {

   /**
    * Time period for which the current unitobject is valid
    */
   private PeriodIF period;

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
   protected UnitObject(final PeriodIF period) {
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
   public int compareTo(final UnitObjectIF<T> otherUnitObject) {

      return period.compareTo(otherUnitObject.getPeriod());
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#before(stlib.interfaces.unit.UnitObjectIF)
    */
   @Override
   public boolean before(final UnitObjectIF<?> otherUnitObject) {
      return period.before(otherUnitObject.getPeriod());
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#periodEndsWithin(stlib.interfaces.unit.
    * UnitObjectIF)
    */
   @Override
   public boolean periodEndsWithin(final UnitObjectIF<?> otherUnitObject) {

      TimeInstantIF upperBound = period.getUpperBound();

      TimeInstantIF otherLowerBound = otherUnitObject.getPeriod().getLowerBound();
      TimeInstantIF otherUpperBound = otherUnitObject.getPeriod().getUpperBound();

      return upperBound.compareTo(otherLowerBound) >= 0 && upperBound.compareTo(otherUpperBound) < 0;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getPeriod()
    */
   @Override
   public PeriodIF getPeriod() {
      return period;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#setPeriod(stlib.interfaces.interval.
    * PeriodIF)
    */
   @Override
   public void setPeriod(PeriodIF period) {
      this.period = period;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#contains(stlib.interfaces.time.
    * TimeInstantIF)
    */
   @Override
   public boolean contains(final TimeInstantIF instant) {
      return period.contains(instant);
   }

}
