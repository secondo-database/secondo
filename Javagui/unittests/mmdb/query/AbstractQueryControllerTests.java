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
import mmdb.query.AbstractQueryController;
import mmdb.query.ExtensionController;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "AbstractQueryController".
 *
 * @author Alexander Castor
 */
public class AbstractQueryControllerTests {

	@Test
	public void testDivideTupleListForThreadsFewTuples() {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(1, false, false);
		List<MemoryTuple> tuples = relation.getTuples();
		int[] result = (new ExtensionController()).divideTupleListForThreads(tuples);
		assertEquals(0, result[0]);
		for (int i = 1; i < result.length; i++) {
			assertEquals(-1, result[i]);
		}
	}

	@Test
	public void testDivideTupleListForThreadsManyTuples() {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(65, false, false);
		List<MemoryTuple> tuples = relation.getTuples();
		int[] result = (new ExtensionController()).divideTupleListForThreads(tuples);
		assertEquals(65 / AbstractQueryController.NUMBER_OF_THREADS - 1, result[0]);
		for (int i = 1; i < result.length - 1; i++) {
			int range = result[i - 1] + 65 / AbstractQueryController.NUMBER_OF_THREADS;
			assertEquals(range, result[i]);
		}
		assertEquals(64, result[result.length - 1]);
	}

}
