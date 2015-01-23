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
import mmdb.operator.OperationController.COperator;
import mmdb.query.JoinController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "JoinController".
 *
 * @author Alexander Castor
 */
public class JoinControllerTests {

	@Test
	public void testExecuteQueryNestedLoopJoin() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierString";
		parameters[3] = "identifierString";
		parameters[4] = COperator.CONTAINS;
		parameters[5] = true;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(11, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(4, attributes.size());
		assertEquals("identifierInt_R1", result.getHeader().get(0).getIdentifier());
		assertEquals("identifierString_R1", result.getHeader().get(1).getIdentifier());
		assertEquals("identifierInt_R2", result.getHeader().get(2).getIdentifier());
		assertEquals("identifierString_R2", result.getHeader().get(3).getIdentifier());
	}

	@Test
	public void testExecuteQueryHashJoinIndexSmallerRelation() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(2000, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(3000, false, false);
		firstRelation.createIndex("identifierInt", IndexType.SIMPLE_HASH.toString());
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierInt";
		parameters[3] = "identifierInt";
		parameters[4] = COperator.EQUALS;
		parameters[5] = true;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(2000, tuples.size());
	}

	@Test
	public void testExecuteQueryHashJoinIndexBiggerRelation() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(2000, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(3000, false, false);
		secondRelation.createIndex("identifierInt", IndexType.SIMPLE_HASH.toString());
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierInt";
		parameters[3] = "identifierInt";
		parameters[4] = COperator.EQUALS;
		parameters[5] = true;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(2000, tuples.size());
	}

	@Test
	public void testExecuteQueryHashJoinWithoutIndex() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(3000, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(2000, false, false);
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierInt";
		parameters[3] = "identifierInt";
		parameters[4] = COperator.EQUALS;
		parameters[5] = true;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(2000, tuples.size());
	}

	@Test
	public void testExecuteQueryNestedLoopJoinMeasureMode() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierString";
		parameters[3] = "identifierString";
		parameters[4] = COperator.CONTAINS;
		parameters[5] = false;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(1, attributes.size());
		AttributeInt attribute = (AttributeInt) attributes.get(0);
		assertEquals(11, attribute.getValue());
	}

	@Test
	public void testExecuteQueryHashJoinMeasureMode() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(2000, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(3000, false, false);
		Object[] parameters = new Object[6];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		parameters[2] = "identifierInt";
		parameters[3] = "identifierInt";
		parameters[4] = COperator.EQUALS;
		parameters[5] = false;
		MemoryRelation result = (new JoinController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(1, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(1, attributes.size());
		AttributeInt attribute = (AttributeInt) attributes.get(0);
		assertEquals(2000, attribute.getValue());
	}

}
