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

package unittests.mmdb.data.attributes.moving;

import static org.junit.Assert.assertEquals;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;

import mmdb.data.attributes.moving.AttributeMbool;
import mmdb.data.attributes.moving.AttributeMint;
import mmdb.data.attributes.moving.AttributeMpoint;
import mmdb.data.attributes.moving.AttributeMreal;
import mmdb.data.attributes.moving.AttributeMregion;
import mmdb.data.attributes.moving.AttributeMstring;
import mmdb.data.attributes.moving.MovingAttribute;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.error.convert.ConversionException;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for all moving types.
 *
 * @author Alexander Castor
 */
@SuppressWarnings("rawtypes")
public class MovingAttributeTests {

	private static List<MovingAttribute> attributes = new ArrayList<MovingAttribute>();
	private static List<ListExpr> lists = new ArrayList<ListExpr>();

	@BeforeClass
	public static void init() throws ParseException {
		AttributeMbool bool = new AttributeMbool();
		AttributeMint integer = new AttributeMint();
		AttributeMpoint point = new AttributeMpoint();
		AttributeMreal real = new AttributeMreal();
		AttributeMregion region = new AttributeMregion();
		AttributeMstring string = new AttributeMstring();
		attributes.add(bool);
		attributes.add(integer);
		attributes.add(point);
		attributes.add(real);
		attributes.add(region);
		attributes.add(string);
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUboolList(),
				TestUtilAttributes.getUboolList()));
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUintList(),
				TestUtilAttributes.getUintList()));
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUpointList(),
				TestUtilAttributes.getUpointList()));
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUrealList(),
				TestUtilAttributes.getUrealList()));
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUregionList(),
				TestUtilAttributes.getUregionList()));
		lists.add(ListExpr.twoElemList(TestUtilAttributes.getUstringList(),
				TestUtilAttributes.getUstringList()));
	}

	@Test
	public void testFromList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			MovingAttribute<?> attribute = attributes.get(i);
			attribute.fromList(lists.get(i));
			assertEquals(2, attribute.getUnits().size());
		}
	}

	@Test
	public void testToList() throws ConversionException {
		for (int i = 0; i < attributes.size(); i++) {
			MovingAttribute attribute = attributes.get(i);
			ListExpr list = attribute.toList();
			assertEquals(2, list.listLength());
		}
	}

	@Test
	@SuppressWarnings("unchecked")
	public void testGetPeriods() {
		for (int i = 0; i < attributes.size(); i++) {
			MovingAttribute attribute = attributes.get(i);
			List<Period> periods = attribute.getPeriods();
			assertEquals(2, periods.size());
		}
	}

}
