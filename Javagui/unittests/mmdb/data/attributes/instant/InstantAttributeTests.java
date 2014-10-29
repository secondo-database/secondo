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

package unittests.mmdb.data.attributes.instant;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.instant.AttributeIbool;
import mmdb.data.attributes.instant.AttributeIint;
import mmdb.data.attributes.instant.AttributeIpoint;
import mmdb.data.attributes.instant.AttributeIreal;
import mmdb.data.attributes.instant.AttributeIregion;
import mmdb.data.attributes.instant.AttributeIstring;
import mmdb.data.attributes.instant.InstantAttribute;
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
public class InstantAttributeTests {

	private static AttributeInstant instant;
	private static List<InstantAttribute> attributes = new ArrayList<InstantAttribute>();
	private static List<ListExpr> lists = new ArrayList<ListExpr>();
	private static List<MemoryAttribute> values = new ArrayList<MemoryAttribute>();

	@BeforeClass
	public static void init() throws ParseException {
		instant = TestUtilAttributes.getInstant("01");
		attributes.add(new AttributeIbool());
		attributes.add(new AttributeIint());
		attributes.add(new AttributeIpoint());
		attributes.add(new AttributeIreal());
		attributes.add(new AttributeIregion());
		attributes.add(new AttributeIstring());
		lists.add(TestUtilAttributes.getBoolList(true));
		lists.add(TestUtilAttributes.getIntList(1));
		lists.add(TestUtilAttributes.getPointList(0.0f, 1.0f));
		lists.add(TestUtilAttributes.getRealList(1.0f));
		lists.add(TestUtilAttributes.getRegionList());
		lists.add(TestUtilAttributes.getStringList("A"));
		values.add(TestUtilAttributes.getBool(true));
		values.add(TestUtilAttributes.getInt(1));
		values.add(TestUtilAttributes.getPoint(0.0f, 1.0f));
		values.add(TestUtilAttributes.getReal(1.0f));
		values.add(TestUtilAttributes.getRegion());
		values.add(TestUtilAttributes.getString("A"));
	}

	@Test
	public void testFromList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			InstantAttribute<?> attribute = attributes.get(i);
			ListExpr list = lists.get(i);
			attribute.fromList(ListExpr.twoElemList(TestUtilAttributes.getInstantList("01"), list));
			assertNotNull(attribute.getInstant());
			assertNotNull(attribute.getValue());
		}
	}

	@Test
	@SuppressWarnings("unchecked")
	public void testToList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			InstantAttribute attribute = attributes.get(i);
			MemoryAttribute value = values.get(i);
			attribute.setInstant(instant);
			attribute.setValue(value);
			ListExpr list = attribute.toList();
			assertEquals(2, list.listLength());
		}
	}

}
