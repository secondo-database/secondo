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
import mmdb.data.attributes.spatial.AttributeRect;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributeRect".
 *
 * @author Alexander Castor
 */
public class AttributeRectTests {

	private static AttributeRect firstRect;
	private static AttributeRect secondRect;

	@BeforeClass
	public static void init() {
		firstRect = new AttributeRect();
		firstRect.setLeftValue(1.0f);
		firstRect.setRightValue(2.0f);
		firstRect.setBottomValue(3.0f);
		firstRect.setTopValue(4.0f);
		secondRect = new AttributeRect();
		secondRect.setLeftValue(4.0f);
		secondRect.setRightValue(5.0f);
		secondRect.setBottomValue(6.0f);
		secondRect.setTopValue(7.0f);
	}

	@Test
	public void testFromList() {
		AttributeRect attribute = new AttributeRect();
		ListExpr list = ListExpr.fourElemList(TestUtilAttributes.getRealList(1.0f),
				TestUtilAttributes.getRealList(2.0f), TestUtilAttributes.getRealList(3.0f),
				TestUtilAttributes.getRealList(4.0f));
		attribute.fromList(list);
		assertEquals(1.0f, attribute.getLeftValue(), Float.MIN_VALUE);
		assertEquals(2.0f, attribute.getRightValue(), Float.MIN_VALUE);
		assertEquals(3.0f, attribute.getBottomValue(), Float.MIN_VALUE);
		assertEquals(4.0f, attribute.getTopValue(), Float.MIN_VALUE);
	}

	@Test
	public void testToList() {
		ListExpr list = firstRect.toList();
		assertEquals(1.0, list.first().realValue(), Float.MIN_VALUE);
		assertEquals(2.0, list.second().realValue(), Float.MIN_VALUE);
		assertEquals(3.0, list.third().realValue(), Float.MIN_VALUE);
		assertEquals(4.0, list.fourth().realValue(), Float.MIN_VALUE);
	}

	@Test
	public void testEquals() {
		assertEquals(firstRect, firstRect);
		assertNotEquals(firstRect, secondRect);
	}

	@Test
	public void testHashCode() {
		assertEquals(936253313, firstRect.hashCode());
		assertEquals(1007556481, secondRect.hashCode());
	}

}
