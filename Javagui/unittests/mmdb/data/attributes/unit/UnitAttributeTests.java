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

package unittests.mmdb.data.attributes.unit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.unit.AttributeUbool;
import mmdb.data.attributes.unit.AttributeUint;
import mmdb.data.attributes.unit.AttributeUpoint;
import mmdb.data.attributes.unit.AttributeUreal;
import mmdb.data.attributes.unit.AttributeUregion;
import mmdb.data.attributes.unit.AttributeUstring;
import mmdb.data.attributes.unit.UnitAttribute;
import mmdb.data.attributes.util.TemporalObjects.Period;
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
public class UnitAttributeTests {

	private static List<UnitAttribute> attributes = new ArrayList<UnitAttribute>();
	private static List<ListExpr> lists = new ArrayList<ListExpr>();

	@BeforeClass
	public static void init() throws ParseException {
		attributes.add(new AttributeUbool());
		attributes.add(new AttributeUint());
		attributes.add(new AttributeUpoint());
		attributes.add(new AttributeUreal());
		attributes.add(new AttributeUregion());
		attributes.add(new AttributeUstring());
		lists.add(TestUtilAttributes.getUboolList());
		lists.add(TestUtilAttributes.getUintList());
		lists.add(TestUtilAttributes.getUpointList());
		lists.add(TestUtilAttributes.getUrealList());
		lists.add(TestUtilAttributes.getUregionList());
		lists.add(TestUtilAttributes.getUstringList());
	}

	@Test
	public void testFromList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			UnitAttribute<?> attribute = attributes.get(i);
			attribute.fromList(lists.get(i));
			assertNotNull(attribute.getPeriod());
			assertNotNull(attribute.getValue());
		}
	}

	@Test
	public void testToList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			UnitAttribute attribute = attributes.get(i);
			ListExpr list = attribute.toList();
			assertEquals(2, list.listLength());
		}
	}

	@Test
	@SuppressWarnings("unchecked")
	public void testGetPeriods() {
		for (int i = 0; i < attributes.size(); i++) {
			UnitAttribute attribute = attributes.get(i);
			List<Period> periods = attribute.getPeriods();
			assertEquals(1, periods.size());
		}
	}

}
