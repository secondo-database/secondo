//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package fuzzyobjects.basic;

import fuzzyobjects.GeoObject;
import java.io.Serializable;

/**
 * This class provides a interfaces for comparable
 * spatial basicobjects.
 * Such objects contains a fixed set of Basicpoints
 * @author Thomas Behr
 */

public interface BasicObject extends GeoObject,Serializable{

/**
 * make it possible to compare 2 BasicObjects
 *
 * @param BO the object to compare
 * @return <ul>
 *            <li>  -1 if the this smaller then BO </li>
 *            <li>   0 if this equals to BO </li>
 *            <li>   1 if this greater then BO </li>
 *          </ul>
 */
int compareTo(BasicObject BO);

/**
  * returns the BasicPoints contained in this object
  * @return the set of BasicPoints of this
  */
BasicPoint[] getBasicPoints();

/**
  * computes a distance between 2 BasicObjects
  * @param BO the another object
  * @return the euclidic distance between this and BO
  */
double euclid_distance(BasicObject BO);

/**
 * computes the quadratic euclidic distance
 * @param BO the another object
 * @return <code>euclid_distance</code><sup>2</sup>
 */
double q_euclid_distance(BasicObject BO);

/** returns the minX of the bounding box */
int getMinX();

/** returns the min Y of the bounding box */
int getMinY();

/** returns the maxX of the bounding box */
int getMaxX();

/** returns the max Y of the bounding box */
int getMaxY();

}

