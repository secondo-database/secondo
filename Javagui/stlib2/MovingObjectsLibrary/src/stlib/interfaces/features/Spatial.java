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

package stlib.interfaces.features;

import stlib.interfaces.spatial.util.RectangleIF;

/**
 * Marker interface for spatial objects
 * 
 * @author Markus Fuessel
 */
public interface Spatial {

   /**
    * Verify if the spatial object is empty
    * 
    * @return true, if the spatial object is empty
    */
   boolean isEmpty();

   /**
    * Get the minimum bounding rectangle of the spatial object
    * 
    * @return a 'Rectangle' object
    */
   RectangleIF getBoundingBox();

   /**
    * Is this object defined
    * 
    * @return true if object is defined, false otherwise
    */
   boolean isDefined();

   /**
    * Set the defined flag of this Object
    * 
    * @param defined
    */
   void setDefined(final boolean defined);
}