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

package fuzzyobjects.simple;

import fuzzyobjects.GeoObject;
import fuzzyobjects.basic.BasicObject;
import java.io.Serializable;

/**
 * a interface to unite fuzzy elementary objects
 * @author Thomas Behr
 */
public interface SimpleObject extends GeoObject,Serializable{

 /**
  * comutes the basic of this Object
  * @return the basic of this Object
  */
 BasicObject basic();

 /**
  * check whether this is a valid elementary Object
  * @return true if this is valid
  */
 boolean isValid();


 /** returns the min X of the bounding box */
 int getMinX();
 /** returns the max X of the bounding box */
 int getMaxX();
 /** returns the min Y of the bounding box */
 int getMinY();
 /** returns the max X of the bounding box */
 int getMaxY();

}
