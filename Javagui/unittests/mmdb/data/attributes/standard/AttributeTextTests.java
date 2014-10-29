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
import mmdb.data.attributes.standard.AttributeText;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;

/**
 * Tests for class "AttributeText".
 *
 * @author Alexander Castor
 */
public class AttributeTextTests {

	private static AttributeText attributeA;
	private static AttributeText attributeB;

	@BeforeClass
	public static void init() {
		attributeA = new AttributeText();
		attributeA.fromList(ListExpr.textAtom("A"));
		attributeB = new AttributeText();
		attributeB.fromList(ListExpr.textAtom("B"));
	}

	@Test
	public void testFromList() {
		assertEquals("A", attributeA.getValue());
	}

	@Test
	public void testToList() {
		ListExpr list = attributeA.toList();
		assertEquals("<text>A</text--->", list.textValue());
	}

	@Test
	public void testEquals() {
		assertEquals(attributeA, attributeA);
		assertNotEquals(attributeA, attributeB);
	}

	@Test
	public void testHashCode() {
		assertEquals(65, attributeA.hashCode());
		assertEquals(66, attributeB.hashCode());
	}

	@Test
	public void testCompareTo() {
		assertTrue(attributeB.compareTo(attributeA) > 0);
		assertTrue(attributeA.compareTo(attributeB) < 0);
		assertTrue(attributeA.compareTo(attributeA) == 0);
	}

	@Test
	public void testParseValid() {
		AttributeText attribute = (AttributeText) attributeA.parse("A");
		assertEquals(attributeA, attribute);
	}

	@Test
	public void testParseInvalid() {
		AttributeText attribute = (AttributeText) attributeA.parse(null);
		assertNull(attribute);
	}

}
