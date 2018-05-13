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

package mol.datatypes.base;

import static org.junit.Assert.*;

import org.junit.Test;

/**
 * Tests for class 'BaseInt'
 * 
 * @author Markus Fuessel
 */
public class BaseIntTest {

	@Test
	public void testHashCode_DifferentObjectSameValue_HashCodeShouldBeEqual() {
		BaseInt posIntegerA = new BaseInt(1);
		BaseInt posIntegerB = new BaseInt(1);
		BaseInt negIntegerA = new BaseInt(-1);
		BaseInt negIntegerB = new BaseInt(-1);

		assertTrue(posIntegerA.hashCode() == posIntegerB.hashCode());
		assertTrue(negIntegerA.hashCode() == negIntegerB.hashCode());
	}

	@Test
	public void testHashCode_DifferentValue_HashCodeShouldBeNonEqual() {
		BaseInt posInteger = new BaseInt(1);
		BaseInt negInteger = new BaseInt(-1);

		assertFalse(posInteger.hashCode() == negInteger.hashCode());
	}

	@Test
	public void testCompareTo_LowVsHighValue_ShouldBeLowerZero() {
		BaseInt posInteger = new BaseInt(1);
		BaseInt negInteger = new BaseInt(-1);

		assertTrue(negInteger.compareTo(posInteger) < 0);

	}

	@Test
	public void testCompareTo_HighVsLowValue_ShouldBeGreaterZero() {
		BaseInt posInteger = new BaseInt(1);
		BaseInt negInteger = new BaseInt(-1);

		assertTrue(posInteger.compareTo(negInteger) > 0);

	}

	@Test
	public void testCompareTo_SameValue_ShouldBeZero() {
		BaseInt posIntegerA = new BaseInt(1);
		BaseInt posIntegerB = new BaseInt(1);
		BaseInt negIntegerA = new BaseInt(-1);
		BaseInt negIntegerB = new BaseInt(-1);

		assertTrue(posIntegerA.compareTo(posIntegerB) == 0);
		assertTrue(negIntegerA.compareTo(negIntegerB) == 0);

	}

	@Test
	public void testEquals_EqualValue_ShouldBeTrue() {
		BaseInt posIntegerA = new BaseInt(1);
		BaseInt posIntegerB = new BaseInt(1);
		BaseInt negIntegerA = new BaseInt(-1);
		BaseInt negIntegerB = new BaseInt(-1);

		assertTrue(posIntegerA.equals(posIntegerB));
		assertTrue(negIntegerA.equals(negIntegerB));

	}

	@Test
	public void testEquals_NonEqualValue_ShouldBeFalse() {
		BaseInt posInteger = new BaseInt(1);
		BaseInt negInteger = new BaseInt(-1);

		assertFalse(posInteger.equals(negInteger));

	}

}
