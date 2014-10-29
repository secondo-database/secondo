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

import java.util.List;

import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.features.Summable;

/**
 * This class represents the 'AVERAGE' operator.
 *
 * @author Alexander Castor
 */
public class OperatorAverage implements AggregationOperator {

	/**
	 * Calculates the arithmetic mean for all elements of a given list of
	 * attributes.
	 * 
	 * @param attributes
	 *            the list of attributes being processed
	 * @return the arithmetic mean of all elements
	 */
	public static AttributeReal operate(List<Summable> attributes, Summable dummy) {
		AttributeReal result = new AttributeReal();
		float sum = 0;
		for (int i = 0; i < attributes.size(); i++) {
			sum += attributes.get(i).getValueAsReal();
		}
		float average = sum / attributes.size();
		result.setValue(average);
		return result;
	}

}
