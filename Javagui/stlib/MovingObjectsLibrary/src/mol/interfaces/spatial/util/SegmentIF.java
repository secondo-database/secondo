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

import mol.interfaces.features.Spatial;
import mol.interfaces.spatial.PointIF;

/**
 * Interface for segment objects, generally used for representing the spatial
 * line objects
 * 
 * @author Markus Fuessel
 */
public interface SegmentIF extends Spatial {

   /**
    * Verify if this 'SegmentIF' intersects with the passed other 'SegmentIF'.<br>
    * - Intersection thru left endpoint is a valid intersection<br>
    * - Intersection thru right endpoint is not a valid intersection
    * 
    * @param other
    *           - the other 'SegmentIF'
    * 
    * @return true - both 'SegmentIF' objects intersect each other, false -
    *         otherwise
    */
   boolean intersect(SegmentIF other);

   /**
    * Is the alignment of this segment is vertical
    * 
    * @return true - alignment is vertical, false otherwise
    */
   boolean isVertical();

   /**
    * Verify if the end points of this segment are very close to each other
    * 
    * @return true if the end points are very close to each other, false otherwise
    */
   boolean isAlmostAPoint();

   /**
    * Get the length of this 'SegmentIF'
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
    * Get the lower endpoint
    * 
    * @return the leftPoint
    */
   PointIF getLeftPoint();

   /**
    * Get the greater endpoint
    * 
    * @return the rightPoint
    */
   PointIF getRightPoint();

}