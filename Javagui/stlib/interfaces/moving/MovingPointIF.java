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
package stlib.interfaces.moving;

import stlib.interfaces.features.MovableSpatial;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.PointIF;
import stlib.interfaces.unit.spatial.UnitPointIF;

/**
 * Interface that should be provided by MovingPointIF objects
 * 
 * @author Markus Fuessel
 *
 */
public interface MovingPointIF extends MovingObjectIF<UnitPointIF, PointIF>, MovableSpatial {

   /**
    * Append this 'MovingPoint' object by a further movement section defined by the
    * passed values.<br>
    * Creates an appropriate unit object and append this to this object.
    * 
    * @param period
    *           - time period of this movement section
    * @param startPoint
    *           - point at the beginning of the movement for this period
    * @param endPoint
    *           - point at the end of the movement for this period
    * 
    * @return true if the adding was successful, false otherwise
    */
   boolean add(PeriodIF period, PointIF startPoint, PointIF endPoint);

}