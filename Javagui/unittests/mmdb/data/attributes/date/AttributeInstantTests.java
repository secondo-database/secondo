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

package unittests.mmdb.data.attributes.date;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.text.ParseException;
import java.util.Date;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.error.convert.ConversionException;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributeInstant".
 *
 * @author Alexander Castor
 */
public class AttributeInstantTests {

	private static AttributeInstant attributeOlder;
	private static AttributeInstant attributeNewer;

	@BeforeClass
	public static void init() throws ParseException {
		attributeOlder = TestUtilAttributes.getInstant("01");
		attributeNewer = TestUtilAttributes.getInstant("02");
	}

	@Test
	public void testFromListValid() throws Exception {
		AttributeInstant attribute = new AttributeInstant();
		Date expected = AttributeInstant.FORMAT_1.parse("2014-10-01-01:01:01.001");
		attribute.fromList(TestUtilAttributes.getStringList("2014-10-01-01:01:01.001"));
		assertEquals(expected, attribute.getDate());
		expected = AttributeInstant.FORMAT_2.parse("2014-10-01-01:01:01");
		attribute.fromList(TestUtilAttributes.getStringList("2014-10-01-01:01:01"));
		assertEquals(expected, attribute.getDate());
		expected = AttributeInstant.FORMAT_3.parse("2014-10-01-01:01");
		attribute.fromList(TestUtilAttributes.getStringList("2014-10-01-01:01"));
		assertEquals(expected, attribute.getDate());
		expected = AttributeInstant.FORMAT_4.parse("2014-10-01");
		attribute.fromList(TestUtilAttributes.getStringList("2014-10-01"));
		assertEquals(expected, attribute.getDate());
	}

	@Test(expected = ConversionException.class)
	public void testFromListInvalid() throws ConversionException {
		AttributeInstant attribute = new AttributeInstant();
		attribute.fromList(TestUtilAttributes.getStringList("2014-10-32"));
	}

	@Test
	public void testToList() throws ParseException {
		AttributeInstant attribute = new AttributeInstant();
		Date date = AttributeInstant.FORMAT_1.parse("2014-10-01-01:01:01.001");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_1);
		ListExpr list = attribute.toList();
		assertEquals("2014-10-01-01:01:01.001", list.stringValue());
		date = AttributeInstant.FORMAT_2.parse("2014-10-01-01:01:01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_2);
		list = attribute.toList();
		assertEquals("2014-10-01-01:01:01", list.stringValue());
		date = AttributeInstant.FORMAT_3.parse("2014-10-01-01:01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_3);
		list = attribute.toList();
		assertEquals("2014-10-01-01:01", list.stringValue());
		date = AttributeInstant.FORMAT_4.parse("2014-10-01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_4);
		list = attribute.toList();
		assertEquals("2014-10-01", list.stringValue());
	}

	@Test
	public void testEquals() {
		assertEquals(attributeOlder, attributeOlder);
		assertNotEquals(attributeOlder, attributeNewer);
	}

	@Test
	public void testHashCode() {
		assertEquals(-929840568, attributeOlder.hashCode());
		assertEquals(-843440568, attributeNewer.hashCode());
	}

	@Test
	public void testCompareTo() {
		assertTrue(attributeNewer.compareTo(attributeOlder) > 0);
		assertTrue(attributeOlder.compareTo(attributeNewer) < 0);
		assertTrue(attributeOlder.compareTo(attributeOlder) == 0);
	}

	@Test
	public void testParseValid() throws ParseException {
		AttributeInstant attribute = new AttributeInstant();
		Date date = AttributeInstant.FORMAT_1.parse("2014-10-01-01:01:01.001");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_1);
		attribute = (AttributeInstant) attributeOlder.parse("2014-10-01-01:01:01.001");
		assertEquals(date, attribute.getDate());
		date = AttributeInstant.FORMAT_2.parse("2014-10-01-01:01:01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_2);
		attribute = (AttributeInstant) attributeOlder.parse("2014-10-01-01:01:01");
		assertEquals(date, attribute.getDate());
		date = AttributeInstant.FORMAT_3.parse("2014-10-01-01:01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_3);
		attribute = (AttributeInstant) attributeOlder.parse("2014-10-01-01:01");
		assertEquals(date, attribute.getDate());
		date = AttributeInstant.FORMAT_4.parse("2014-10-01");
		attribute.setDate(date);
		attribute.setFormat(AttributeInstant.FORMAT_4);
		attribute = (AttributeInstant) attributeOlder.parse("2014-10-01");
		assertEquals(date, attribute.getDate());
	}

	@Test
	public void testParseInvalid() {
		AttributeInstant attribute = (AttributeInstant) attributeOlder.parse("32.10.2014");
		assertNull(attribute);
	}

}
