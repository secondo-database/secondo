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
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.indices.MemoryIndex.IndexType;
import mmdb.error.query.SelectionException;
import mmdb.operator.OperationController.COperator;
import mmdb.query.SelectionController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "SelectionController".
 *
 * @author Alexander Castor
 */
public class SelectionControllerTests {

	@Test
	public void testExecuteQueryWithoutIndex() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		AttributeInt value = new AttributeInt();
		value.setValue(100);
		Object[] parameters = new Object[5];
		parameters[0] = relation;
		parameters[1] = COperator.LESS;
		parameters[2] = "identifierInt";
		parameters[3] = value;
		parameters[4] = true;
		MemoryRelation result = (new SelectionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(99, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(2, attributes.size());
	}

	@Test
	public void testExecuteQueryWithIndex() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		relation.createIndex("identifierInt", IndexType.T_TREE.toString());
		AttributeInt value = new AttributeInt();
		value.setValue(100);
		Object[] parameters = new Object[5];
		parameters[0] = relation;
		parameters[1] = COperator.EQUALS;
		parameters[2] = "identifierInt";
		parameters[3] = value;
		parameters[4] = true;
		MemoryRelation result = (new SelectionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(2, attributes.size());
		AttributeInt resultAttribute = (AttributeInt) attributes.get(0);
		assertEquals(100, resultAttribute.getValue());
	}

	@Test(expected = SelectionException.class)
	public void testExecuteQueryEmptyResult() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		AttributeInt value = new AttributeInt();
		value.setValue(5000);
		Object[] parameters = new Object[5];
		parameters[0] = relation;
		parameters[1] = COperator.GREATER;
		parameters[2] = "identifierInt";
		parameters[3] = value;
		parameters[4] = true;
		(new SelectionController()).executeQuery(parameters);
	}

	@Test
	public void testExecuteQueryMeasureMode() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		AttributeInt value = new AttributeInt();
		value.setValue(100);
		Object[] parameters = new Object[5];
		parameters[0] = relation;
		parameters[1] = COperator.LESS;
		parameters[2] = "identifierInt";
		parameters[3] = value;
		parameters[4] = false;
		MemoryRelation result = (new SelectionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(1, attributes.size());
		AttributeInt attribute = (AttributeInt) attributes.get(0);
		assertEquals(99, attribute.getValue());
	}

}
