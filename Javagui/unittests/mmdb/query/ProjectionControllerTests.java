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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.query.ProjectionController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "ProjectionController".
 *
 * @author Alexander Castor
 */
public class ProjectionControllerTests {

	@Test
	public void testExecuteQueryOneAttribute() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(10, false, false);
		List<String> projectionAttributes = new ArrayList<String>();
		projectionAttributes.add("identifierInt");
		Object[] parameters = new Object[2];
		parameters[0] = relation;
		parameters[1] = projectionAttributes;
		MemoryRelation result = (new ProjectionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(10, tuples.size());
		RelationHeaderItem headerItemInt = result.getHeader().get(0);
		RelationHeaderItem headerItemString = result.getHeader().get(1);
		assertTrue(headerItemInt.isProjected());
		assertFalse(headerItemString.isProjected());
	}

	@Test
	public void testExecuteQueryTwoAttributes() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(10, false, false);
		List<String> projectionAttributes = new ArrayList<String>();
		projectionAttributes.add("identifierInt");
		projectionAttributes.add("identifierString");
		Object[] parameters = new Object[2];
		parameters[0] = relation;
		parameters[1] = projectionAttributes;
		MemoryRelation result = (new ProjectionController()).executeQuery(parameters);
		List<MemoryTuple> tuples = result.getTuples();
		assertEquals(10, tuples.size());
		RelationHeaderItem headerItemInt = result.getHeader().get(0);
		RelationHeaderItem headerItemString = result.getHeader().get(1);
		assertTrue(headerItemInt.isProjected());
		assertTrue(headerItemString.isProjected());
	}

}
