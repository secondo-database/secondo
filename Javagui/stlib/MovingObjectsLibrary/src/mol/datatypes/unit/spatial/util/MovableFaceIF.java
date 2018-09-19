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

import java.util.List;

import mol.interfaces.features.MovableSpatial;
import mol.interfaces.interval.PeriodIF;
import mol.interfaces.spatial.util.FaceIF;
import mol.interfaces.time.TimeInstantIF;

/**
 * Interface for MovableFaceIF objects, which are generally used to build
 * 'UnitRegionIF' objects.<br>
 * This objects can have moving holes.
 * 
 * @author Markus Fuessel
 */
public interface MovableFaceIF extends MovableSpatial {

   /**
    * Add a moving hole to this 'MovableFaceIF'
    * 
    * @param movingHole
    *           - the moving hole to add, a 'MovableCycle' object
    * 
    * @return true if adding was successful, false otherwise
    */
   boolean add(MovableCycleIF movingHole);

   /**
    * This method returns a 'FaceIF' which is valid at the passed time instant
    * within the period of the movement of this 'MovableFaceIF'
    * 
    * @param movementPeriod
    *           - the period in which the movement of this face is defined
    * @param instant
    *           - time instant
    * 
    * @return a defined 'FaceIF' which is valid at the passed instant, otherwise
    *         the returned 'FaceIF' is undefined
    */
   FaceIF getValue(PeriodIF movementPeriod, TimeInstantIF instant);

   /**
    * Get the initial 'FaceIF' object at the beginning of the movement
    * 
    * @return the initial 'FaceIF' object
    */
   FaceIF getInitial();

   /**
    * Get the final 'FaceIF' object at the end of the movement
    * 
    * @return the final 'FaceIF' object
    */
   FaceIF getFinal();

   /**
    * Get all 'MovableSegmentIF' objects of boundary and holes
    * 
    * @return list of 'MovableSegmentIF' objects of boundary and holes
    */
   List<MovableSegmentIF> getMovingSegments();

   /**
    * Get the number of moving holes in this 'MovableFaceIF' object
    * 
    * @return number of moving holes
    */
   int getNoMovingHoles();

}