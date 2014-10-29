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

package unittests.mmdb.data.attributes.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import mmdb.data.attributes.util.MovableUnitObjects.MovableFace;
import mmdb.data.attributes.util.MovableUnitObjects.MovableReal;
import mmdb.data.attributes.util.MovableUnitObjects.MovableRegion;

import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "MovableUnitObjects".
 *
 * @author Alexander Castor
 */
public class MovableUnitObjectsTests {

	/**
	 * Tests for class "MovableRegion".
	 *
	 * @author Alexander Castor
	 */
	public static class MovableRegionTests {

		@Test
		public void testFromList() {
			MovableRegion movableRegion = new MovableRegion();
			movableRegion.fromList(TestUtilAttributes.getMovableRegionList());
			assertEquals(2, movableRegion.getFaces().size());
		}

		@Test
		public void testToList() {
			ListExpr list = TestUtilAttributes.getMovableRegion().toList();
			assertEquals(2, list.listLength());
		}

	}

	/**
	 * Tests for class "MovableFace".
	 *
	 * @author Alexander Castor
	 */
	public static class MovableFaceTests {

		@Test
		public void testFromList() {
			MovableFace movableFace = new MovableFace();
			movableFace.fromList(TestUtilAttributes.getMovableFaceList());
			assertNotNull(movableFace.getOuterCycle());
			assertEquals(2, movableFace.getHoleCycles().size());
		}

		@Test
		public void testToList() {
			ListExpr list = TestUtilAttributes.getMovableFace().toList();
			assertEquals(3, list.listLength());
		}

	}

	/**
	 * Tests for class "MovableReal".
	 *
	 * @author Alexander Castor
	 */
	public static class MovableRealTests {

		@Test
		public void testFromList() {
			MovableReal movableReal = new MovableReal();
			movableReal.fromList(TestUtilAttributes.getMovableRealList());
			assertEquals(1.0f, movableReal.getFirstReal(), Float.MIN_VALUE);
			assertEquals(2.0f, movableReal.getSecondReal(), Float.MIN_VALUE);
			assertEquals(3.0f, movableReal.getThirdReal(), Float.MIN_VALUE);
			assertTrue(movableReal.getBooleanPart());
		}

		@Test
		public void testToList() {
			MovableReal movableReal = TestUtilAttributes.getMovableReal();
			ListExpr list = movableReal.toList();
			assertEquals(1.0, list.first().realValue(), Float.MIN_VALUE);
			assertEquals(2.0, list.second().realValue(), Float.MIN_VALUE);
			assertEquals(3.0, list.third().realValue(), Float.MIN_VALUE);
			assertTrue(list.fourth().boolValue());
		}

	}

}
