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

package mmdb.operator.condition;

import mmdb.data.attributes.standard.AttributeString;

/**
 * This class represents the 'CONTAINS' operator.
 *
 * @author Alexander Castor
 */
public class OperatorContains implements ConditionOperator {

	/**
	 * Checks if the attribute string contains the value string.
	 * 
	 * @param attribute
	 *            the attribute which is compared
	 * @param value
	 *            the value which is compared to the attribute
	 * @return true if the attribute contains the value, else false
	 */
	public static boolean operate(AttributeString attribute,
			AttributeString value) {
		return attribute.getValue().contains(value.getValue());
	}

}
