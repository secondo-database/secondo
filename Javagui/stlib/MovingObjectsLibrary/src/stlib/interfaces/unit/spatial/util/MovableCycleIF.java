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
package stlib.interfaces.unit.spatial.util;

import java.util.List;

import stlib.interfaces.features.MovableSpatial;
import stlib.interfaces.interval.PeriodIF;
import stlib.interfaces.spatial.util.CycleIF;
import stlib.interfaces.time.TimeInstantIF;

/**
 * Interface for MovableCycleIF objects, which are generally used to build
 * 'UnitRegionIF' objects.<br>
 * 
 * @author Markus Fuessel
 */
public interface MovableCycleIF extends MovableSpatial {

   /**
    * This method returns a 'CycleIF' which is valid at the passed time instant
    * within the period of the movement of this 'MovableCycleIF'.<br>
    * If the 'CycleIF' degenerates at the passed time instant, the returned
    * 'CycleIF' will be undefined.
    * 
    * @param movementPeriod
    *           - the period in which the movement of this cycle is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'CycleIF' which is valid at the passed instant, otherwise
    *         the returned 'CycleIF' is undefined
    */
   CycleIF getValue(PeriodIF movementPeriod, TimeInstantIF instant);

   /**
    * Get the initial 'CycleIF' object at the beginning of the movement
    * 
    * @return the initial 'CycleIF' object
    */
   CycleIF getInitial();

   /**
    * Get the final 'CycleIF' object at the end of the movement
    * 
    * @return the final 'CycleIF' object
    */
   CycleIF getFinal();

   /**
    * Get all 'MovableSegmentIF' objects of this object
    * 
    * @return list of 'MovableSegmentIF' objects
    */
   List<MovableSegmentIF> getMovingSegments();

}