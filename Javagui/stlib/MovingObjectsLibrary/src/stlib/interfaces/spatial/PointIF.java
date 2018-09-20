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

import stlib.interfaces.GeneralTypeIF;
import stlib.interfaces.features.Spatial;
import stlib.util.Vector2D;

/**
 * Interface that should be provided by spatial objects of type point
 * 
 * 
 * @author Markus Fuessel
 */
public interface PointIF extends GeneralTypeIF, Spatial, Comparable<PointIF> {

   /**
    * Get the distance between this and the passed point.<br>
    * If this and the passed point are geographical points, then
    * useSphericalGeometry should set to true to get the geographical distance.
    * Otherwise euclidean distance is calculated and returned.
    * 
    * @param otherPoint
    *           - the other point to which the distance is to be calculated
    * 
    * @param useSphericalGeometry
    *           - if true spherical geometry is used to calculate the distance in
    *           metres, otherwise euclidean distance is used
    * 
    * @return euclidean or geographical distance, depends on parameter
    *         useSphericalGeometry
    */
   double distance(PointIF otherPoint, boolean useSphericalGeometry);

   /**
    * Verify if this point and the passed one are almost equal. This means that the
    * two points are considered the same if they differ only slightly.
    * 
    * @param otherPoint
    *           - the reference point with which to compare.
    * 
    * @return true, the two points are equal, false otherwise
    * 
    * @see stlib.util.GeneralHelper#almostEqual(double d1, double d2)
    */
   boolean almostEqual(PointIF otherPoint);

   /**
    * Get the x value
    * 
    * @return x value
    */
   double getXValue();

   /**
    * Get the y value
    * 
    * @return y value
    */
   double getYValue();

   /**
    * Add a 'Vector2D' to this 'Point'
    * 
    * @param vector
    *           - a 'Vector2D'
    * 
    * @return a new 'Point'
    */
   PointIF plus(Vector2D vector);

   /**
    * Subtract a 'Point' from this 'Point' to get a delta vector
    * 
    * @param point
    * 
    * @return a delta 'Vector2D'
    */
   Vector2D minus(PointIF point);

}