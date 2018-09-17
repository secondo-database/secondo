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

import mol.datatypes.base.BaseInt;
import mol.datatypes.interval.Period;

/**
 * This class represents 'UnitInt' objects and is used for 'MovingInt' objects
 * with a constant integer value over a certain period of time.
 * 
 * @author Markus Fuessel
 */
public class UnitInt extends UnitObjectConst<BaseInt> {

   /**
    * Constructor for an undefined 'UnitInt' object<br>
    * Required for subclasses
    */
   public UnitInt() {
      super(new BaseInt());
   }

   /**
    * Base constructor for a 'UnitInt' object
    * 
    * @param period
    *           - valid time period for this unit
    * @param intValue
    *           - the constant int value for this unit
    * 
    */
   public UnitInt(final Period period, final int intValue) {
      this(period, new BaseInt(intValue));
   }

   /**
    * Base constructor for a 'UnitInt' object
    * 
    * @param period
    *           - valid time period for this unit
    * @param baseIntValue
    *           - the constant BaseInt value for this unit
    * 
    */
   public UnitInt(final Period period, final BaseInt baseIntValue) {
      super(period, baseIntValue);
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#atPeriod(mol.datatypes.interval.Period)
    */
   @Override
   public UnitInt atPeriod(Period period) {
      Period newPeriod = this.getPeriod().intersection(period);

      if (!newPeriod.isDefined()) {
         return new UnitInt();
      }

      return new UnitInt(newPeriod, getValue());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObjectConst#getUndefinedObject()
    */
   @Override
   protected BaseInt getUndefinedObject() {
      return new BaseInt();
   }

}
