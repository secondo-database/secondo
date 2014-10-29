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
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import mmdb.data.attributes.standard.AttributeBool;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributeBool".
 *
 * @author Alexander Castor
 */
public class AttributeBoolTests {

	private static AttributeBool attributeTrue;
	private static AttributeBool attributeFalse;

	@BeforeClass
	public static void init() {
		attributeTrue = TestUtilAttributes.getBool(true);
		attributeFalse = TestUtilAttributes.getBool(false);
	}

	@Test
	public void testFromList() {
		AttributeBool attribute = new AttributeBool();
		attribute.fromList(TestUtilAttributes.getBoolList(true));
		assertTrue(attribute.isValue());
	}

	@Test
	public void testToList() {
		ListExpr list = attributeTrue.toList();
		assertTrue(list.boolValue());
	}

	@Test
	public void testEquals() {
		assertEquals(attributeTrue, attributeTrue);
		assertNotEquals(attributeTrue, attributeFalse);
	}

	@Test
	public void testHashCode() {
		assertEquals(1231, attributeTrue.hashCode());
		assertEquals(1237, attributeFalse.hashCode());
	}

	@Test
	public void testCompareTo() {
		assertTrue(attributeTrue.compareTo(attributeFalse) > 0);
		assertTrue(attributeFalse.compareTo(attributeTrue) < 0);
		assertTrue(attributeTrue.compareTo(attributeTrue) == 0);
	}

	@Test
	public void testParseValid() {
		AttributeBool attribute = (AttributeBool) attributeTrue.parse("true");
		assertTrue(attribute.isValue());
		attribute = (AttributeBool) attributeTrue.parse("false");
		assertFalse(attribute.isValue());
	}

	@Test
	public void testParseInvalid() {
		AttributeBool attribute = (AttributeBool) attributeTrue.parse("invalid");
		assertNull(attribute);
	}

}
