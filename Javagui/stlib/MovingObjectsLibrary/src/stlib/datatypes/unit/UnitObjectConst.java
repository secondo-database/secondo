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

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.time.TimeInstantIF;
import stlib.interfaces.unit.UnitObjectConstIF;
import stlib.interfaces.unit.UnitObjectIF;

/**
 * This abstract class represents 'UnitObjectConst{@code <T>}' objects and
 * serves as a base class for unit subclasses that uses a constant value for the
 * duration of the unit period
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 */
public abstract class UnitObjectConst<T extends GeneralTypeIF> extends UnitObject<T> implements UnitObjectConstIF<T> {

   /**
    * Constant value for a constant unitobject
    */
   protected final T constValue;

   /**
    * Constructor for an undefined 'UnitObjectConst' object<br>
    * Required for subclasses
    */
   protected UnitObjectConst(final T constValue) {
      this.constValue = constValue;
   }

   /**
    * Base constructor for a 'UnitObjectConst' object
    * 
    * @param period
    *           - valid time period for this unit
    * @param constValue
    *           - the constant value for this unit
    * 
    */
   public UnitObjectConst(final PeriodIF period, final T constValue) {
      super(period);
      this.constValue = Objects.requireNonNull(constValue, "'constValue' must not be null");

      setDefined(period.isDefined() && constValue.isDefined());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getValue(java.time.Instant)
    */
   @Override
   public T getValue(final TimeInstantIF instant) {

      PeriodIF period = getPeriod();

      if (isDefined() && period.contains(instant)) {
         return getValue();
      } else {
         return getUndefinedObject();
      }

   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#equalValue(stlib.interfaces.unit.
    * UnitObjectIF)
    */
   @Override
   public boolean equalValue(UnitObjectIF<T> otherUnitObject) {

      if (!(otherUnitObject instanceof UnitObjectConst<?>) || !otherUnitObject.isDefined()) {
         return false;
      }

      UnitObjectConstIF<?> otherConstUnit = (UnitObjectConstIF<?>) otherUnitObject;

      return constValue.equals(otherConstUnit.getValue());
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.unit.UnitObjectIF#finalEqualToInitialValue(stlib.interfaces.
    * unit. UnitObjectIF)
    */
   @Override
   public boolean finalEqualToInitialValue(UnitObjectIF<T> otherUnitObject) {
      return constValue.equals(otherUnitObject.getInitial());
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectConstIF#getValue()
    */
   @Override
   public T getValue() {
      return constValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getInitial()
    */
   @Override
   public T getInitial() {
      return getValue();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.unit.UnitObjectIF#getFinal()
    */
   @Override
   public T getFinal() {
      return getValue();
   }

   /**
    * Returns an undefined object of type {@code <T>} <br>
    * Must be implemented by the specific subclass
    * 
    * @return undefined object of type {@code <T>}
    */
   protected abstract T getUndefinedObject();

}
