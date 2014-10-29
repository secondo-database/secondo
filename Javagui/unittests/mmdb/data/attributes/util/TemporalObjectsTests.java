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

import java.text.ParseException;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.util.TemporalObjects.Period;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "TemporalObjects".
 *
 * @author Alexander Castor
 */
public class TemporalObjectsTests {

	/**
	 * Tests for class "Period".
	 *
	 * @author Alexander Castor
	 */
	public static class PeriodTests {

		private static Period firstPeriod;
		private static Period secondPeriod;

		@BeforeClass
		public static void init() throws Exception {
			firstPeriod = TestUtilAttributes.getPeriod("01", "03");
			secondPeriod = TestUtilAttributes.getPeriod("02", "03");
		}

		@Test
		public void testFromList() throws Exception {
			Period period = new Period();
			period.fromList(TestUtilAttributes.getPeriodList("01", "02"));
			assertNotNull(period.getStartTime());
			assertNotNull(period.getEndTime());
			assertTrue(period.isLeftClosed());
			assertTrue(period.isRightClosed());
		}

		@Test
		public void testToList() throws ParseException {
			ListExpr list = TestUtilAttributes.getPeriodList("01", "02");
			assertEquals(4, list.listLength());
			assertTrue(list.third().boolValue());
			assertTrue(list.fourth().boolValue());
		}

		@Test
		public void testCompareToStartTimesDifferent() {
			assertTrue(secondPeriod.compareTo(firstPeriod) > 0);
			assertTrue(firstPeriod.compareTo(secondPeriod) < 0);
			assertTrue(firstPeriod.compareTo(firstPeriod) == 0);
		}

		@Test
		public void testCompareToStartTimesEqual() {
			AttributeInstant tmp = secondPeriod.getStartTime();
			secondPeriod.setStartTime(firstPeriod.getStartTime());
			secondPeriod.setLeftClosed(false);
			assertTrue(firstPeriod.compareTo(secondPeriod) > 0);
			assertTrue(secondPeriod.compareTo(firstPeriod) < 0);
			secondPeriod.setLeftClosed(true);
			secondPeriod.setStartTime(tmp);
		}

	}

}
