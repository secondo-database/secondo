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

package unittests.mmdb.service;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryRelation.RelationHeaderItem;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.convert.ConvertToListException;
import mmdb.error.convert.ConvertToObjectException;
import mmdb.service.ObjectConverter;

import org.junit.Before;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilMocks.ObjectListMock;
import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "ObjectConverter".
 *
 * @author Alexander Castor
 */
public class ObjectConverterTests {

	private ObjectListMock objectList;
	private List<SecondoObject> objects;

	@Before
	public void init() {
		objectList = new ObjectListMock();
		objects = new ArrayList<SecondoObject>();
		SecondoObject firstObject = new SecondoObject("object_1 [+]", null);
		firstObject.setMemoryObject(TestUtilRelation.getIntStringRelation(1, false, false));
		SecondoObject secondObject = new SecondoObject("object_2 [+]", null);
		secondObject.setMemoryObject(TestUtilRelation.getIntStringRelation(1, true, false));
		objects.add(firstObject);
		objects.add(secondObject);
		objectList.objects.addAll(objects);
	}

	@Test
	public void testConvertListToObjectValidRelation() throws Exception {
		ListExpr list = TestUtilRelation.getValidRelationList();
		MemoryRelation relation = ObjectConverter.getInstance().convertListToObject(list);
		List<RelationHeaderItem> header = relation.getHeader();
		List<MemoryTuple> tuples = relation.getTuples();
		assertEquals(1, header.size());
		assertEquals("identifier", header.get(0).getIdentifier());
		assertEquals("int", header.get(0).getTypeName());
		assertEquals(2, tuples.size());
		AttributeInt firstAttribute = (AttributeInt) tuples.get(0).getAttribute(0);
		AttributeInt secondAttribute = (AttributeInt) tuples.get(1).getAttribute(0);
		assertEquals(1, firstAttribute.getValue());
		assertEquals(2, secondAttribute.getValue());
	}

	@Test(expected = ConvertToObjectException.class)
	public void testConvertListToObjectInvalidRelation() throws Exception {
		ObjectConverter.getInstance()
				.convertListToObject(TestUtilRelation.getInvalidRelationList());
	}

	@Test
	public void testConvertObjectToList() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(2, false, false);
		ListExpr list = ObjectConverter.getInstance().convertObjectToList(relation);
		ListExpr type = list.first();
		ListExpr reltype = type.first();
		ListExpr tupletype = type.second();
		ListExpr tupleFirst = tupletype.first();
		ListExpr tupleSecond = tupletype.second();
		ListExpr tupleTypeInt = tupleSecond.first();
		ListExpr tupleTypeString = tupleSecond.second();
		assertEquals("rel", reltype.symbolValue());
		assertEquals("tuple", tupleFirst.symbolValue());
		assertEquals("identifierInt", tupleTypeInt.first().symbolValue());
		assertEquals("int", tupleTypeInt.second().symbolValue());
		assertEquals("identifierString", tupleTypeString.first().symbolValue());
		assertEquals("string", tupleTypeString.second().symbolValue());
		ListExpr value = list.second();
		ListExpr firstValue = value.first();
		ListExpr secondValue = value.second();
		assertEquals(1, firstValue.first().intValue());
		assertEquals("string_1", firstValue.second().stringValue());
		assertEquals(2, secondValue.first().intValue());
		assertEquals("string_2", secondValue.second().stringValue());
	}

	@Test
	public void testAddNestedListToSecondoObject() throws Exception {
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(1, false, false);
		SecondoObject object = new SecondoObject("object [+]", null);
		object.setMemoryObject(relation);
		ObjectConverter.getInstance().addNestedListToSecondoObject(object);
		assertNotNull(object.toListExpr());
		assertEquals("object [++]", object.getName());
	}

	@Test(expected = ConvertToListException.class)
	public void testConvertAllObjectsEmptyList() throws Exception {
		ObjectConverter.getInstance().convertAllObjects(new ArrayList<SecondoObject>(), null);
	}

	@Test
	public void testConvertAllObjectsNoFailure() throws Exception {
		List<String> failures = ObjectConverter.getInstance()
				.convertAllObjects(objects, objectList);
		assertEquals(0, failures.size());
		assertEquals(2, objectList.objects.size());
		assertNotNull(objectList.objects.get(0).toListExpr());
		assertNotNull(objectList.objects.get(1).toListExpr());

	}

	@Test
	public void testConvertAllObjectsOneFailure() throws Exception {
		objectList.objects.get(0).setMemoryObject(null);
		List<String> failures = ObjectConverter.getInstance()
				.convertAllObjects(objects, objectList);
		assertEquals(1, failures.size());
		assertEquals(2, objectList.objects.size());
		assertNull(objectList.objects.get(0).toListExpr());
		assertNotNull(objectList.objects.get(1).toListExpr());
	}

	@Test(expected = ConvertToListException.class)
	public void testConvertAllObjectsTwoFailures() throws Exception {
		objectList.objects.get(0).setMemoryObject(null);
		objectList.objects.get(1).setMemoryObject(null);
		ObjectConverter.getInstance().convertAllObjects(objects, objectList);
	}

}
