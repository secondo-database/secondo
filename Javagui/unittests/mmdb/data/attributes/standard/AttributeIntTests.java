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

package unittests.mmdb.data.attributes.standard;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import mmdb.data.attributes.standard.AttributeInt;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributeInt".
 *
 * @author Alexander Castor
 */
public class AttributeIntTests {

	private static AttributeInt attribute1;
	private static AttributeInt attribute2;

	@BeforeClass
	public static void init() {
		attribute1 = TestUtilAttributes.getInt(1);
		attribute2 = TestUtilAttributes.getInt(2);
	}

	@Test
	public void testFromList() {
		AttributeInt attribute = new AttributeInt();
		attribute.fromList(TestUtilAttributes.getIntList(1));
		assertEquals(1, attribute.getValue());
	}

	@Test
	public void testToList() {
		ListExpr list = attribute1.toList();
		assertEquals(1, list.intValue());
	}

	@Test
	public void testEquals() {
		assertEquals(attribute1, attribute1);
		assertNotEquals(attribute1, attribute2);
	}

	@Test
	public void testHashCode() {
		assertEquals(1, attribute1.hashCode());
		assertEquals(2, attribute2.hashCode());
	}

	@Test
	public void testCompareTo() {
		assertTrue(attribute2.compareTo(attribute1) > 0);
		assertTrue(attribute1.compareTo(attribute2) < 0);
		assertTrue(attribute1.compareTo(attribute1) == 0);
	}

	@Test
	public void testParseValid() {
		AttributeInt attribute = (AttributeInt) attribute1.parse("1");
		assertEquals(attribute1, attribute);
	}

	@Test
	public void testParseInvalid() {
		AttributeInt attribute = (AttributeInt) attribute1.parse("invalid");
		assertNull(attribute);
	}

}
