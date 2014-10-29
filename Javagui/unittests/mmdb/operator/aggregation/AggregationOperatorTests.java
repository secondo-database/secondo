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

package unittests.mmdb.operator.aggregation;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.features.Orderable;
import mmdb.data.features.Summable;
import mmdb.operator.aggregation.OperatorAverage;
import mmdb.operator.aggregation.OperatorMax;
import mmdb.operator.aggregation.OperatorMin;
import mmdb.operator.aggregation.OperatorSum;

import org.junit.BeforeClass;
import org.junit.Test;

/**
 * Tests for all simple extension operators. More complicated ones have their
 * own test classes.
 *
 * @author Alexander Castor
 */
public class AggregationOperatorTests {

	private static List<Orderable> orderableList = new ArrayList<Orderable>();
	private static List<Summable> summableList = new ArrayList<Summable>();

	@BeforeClass
	public static void init() {
		for (int i = 1; i <= 5; i++) {
			AttributeInt attribute = new AttributeInt();
			attribute.setValue(i);
			orderableList.add(attribute);
			summableList.add(attribute);
		}
	}

	@Test
	public void testAverage() throws Exception {
		AttributeReal average = OperatorAverage.operate(summableList, summableList.get(0));
		assertEquals(3.0f, average.getValue(), Float.MIN_VALUE);
	}

	@Test
	public void testMax() {
		AttributeInt result = (AttributeInt) OperatorMax.operate(orderableList,
				orderableList.get(0));
		assertEquals(5, result.getValue());
	}

	@Test
	public void testMin() {
		AttributeInt result = (AttributeInt) OperatorMin.operate(orderableList,
				orderableList.get(0));
		assertEquals(1, result.getValue());
	}

	@Test
	public void testSum() {
		AttributeInt result = (AttributeInt) OperatorSum.operate(summableList, summableList.get(0));
		assertEquals(15, result.getValue());
	}

}
