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
package mol.datatypes.moving;

import mol.datatypes.base.BaseBool;
import mol.datatypes.interval.Period;
import mol.datatypes.unit.UnitBool;

/**
 * This class represents moving boolean objects of type 'MovingBool'
 * 
 * @author Markus Fuessel
 */
public class MovingBool extends MovingObject<UnitBool, BaseBool> {

   /**
    * Basic constructor to create a empty 'MovingBool' object
    * 
    * @param initialSize
    *           - used to initialize the internal used array structures
    */
   public MovingBool(final int initialSize) {
      super(initialSize);

      this.setDefined(true);
   }

   /**
    * Append this 'MovingBool' object by a further movement section defined by the
    * passed values.<br>
    * Creates an appropriate unit object and append this to this object.
    * 
    * @param period
    *           - time period of this movement section
    * 
    * @param booleanValue
    *           - boolean value for this period
    * 
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final Period period, final boolean booleanValue) {

      return add(new UnitBool(period, booleanValue));
   }

   /**
    * Append this 'MovingBool' object by a further movement section defined by the
    * passed values.<br>
    * Creates an appropriate unit object and append this to this object.
    * 
    * @param period
    *           - time period of this movement section
    * 
    * @param baseBoolValue
    *           - BaseBool value for this period
    * 
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final Period period, final BaseBool baseBoolValue) {

      return add(new UnitBool(period, baseBoolValue));
   }

   /**
    * Extends this 'MovingBool' object by the passed 'MovingBool' object.<br>
    * The time period of the passed object must be right adjacent to or begin after
    * the period of this object.
    * 
    * @param period
    *           - time period of this movement section
    * 
    * @param otherMovingBool
    *           - 'MovingBool' to append
    * 
    * @return true if the adding was successful, false otherwise
    */
   public boolean add(final MovingBool otherMovingBool) {
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
    * @see mol.datatypes.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitBool getUndefinedUnitObject() {
      return new UnitBool();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected BaseBool getUndefinedObject() {
      return new BaseBool();
   }

}
