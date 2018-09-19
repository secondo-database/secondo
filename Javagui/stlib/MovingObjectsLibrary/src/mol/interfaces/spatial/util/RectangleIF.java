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

/**
 * Interface for RectangleIF objects, generally used for representing the
 * bounding box of spatial objects
 * 
 * @author Markus Fuessel
 */
public interface RectangleIF extends Spatial {

   /**
    * Copy this rectangle
    * 
    * @return new RectangleIF
    */
   RectangleIF copy();

   /**
    * Merge this and the passed rectangle to a new one and returns it
    * 
    * @return new RectangleIF
    */
   RectangleIF merge(RectangleIF rect);

   /**
    * Check if this 'RectangleIF' intersects with the passed one
    * 
    * @param rect
    *           - the other 'RectangleIF'
    * 
    * @return true if both 'RectangleIF' intersects, false otherwise
    */
   boolean intersects(RectangleIF rect);

   /**
    * Getter for the leftValue, left edge
    * 
    * @return the leftValue
    */
   double getLeftValue();

   /**
    * Getter for the rightValue, right edge
    * 
    * @return the rightValue
    */
   double getRightValue();

   /**
    * Getter for the topValue, upper edge
    * 
    * @return the topValue
    */
   double getTopValue();

   /**
    * Getter for the bottomValue, bottom edge
    * 
    * @return the bottomValue
    */
   double getBottomValue();

}