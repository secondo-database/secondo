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

package unittests.mmdb.operator.extension;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.lang.reflect.Field;

import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.operator.extension.OperatorAddString;
import mmdb.operator.extension.OperatorConcat;
import mmdb.operator.extension.OperatorRandomInt;
import mmdb.operator.extension.OperatorSumInt;
import mmdb.operator.extension.OperatorSumReal;

import org.junit.Test;

import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for all simple extension operators. More complicated ones have their
 * own test classes.
 *
 * @author Alexander Castor
 */
public class ExtensionOperatorTests {

	@Test
	public void testAddString() throws Exception {
		OperatorAddString operator = new OperatorAddString();
		Field text = operator.getClass().getDeclaredField("constValue");
		text.setAccessible(true);
		text.set(operator, "CONST");
		AttributeString argument = TestUtilAttributes.getString("A");
		AttributeString result = operator.operate(argument);
		assertEquals("ACONST", result.getValue());
	}

	@Test
	public void testConcat() {
		AttributeString first = TestUtilAttributes.getString("A");
		AttributeString second = TestUtilAttributes.getString("B");
		AttributeString result = (new OperatorConcat()).operate(first, second);
		assertEquals("AB", result.getValue());
	}

	@Test
	public void testRandomInt() {
		AttributeInt result = (new OperatorRandomInt()).operate();
		assertTrue(result.getValue() >= 0);
		assertTrue(result.getValue() <= 10);

	}

	@Test
	public void testSumInt() {
		AttributeInt first = TestUtilAttributes.getInt(1);
		AttributeInt second = TestUtilAttributes.getInt(2);
		AttributeInt result = (new OperatorSumInt()).operate(first, second);
		assertEquals(3, result.getValue());
	}

	@Test
	public void testSumReal() {
		AttributeReal first = TestUtilAttributes.getReal(1.0f);
		AttributeReal second = TestUtilAttributes.getReal(2.0f);
		AttributeReal result = (new OperatorSumReal()).operate(first, second);
		assertEquals(3.0f, result.getValue(), Float.MIN_VALUE);
	}

}
