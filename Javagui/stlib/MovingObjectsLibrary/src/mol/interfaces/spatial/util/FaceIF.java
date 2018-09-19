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
package mol.interfaces.spatial.util;

import java.util.List;

import mol.interfaces.features.Spatial;
import mol.interfaces.spatial.PointIF;

/**
 * Interface for face objects
 * 
 * @author Markus Fuessel
 */
public interface FaceIF extends Spatial {

   /**
    * Add a hole to this 'FaceIF'.<br>
    * Only defined 'CycleIF' hole objects will be added.
    * 
    * @param hole
    *           - the hole, a 'CycleIF', to add
    * 
    * @return true if adding was successful, false otherwise
    */
   boolean add(CycleIF hole);

   /**
    * Add a hole to this 'FaceIF' by passing a list of points which define the hole
    * boundary. The points of the passed list are connected in the order of their
    * occurrence in the list, whereby the last point of the list is connected with
    * the first.
    * 
    * @param holePoints
    *           - the hole, List of points of the hole
    * 
    * @return true if adding was successful, false otherwise
    */
   boolean add(List<PointIF> holePoints);

   /**
    * Get the boundary of this 'FaceIF'
    * 
    * @return the boundary, a 'CycleIF' object
    */
   CycleIF getBoundary();

   /**
    * @return the holes
    */
   List<CycleIF> getHoles();

   /**
    * Get the number of holes in this 'FaceIF' object
    * 
    * @return number of holes
    */
   int getNoHoles();

   /**
    * Get the entire halfsegments of boundary and holes
    * 
    * @return the halfsegments
    */
   List<HalfsegmentIF> getHalfsegments();

}