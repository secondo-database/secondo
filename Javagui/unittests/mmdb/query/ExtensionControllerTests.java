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

package unittests.mmdb.query;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.operator.OperationController.EOperator;
import mmdb.query.ExtensionController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "ExtensionController".
 *
 * @author Alexander Castor
 */
public class ExtensionControllerTests {

	@Test
	public void testExecuteQuerySumInt() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		List<String> arguments = new ArrayList<String>();
		arguments.add("identifierInt");
		arguments.add("identifierInt");
		Object[] parameters = new Object[4];
		parameters[0] = relation;
		parameters[1] = EOperator.SUM_INT;
		parameters[2] = "identifierSum";
		parameters[3] = arguments;
		MemoryRelation result = (new ExtensionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(5000, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(3, attributes.size());
		assertEquals("identifierSum", result.getHeader().get(2).getIdentifier());
		for (int i = 0; i < 5000; i++) {
			AttributeInt summand = (AttributeInt) tuples.get(i).getAttribute(0);
			AttributeInt sum = (AttributeInt) tuples.get(i).getAttribute(2);
			assertEquals(2 * summand.getValue(), sum.getValue());
		}
	}

	@Test
	public void testExecuteQueryConcat() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		List<String> arguments = new ArrayList<String>();
		arguments.add("identifierString");
		arguments.add("identifierString");
		Object[] parameters = new Object[4];
		parameters[0] = relation;
		parameters[1] = EOperator.CONCAT;
		parameters[2] = "identifierConcat";
		parameters[3] = arguments;
		MemoryRelation result = (new ExtensionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(5000, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(3, attributes.size());
		assertEquals("identifierConcat", result.getHeader().get(2).getIdentifier());
		for (int i = 0; i < 5000; i++) {
			AttributeString inputString = (AttributeString) tuples.get(i).getAttribute(1);
			AttributeString concatenation = (AttributeString) tuples.get(i).getAttribute(2);
			assertEquals(inputString.getValue() + inputString.getValue(), concatenation.getValue());
		}
	}

}
