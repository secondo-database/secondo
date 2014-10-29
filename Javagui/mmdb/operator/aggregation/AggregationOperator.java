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

package mmdb.operator.aggregation;

/**
 * Marker-Interface for all aggregation operators. An aggregation operator has a
 * method 'operate' with one parameter of type List<MemoryAttribute/Feature> and
 * a second parameter which is the before specified generic list type parameter.
 * This is necessary because at runtime generic type parameters are erased and
 * without the second parameter it would not be possible to determine the
 * correct argument type during reflection. The method returns a
 * MemoryAttribute. The method must not be overloaded, be declared public and
 * static.
 *
 * @author Alexander Castor
 */
public interface AggregationOperator {

}
