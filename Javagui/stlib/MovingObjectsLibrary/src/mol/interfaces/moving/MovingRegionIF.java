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
package mol.interfaces.moving;

import mol.interfaces.features.MovableSpatial;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.spatial.RegionIF;
import mol.interfaces.unit.spatial.UnitRegionIF;

/**
 * Interface that should be provided by MovingRegionIF objects
 * 
 * @author Markus Fuessel
 *
 */
public interface MovingRegionIF extends MovingObjectIF<UnitRegionIF, RegionIF>, MovableSpatial {

   /*
    * (non-Javadoc)
    * 
    * @see mol.datatypes.moving.MovingObjectIF#add(mol.datatypes.unit.UnitObject)
    */
   boolean add(UnitRegionIF unit);

   /**
    * Append this 'MovingRegion' object by a further movement section defined by
    * the passed values.<br>
    * Creates an constant region unit object and append this to this object.
    * 
    * @param period
    *           - time period of this movement section
    * @param region
    *           - the region
    * 
    * @return true if the adding was successful, false otherwise
    */
   boolean add(PeriodIF period, RegionIF region);

}