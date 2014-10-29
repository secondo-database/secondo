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

import mmdb.data.attributes.standard.AttributeReal;

/**
 * This class represents the 'SUM' operator for real-values.
 *
 * @author Alexander Castor
 */
public class OperatorSumReal extends ExtensionOperator {

	/**
	 * Calculates the sum of two attributes of type real.
	 * 
	 * @param first
	 *            the first real
	 * @param second
	 *            the second real
	 * @return the sum
	 */
	public AttributeReal operate(AttributeReal first, AttributeReal second) {
		float sum = first.getValue() + second.getValue();
		AttributeReal result = new AttributeReal();
		result.setValue(sum);
		return result;
	}

}
