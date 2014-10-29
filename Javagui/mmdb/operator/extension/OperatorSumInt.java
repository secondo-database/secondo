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

import mmdb.data.attributes.standard.AttributeInt;

/**
 * This class represents the 'SUM' operator for int-values.
 *
 * @author Alexander Castor
 */
public class OperatorSumInt extends ExtensionOperator {

	/**
	 * Calculates the sum of two attributes of type int.
	 * 
	 * @param first
	 *            the first int
	 * @param second
	 *            the second int
	 * @return the sum
	 */
	public AttributeInt operate(AttributeInt first, AttributeInt second) {
		int sum = first.getValue() + second.getValue();
		AttributeInt result = new AttributeInt();
		result.setValue(sum);
		return result;
	}

}
