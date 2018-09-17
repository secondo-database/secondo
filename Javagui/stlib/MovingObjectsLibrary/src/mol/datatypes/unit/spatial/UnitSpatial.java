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

import mol.datatypes.GeneralType;
import mol.datatypes.features.Spatial;
import mol.datatypes.interval.Period;
import mol.datatypes.spatial.util.Rectangle;
import mol.datatypes.unit.UnitObject;

/**
 * Abstract base class for all 'UnitSpatial' subclasses
 * 
 * @author Markus Fuessel
 *
 * @param <T>
 *           - specifies the spatial type
 */
public abstract class UnitSpatial<T extends GeneralType & Spatial> extends UnitObject<T> {

   /**
    * Constructor for an undefined 'UnitSpatial' object<br>
    * Required for subclasses
    */
   protected UnitSpatial() {
   }

   /**
    * Base constructor for a 'UnitSpatial' object<br>
    * Required for subclasses
    * 
    * @param period
    *           - valid time period for this unit
    */
   protected UnitSpatial(Period period) {
      super(period);
   }

   /**
    * Method to get the bounding box of the spatial projection of the spatial unit
    * object
    * 
    * @return the projectionBoundingBox, a 'Rectangle' object
    */
   public abstract Rectangle getProjectionBoundingBox();

}
