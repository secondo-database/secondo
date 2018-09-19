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
package mol.interfaces.spatial;

import java.util.List;

import mol.interfaces.GeneralTypeIF;
import mol.interfaces.features.Spatial;
import mol.interfaces.spatial.util.FaceIF;
import mol.interfaces.spatial.util.HalfsegmentIF;

/**
 * Interface that should be provided by spatial objects of type region
 * 
 * 
 * @author Markus Fuessel
 */
public interface RegionIF extends GeneralTypeIF, Spatial {

   /**
    * Add a 'Face' object to this 'Region'<br>
    * Only a defined 'Face' will be added.
    * 
    * @param face
    *           - the 'Face' to add
    * 
    * @return true if adding was successful, false otherwise
    */
   boolean add(FaceIF face);

   /**
    * Get the number of components in this 'Region'. Number of faces.
    * 
    * @return number of faces in this 'Region'
    */
   int getNoComponents();

   /**
    * Get the entire halfsegments of boundary and holes
    * 
    * @return the halfsegments
    */
   List<HalfsegmentIF> getHalfsegments();

}