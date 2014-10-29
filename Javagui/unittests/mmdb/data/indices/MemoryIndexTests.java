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

package unittests.mmdb.data.indices;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.indices.MemoryIndex;
import mmdb.data.indices.sorted.IndexRBTree;
import mmdb.data.indices.sorted.IndexSimpleArray;
import mmdb.data.indices.sorted.IndexTTree;
import mmdb.data.indices.unsorted.IndexSimpleHash;

import org.junit.BeforeClass;
import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for all memory indices.
 *
 * @author Alexander Castor
 */
public class MemoryIndexTests {

	private static MemoryRelation relation;

	@BeforeClass
	public static void init() {
		relation = TestUtilRelation.getIntStringRelation(5000, false, false);
		MemoryRelation tmp = TestUtilRelation.getIntStringRelation(5000, false, true);
		relation.getTuples().addAll(tmp.getTuples());
	}

	@Test
	public void testIndexRBTree() throws Exception {
		executeIndexTest(new IndexRBTree());
	}

	@Test
	public void testIndexSimpleArray() throws Exception {
		executeIndexTest(new IndexSimpleArray());
	}

	@Test
	public void testIndexTTree() throws Exception {
		executeIndexTest(new IndexTTree());
	}

	@Test
	public void testIndexSimpleHash() throws Exception {
		executeIndexTest(new IndexSimpleHash());
	}

	@SuppressWarnings({ "unchecked", "rawtypes" })
	private void executeIndexTest(MemoryIndex index) throws Exception {
		int result = index.create(0, relation);
		assertEquals(10000, result);
		AttributeInt key = new AttributeInt();
		key.setValue(5000);
		List<MemoryTuple> searchResult = index.searchElements(key);
		assertTrue(searchResult.size() == 2);
		assertEquals("string_5000",
				((AttributeString) searchResult.get(0).getAttribute(1)).getValue());
		assertEquals("string_5000",
				((AttributeString) searchResult.get(1).getAttribute(1)).getValue());
	}

}
