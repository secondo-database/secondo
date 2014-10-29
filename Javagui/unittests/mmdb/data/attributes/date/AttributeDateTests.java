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

import mmdb.data.attributes.date.AttributeDate;
import mmdb.error.convert.ConversionException;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;

/**
 * Tests for class "AttributeDate".
 *
 * @author Alexander Castor
 */
public class AttributeDateTests {

	private static AttributeDate attributeOlder;
	private static AttributeDate attributeNewer;

	@BeforeClass
	public static void init() throws ParseException {
		attributeOlder = new AttributeDate();
		Date older = AttributeDate.FORMAT_1.parse("1.10.2014");
		attributeOlder.setDate(older);
		attributeOlder.setFormat(AttributeDate.FORMAT_1);
		attributeNewer = new AttributeDate();
		Date newer = AttributeDate.FORMAT_1.parse("2.10.2014");
		attributeNewer.setDate(newer);
		attributeNewer.setFormat(AttributeDate.FORMAT_1);
	}

	@Test
	public void testFromListValid() throws Exception {
		AttributeDate attribute = new AttributeDate();
		Date expected = AttributeDate.FORMAT_1.parse("1.10.2014");
		attribute.fromList(ListExpr.stringAtom("1.10.2014"));
		assertEquals(expected, attribute.getDate());
		attribute.fromList(ListExpr.stringAtom("2014-10-1"));
		assertEquals(expected, attribute.getDate());
	}

	@Test(expected = ConversionException.class)
	public void testFromListInvalid() throws ConversionException {
		AttributeDate attribute = new AttributeDate();
		attribute.fromList(ListExpr.stringAtom("32.10.2014"));
	}

	@Test
	public void testToList() throws ParseException {
		AttributeDate attribute = new AttributeDate();
		Date date = AttributeDate.FORMAT_1.parse("1.10.2014");
		attribute.setDate(date);
		attribute.setFormat(AttributeDate.FORMAT_1);
		ListExpr list = attribute.toList();
		assertEquals("1.10.2014", list.stringValue());
		date = AttributeDate.FORMAT_2.parse("2014-10-1");
		attribute.setDate(date);
		attribute.setFormat(AttributeDate.FORMAT_2);
		list = attribute.toList();
		assertEquals("2014-10-1", list.stringValue());
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
		AttributeDate attribute = new AttributeDate();
		Date date = AttributeDate.FORMAT_1.parse("1.10.2014");
		attribute.setDate(date);
		attribute.setFormat(AttributeDate.FORMAT_1);
		attribute = (AttributeDate) attributeOlder.parse("1.10.2014");
		assertEquals(date, attribute.getDate());
		date = AttributeDate.FORMAT_2.parse("2014-10-1");
		attribute.setDate(date);
		attribute.setFormat(AttributeDate.FORMAT_2);
		attribute = (AttributeDate) attributeOlder.parse("2014-10-1");
		assertEquals(date, attribute.getDate());
	}

	@Test
	public void testParseInvalid() {
		AttributeDate attribute = (AttributeDate) attributeOlder.parse("32.10.2014");
		assertNull(attribute);
	}

}
