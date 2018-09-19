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
package mol.datatypes.unit.spatial.util;

import mol.interfaces.features.MovableSpatial;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.spatial.PointIF;
import mol.interfaces.spatial.util.SegmentIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * Interface for MovableSegmentIF objects, generally used for representing the
 * bounding box of spatial objects
 * 
 * @author Markus Fuessel
 */
public interface MovableSegmentIF extends MovableSpatial {

   /**
    * This method returns a 'SegmentIF' which is valid at the passed time instant
    * within the period of the movement of this 'MovableSegmentIF'
    * 
    * @param movementPeriod
    *           - the period in which the movement of this segment is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'SegmentIF' which is valid at the passed instant, otherwise
    *         the returned 'SegmentIF' is undefined
    */
   SegmentIF getValue(PeriodIF movementPeriod, TimeInstantIF instant);

   /**
    * Get the initial 'SegmentIF' object at the beginning of the movement
    * 
    * @return the initial 'SegmentIF' object
    */
   SegmentIF getInitial();

   /**
    * Get the final 'SegmentIF' object at the end of the movement
    * 
    * @return the final 'SegmentIF' object
    */
   SegmentIF getFinal();

   /**
    * Get the start point at the beginning of the movement
    * 
    * @return the initial start point, a 'Point' object
    */
   PointIF getInitialStartPoint();

   /**
    * Get the start point at the end of the movement
    * 
    * @return the final start point, a 'Point' object
    */
   PointIF getFinalStartPoint();

   /**
    * Get the end point at the beginning of the movement
    * 
    * @return the initial end point, a 'Point' object
    */
   PointIF getInitialEndPoint();

   /**
    * Get the end point at the end of the movement
    * 
    * @return the final end point, a 'Point' object
    */
   PointIF getFinalEndPoint();

}