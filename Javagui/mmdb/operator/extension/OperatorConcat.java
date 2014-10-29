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

import mmdb.data.attributes.standard.AttributeString;

/**
 * This class represents the 'CONCAT' operator for string concatenation.
 *
 * @author Alexander Castor
 */
public class OperatorConcat extends ExtensionOperator {

	/**
	 * Concatenates the values of two attributes of type string.
	 * 
	 * @param first
	 *            the first string
	 * @param second
	 *            the second string
	 * @return the concatenation
	 */
	public AttributeString operate(AttributeString first, AttributeString second) {
		String concatenation = first.getValue() + second.getValue();
		AttributeString result = new AttributeString();
		result.setValue(concatenation);
		return result;
	}

}
