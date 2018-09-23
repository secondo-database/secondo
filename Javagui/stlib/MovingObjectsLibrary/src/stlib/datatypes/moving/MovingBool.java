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
package stlib.datatypes.moving;

import stlib.datatypes.base.BaseBool;
import stlib.datatypes.unit.UnitBool;
import stlib.interfaces.base.BaseBoolIF;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.moving.MovingBoolIF;
import stlib.interfaces.unit.UnitBoolIF;

/**
 * This class represents moving boolean objects of type 'MovingBool'
 * 
 * @author Markus Fuessel
 */
public class MovingBool extends MovingObject<UnitBoolIF, BaseBoolIF> implements MovingBoolIF {

   /**
    * The minimum value that this object reaches during its defined period of time
    */
   private BaseBoolIF minValue;

   /**
    * The maximum value that this object reaches during its defined period of time
    */
   private BaseBoolIF maxValue;

   /**
    * Basic constructor to create a empty 'MovingBool' object
    * 
    * @param initialSize
    *           - used to initialize the internal used array structures
    */
   public MovingBool(final int initialSize) {
      super(initialSize);

      minValue = new BaseBool();
      maxValue = new BaseBool();

      this.setDefined(true);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.datatypes.moving.MovingObject#add(stlib.interfaces.unit.UnitObjectIF)
    */
   @Override
   public boolean add(UnitBoolIF newUnit) {
      if (super.add(newUnit)) {

         setMinMaxValue(newUnit);

         return true;
      }
      return false;
   }

   /**
    * Set the minimum and maximum value if the current value exceeds these limits
    * 
    * @param newUnit
    */
   private void setMinMaxValue(UnitBoolIF newUnit) {
      BaseBoolIF unitValue = newUnit.getValue();

      if (getNoUnits() == 1) {
         minValue = unitValue;
         maxValue = unitValue;
      } else {
         if (minValue.compareTo(unitValue) > 0) {
            minValue = unitValue;
         }

         if (maxValue.compareTo(unitValue) < 0) {
            maxValue = unitValue;
         }
      }
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.moving.MovingBoolIF#add(stlib.interfaces.interval.PeriodIF,
    * boolean)
    */
   @Override
   public boolean add(final PeriodIF period, final boolean booleanValue) {

      return add(new UnitBool(period, booleanValue));
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * stlib.interfaces.moving.MovingBoolIF#add(stlib.interfaces.interval.PeriodIF,
    * stlib.interfaces.base.BaseBoolIF)
    */
   @Override
   public boolean add(final PeriodIF period, final BaseBoolIF baseBoolValue) {

      return add(new UnitBool(period, baseBoolValue));
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingBoolIF#add(stlib.interfaces.moving.
    * MovingBoolIF)
    */
   @Override
   public boolean add(final MovingBoolIF otherMovingBool) {
      if (!isDefined() || !otherMovingBool.isDefined()) {
         return false;
      }

      if (getNoUnits() == 0 || getPeriods().before(otherMovingBool.getPeriods().first())) {

         for (int i = 0; i < otherMovingBool.getNoUnits(); i++) {
            add(otherMovingBool.getUnit(i));
         }

         return true;
      }

      return false;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitBoolIF getUndefinedUnitObject() {
      return new UnitBool();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected BaseBoolIF getUndefinedObject() {
      return new BaseBool();
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingBoolIF#getMinValue()
    */
   @Override
   public BaseBoolIF getMinValue() {
      return minValue;
   }

   /*
    * (non-Javadoc)
    * 
    * @see stlib.interfaces.moving.MovingBoolIF#getMaxValue()
    */
   @Override
   public BaseBoolIF getMaxValue() {
      return maxValue;
   }

}
