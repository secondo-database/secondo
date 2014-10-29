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

package mmdb.operator.extension;

/**
 * Superclass for all extension operators. An extension operator has exactly one
 * method 'operate' with at most three parameters of type MemoryAttribute and
 * also MemoryAttribute return type. The method might not be overloaded. It must
 * be declared public and non-static.
 *
 * @author Alexander Castor
 */
public abstract class ExtensionOperator {

	/**
	 * Called once before executions of operate method. Can be overridden i.e.
	 * to set up constant values.
	 */
	public void initialize() {
		// do nothing
	}

}
