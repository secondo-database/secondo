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
import mmdb.query.UnionController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "UnionController".
 *
 * @author Alexander Castor
 */
public class UnionControllerTests {

	@Test
	public void testExecuteQuerySameHeader() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(20, false, false);
		Object[] parameters = new Object[2];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		MemoryRelation result = (new UnionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(30, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(2, attributes.size());
	}

	@Test
	public void testExecuteQueryDifferentHeader() throws Exception {
		MemoryRelation firstRelation = TestUtilRelation.getIntStringRelation(10, false, false);
		MemoryRelation secondRelation = TestUtilRelation.getIntStringRelation(20, true, false);
		Object[] parameters = new Object[2];
		parameters[0] = firstRelation;
		parameters[1] = secondRelation;
		MemoryRelation result = (new UnionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(30, tuples.size());
		List<MemoryAttribute> attributes = tuples.get(0).getAttributes();
		assertEquals(2, attributes.size());
	}

}
