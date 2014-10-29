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

package unittests.mmdb.data.attributes.range;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.range.AttributeRbool;
import mmdb.data.attributes.range.AttributeRint;
import mmdb.data.attributes.range.AttributeRreal;
import mmdb.data.attributes.range.AttributeRstring;
import mmdb.data.attributes.range.RangeAttribute;
import mmdb.error.convert.ConversionException;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for all instant types.
 *
 * @author Alexander Castor
 */
@SuppressWarnings("rawtypes")
public class RangeAttributeTests {

	private static List<RangeAttribute> attributes = new ArrayList<RangeAttribute>();
	private static List<ListExpr> lists = new ArrayList<ListExpr>();
	private static List<MemoryAttribute> values = new ArrayList<MemoryAttribute>();

	@BeforeClass
	public static void init() throws ParseException {
		attributes.add(new AttributeRbool());
		attributes.add(new AttributeRint());
		attributes.add(new AttributeRreal());
		attributes.add(new AttributeRstring());
		lists.add(TestUtilAttributes.getBoolList(true));
		lists.add(TestUtilAttributes.getIntList(1));
		lists.add(TestUtilAttributes.getRealList(1.0f));
		lists.add(TestUtilAttributes.getStringList("A"));
		values.add(TestUtilAttributes.getBool(true));
		values.add(TestUtilAttributes.getInt(1));
		values.add(TestUtilAttributes.getReal(1.0f));
		values.add(TestUtilAttributes.getString("A"));
	}

	@Test
	public void testFromList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			RangeAttribute<?> attribute = attributes.get(i);
			ListExpr list = lists.get(i);
			attribute.fromList(ListExpr.fourElemList(list, list, lists.get(0), lists.get(0)));
			assertNotNull(attribute.getLowerBound());
			assertNotNull(attribute.getUpperBound());
			assertTrue(attribute.isLeftClosed());
			assertTrue(attribute.isRightClosed());
		}
	}

	@Test
	@SuppressWarnings("unchecked")
	public void testToList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			RangeAttribute attribute = attributes.get(i);
			MemoryAttribute value = values.get(i);
			attribute.setLowerBound(value);
			attribute.setUpperBound(value);
			attribute.setLeftClosed(true);
			attribute.setRightClosed(true);
			ListExpr list = attribute.toList();
			assertEquals(4, list.listLength());
			assertTrue(list.third().boolValue());
			assertTrue(list.fourth().boolValue());
		}
	}

}
