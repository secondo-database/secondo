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

package unittests.mmdb.data;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.indices.sorted.IndexSimpleArray;
import mmdb.error.convert.ConvertToObjectException;
import mmdb.error.index.IndexingException;
import mmdb.error.memory.MemoryException;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "MemoryRelation".
 *
 * @author Alexander Castor
 */
public class MemoryRelationTests {

	private static MemoryRelation relation;

	@BeforeClass
	public static void init() {
		relation = TestUtilRelation.getIntStringRelation(0, false, false);
	}

	@Test
	public void testCreateTupleFromListValidTypes() throws Exception {
		ListExpr list = ListExpr.twoElemList(ListExpr.intAtom(1), ListExpr.stringAtom("test"));
		MemoryAttribute expectedAttributeInt = new AttributeInt();
		expectedAttributeInt.fromList(list.first());
		MemoryAttribute expectedAttributeString = new AttributeString();
		expectedAttributeString.fromList(list.second());
		relation.createTupleFromList(list);
		MemoryTuple tuple = relation.getTuples().get(0);
		assertEquals(expectedAttributeInt, tuple.getAttribute(0));
		assertEquals(expectedAttributeString, tuple.getAttribute(1));
	}

	@Test(expected = ConvertToObjectException.class)
	public void testCreateTupleFromListInvalidType() throws ConvertToObjectException {
		RelationHeaderItem headerItem = new RelationHeaderItem("identifier", "invalidType");
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(headerItem);
		MemoryRelation relation = new MemoryRelation(header);
		relation.createTupleFromList(new ListExpr());
	}

	@Test
	public void testCreateIndexValidType() throws IndexingException, MemoryException {
		relation.createIndex("identifierInt", "SIMPLE_ARRAY");
		assertTrue(relation.getIndex("identifierInt") instanceof IndexSimpleArray);
	}

	@Test(expected = IndexingException.class)
	public void testCreateIndexInvalidType() throws Exception {
		relation.createIndex("identifierInt", "invalidType");
	}

	@Test
	public void testGetHeaderIndex() {
		int actual = relation.getHeaderIndex("identifierString");
		assertEquals(1, actual);
	}

	@Test
	public void testGetTypeName() {
		String actual = relation.getTypeName("identifierInt");
		assertEquals("int", actual);
	}

}
