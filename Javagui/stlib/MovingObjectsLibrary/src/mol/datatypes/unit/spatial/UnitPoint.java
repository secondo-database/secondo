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
package mol.datatypes.unit.spatial;

import mol.datatypes.interval.Period;
import mol.datatypes.spatial.Point;
import mol.datatypes.unit.UnitObject;

/**
 * Abstract base class for 'UnitPoint' objects
 * 
 * @author Markus Fuessel
 */
public abstract class UnitPoint extends UnitSpatial<Point> {

   /**
    * Constructor for an undefined 'UnitPoint' object<br>
    * Required for subclasses
    */
   protected UnitPoint() {
   }

   /**
    * Base constructor for a 'UnitPoint' object<br>
    * Required for subclasses
    * 
    * @param period,
    *           the valid time period of this unit
    */
   protected UnitPoint(Period period) {
      super(period);
   }

   /*
    * (non-Javadoc)
    * 
    * @see
    * mol.datatypes.unit.UnitObject#finalEqualToInitialValue(mol.datatypes.unit.
    * UnitObject)
    */
   @Override
   public boolean finalEqualToInitialValue(UnitObject<Point> otherUnitObject) {
      return getFinal().almostEqual(otherUnitObject.getInitial());
   }

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.unit.UnitObject#atPeriod(mol.datatypes.interval.Period)
    */
   @Override
   public abstract UnitPoint atPeriod(Period period);

}
