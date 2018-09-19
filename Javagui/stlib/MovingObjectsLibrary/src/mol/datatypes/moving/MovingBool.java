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
import mol.datatypes.unit.UnitBool;
import mol.interfaces.base.BaseBoolIF;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.moving.MovingBoolIF;
import mol.interfaces.unit.UnitBoolIF;

/**
 * This class represents moving boolean objects of type 'MovingBool'
 * 
 * @author Markus Fuessel
 */
public class MovingBool extends MovingObject<UnitBoolIF, BaseBoolIF> implements MovingBoolIF {

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

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingBoolIF#add(mol.interfaces.interval.PeriodIF,
    * boolean)
    */
   @Override
   public boolean add(final PeriodIF period, final boolean booleanValue) {

      return add(new UnitBool(period, booleanValue));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingBoolIF#add(mol.interfaces.interval.PeriodIF,
    * mol.interfaces.base.BaseBoolIF)
    */
   @Override
   public boolean add(final PeriodIF period, final BaseBoolIF baseBoolValue) {

      return add(new UnitBool(period, baseBoolValue));
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingBoolIF#add(mol.datatypes.moving.MovingBoolIF)
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
    * @see mol.datatypes.moving.MovingObject#getUndefinedUnitObject()
    */
   @Override
   protected UnitBoolIF getUndefinedUnitObject() {
      return new UnitBool();
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObject#getUndefinedObject()
    */
   @Override
   protected BaseBoolIF getUndefinedObject() {
      return new BaseBool();
   }

}
