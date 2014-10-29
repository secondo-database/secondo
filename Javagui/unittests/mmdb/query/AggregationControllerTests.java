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

import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.query.AggregationException;
import mmdb.operator.OperationController.AOperator;
import mmdb.query.AggregationController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "AggregationController".
 *
 * @author Alexander Castor
 */
public class AggregationControllerTests {

	@Test
	public void testExecuteQueryAverage() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(6, false, false);
		Object[] parameters = new Object[3];
		parameters[0] = relation;
		parameters[1] = AOperator.AVERAGE;
		parameters[2] = "identifierInt";
		MemoryRelation result = (new AggregationController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(1, attributes.size());
		AttributeReal resultAttribute = (AttributeReal) attributes.get(0);
		assertEquals(3.5, resultAttribute.getValue(), Float.MIN_VALUE);
	}

	@Test
	public void testExecuteQueryMax() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(6, true, false);
		Object[] parameters = new Object[3];
		parameters[0] = relation;
		parameters[1] = AOperator.MAX;
		parameters[2] = "identifierString";
		MemoryRelation result = (new AggregationController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(1, attributes.size());
		AttributeString resultAttribute = (AttributeString) attributes.get(0);
		assertEquals("string_6", resultAttribute.getValue());
	}

	@Test(expected = AggregationException.class)
	public void testExecuteQueryEmptyRelation() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(0, false, false);
		Object[] parameters = new Object[3];
		parameters[0] = relation;
		parameters[1] = AOperator.MAX;
		parameters[2] = "identifierInt";
		(new AggregationController()).executeQuery(parameters);
	}

}
