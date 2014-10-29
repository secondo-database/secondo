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

package unittests.mmdb.data.attributes.spatial;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.attributes.spatial.AttributePoint;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributePoint".
 *
 * @author Alexander Castor
 */
public class AttributePointTests {

	private static AttributePoint firstPoint;
	private static AttributePoint secondPoint;

	@BeforeClass
	public static void init() {
		firstPoint = TestUtilAttributes.getPoint(1.0f, 2.0f);
		secondPoint = TestUtilAttributes.getPoint(2.0f, 3.0f);
	}

	@Test
	public void testFromList() {
		AttributePoint attribute = new AttributePoint();
		attribute.fromList(TestUtilAttributes.getPointList(1.0f, 2.0f));
		assertEquals(1.0f, attribute.getXValue(), Float.MIN_VALUE);
		assertEquals(2.0f, attribute.getYValue(), Float.MIN_VALUE);
	}

	@Test
	public void testToList() {
		ListExpr list = firstPoint.toList();
		assertEquals(1.0, list.first().realValue(), Float.MIN_VALUE);
		assertEquals(2.0, list.second().realValue(), Float.MIN_VALUE);
	}

	@Test
	public void testEquals() {
		assertEquals(firstPoint, firstPoint);
		assertNotEquals(firstPoint, secondPoint);
	}

	@Test
	public void testHashCode() {
		assertEquals(-260045887, firstPoint.hashCode());
		assertEquals(4195265, secondPoint.hashCode());
	}

	@Test
	public void testParseValid() {
		AttributePoint attribute = (AttributePoint) firstPoint.parse("1.0 2.0");
		assertEquals(attribute, firstPoint);
	}

	@Test
	public void testParseInvalidInputNull() {
		AttributePoint attribute = (AttributePoint) firstPoint.parse(null);
		assertNull(attribute);
	}

	@Test
	public void testParseInvalidThreeTokens() {
		AttributePoint attribute = (AttributePoint) firstPoint.parse("1.0 2.0 3.0");
		assertNull(attribute);
	}

	@Test
	public void testParseInvalidFirstValue() {
		AttributePoint attribute = (AttributePoint) firstPoint.parse("invalid 1.0");
		assertNull(attribute);
	}

	@Test
	public void testParseInvalidSecondValue() {
		AttributePoint attribute = (AttributePoint) firstPoint.parse("1.0 invalid");
		assertNull(attribute);
	}

}
