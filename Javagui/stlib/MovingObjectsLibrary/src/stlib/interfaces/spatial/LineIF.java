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

package stlib.interfaces.spatial;

import java.util.List;

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.features.Spatial;
import stlib.interfaces.spatial.util.HalfsegmentIF;
import stlib.interfaces.spatial.util.SegmentIF;

/**
 * Interface that should be provided by spatial objects of type line
 * 
 * 
 * @author Markus Fuessel
 */
public interface LineIF extends GeneralTypeIF, Spatial {

   /**
    * Add the passed 'Segment' object to this 'Line' object<br>
    * Adds internally two halfsegments to this 'Line' object<br>
    * Updates the bounding box
    * 
    * @param segment
    * @return true if the adding was successful, false otherwise
    */
   boolean add(SegmentIF segment);

   /**
    * Add the passed 'Point' objects to this 'Line' object<br>
    * Adds internally two halfsegments to this 'Line' object<br>
    * Updates the bounding box
    * 
    * @param p0
    *           - first endpoint
    * @param p1
    *           - second endpoint
    * 
    * @return true if the adding was successful, false otherwise
    */
   boolean add(PointIF p0, PointIF p1);

   /**
    * Add the passed halfsegment to this 'Line' object.<br>
    * Updates the bounding box
    * 
    * @param halfsegment
    * @return true if halfsegment was added, false otherwise
    */
   boolean add(HalfsegmentIF halfsegment);

   /**
    * Remove all segments from this 'Line' object and reset the bounding box
    */
   void clear();

   /**
    * Get the total length of this 'Line' object
    * 
    * @param useSphericalGeometry
    *           - if true spherical geometry is used to calculate the length in
    *           metres, otherwise euclidean distance is used
    * 
    * @return euclidean or geographical length, depends on parameter
    *         useSphericalGeometry
    */
   double length(boolean useSphericalGeometry);

   /**
    * Get the halfsegments
    * 
    * @return the halfsegments
    */
   List<HalfsegmentIF> getHalfsegments();

}