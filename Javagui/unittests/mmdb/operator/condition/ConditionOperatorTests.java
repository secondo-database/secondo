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

package unittests.mmdb.operator.condition;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.operator.condition.OperatorContains;
import mmdb.operator.condition.OperatorEquals;
import mmdb.operator.condition.OperatorEqualsGreater;
import mmdb.operator.condition.OperatorEqualsLess;
import mmdb.operator.condition.OperatorEqualsNot;
import mmdb.operator.condition.OperatorGreater;
import mmdb.operator.condition.OperatorLess;
import mmdb.operator.condition.OperatorTrue;

import org.junit.Test;

import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for all simple condition operators. More complicated ones have their
 * own test classes.
 *
 * @author Alexander Castor
 */
public class ConditionOperatorTests {

	private static AttributeString attributeA = TestUtilAttributes.getString("A");
	private static AttributeString attributeB = TestUtilAttributes.getString("B");

	@Test
	public void testContains() throws Exception {
		assertTrue(OperatorContains.operate(attributeA, attributeA));
		assertFalse(OperatorContains.operate(attributeA, attributeB));
	}

	@Test
	public void testEquals() throws Exception {
		assertTrue(OperatorEquals.operate(attributeA, attributeA));
		assertFalse(OperatorEquals.operate(attributeA, attributeB));
	}

	@Test
	public void testEqualsGreater() throws Exception {
		assertTrue(OperatorEqualsGreater.operate(attributeA, attributeA));
		assertTrue(OperatorEqualsGreater.operate(attributeB, attributeA));
		assertFalse(OperatorEqualsGreater.operate(attributeA, attributeB));
	}

	@Test
	public void testEqualsLess() throws Exception {
		assertTrue(OperatorEqualsLess.operate(attributeA, attributeA));
		assertTrue(OperatorEqualsLess.operate(attributeA, attributeB));
		assertFalse(OperatorEqualsLess.operate(attributeB, attributeA));
	}

	@Test
	public void testEqualsNot() throws Exception {
		assertTrue(OperatorEqualsNot.operate(attributeA, attributeB));
		assertFalse(OperatorEqualsNot.operate(attributeA, attributeA));
	}

	@Test
	public void testGreater() throws Exception {
		assertFalse(OperatorGreater.operate(attributeA, attributeA));
		assertTrue(OperatorGreater.operate(attributeB, attributeA));
		assertFalse(OperatorGreater.operate(attributeA, attributeB));
	}

	@Test
	public void testLess() throws Exception {
		assertFalse(OperatorLess.operate(attributeA, attributeA));
		assertTrue(OperatorLess.operate(attributeA, attributeB));
		assertFalse(OperatorLess.operate(attributeB, attributeA));
	}
	
	@Test
	public void testTrue() throws Exception {
		assertTrue(OperatorTrue.operate(attributeA, attributeA));
		assertTrue(OperatorTrue.operate(attributeA, attributeB));
		assertTrue(OperatorTrue.operate(attributeB, attributeA));
	}

}
