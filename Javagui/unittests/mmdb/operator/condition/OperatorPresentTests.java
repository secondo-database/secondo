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

import java.text.ParseException;

import mmdb.data.attributes.date.AttributeInstant;
import mmdb.data.attributes.moving.AttributeMpoint;
import mmdb.data.attributes.range.AttributePeriods;
import mmdb.data.attributes.util.TemporalObjects.Period;
import mmdb.operator.condition.OperatorPresent;

import org.junit.BeforeClass;
import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "OperatorPresent".
 *
 * @author Alexander Castor
 */
public class OperatorPresentTests {

	private static AttributeMpoint movable = new AttributeMpoint();
	private static AttributePeriods periods = new AttributePeriods();

	@BeforeClass
	public static void init() throws Exception {
		movable.fromList(ListExpr.threeElemList(TestUtilAttributes.getUpointList(),
				TestUtilAttributes.getUpointList(), TestUtilAttributes.getUpointList()));
		int count = 1;
		for (Period period : movable.getPeriods()) {
			period.setStartTime(TestUtilAttributes.getInstant("0" + count++));
			period.setEndTime(TestUtilAttributes.getInstant("0" + count));
			count += 2;
		}
		Period firstPeriod = TestUtilAttributes.getPeriod("01", "02");
		Period secondPeriod = TestUtilAttributes.getPeriod("03", "04");
		Period thirdPeriod = TestUtilAttributes.getPeriod("07", "08");
		firstPeriod.setLeftClosed(false);
		periods.addPeriod(firstPeriod);
		periods.addPeriod(secondPeriod);
		periods.addPeriod(thirdPeriod);
	}

	@Test
	public void testPresentInstantBegin() throws ParseException {
		AttributeInstant instant = TestUtilAttributes.getInstant("01");
		assertTrue(OperatorPresent.operate(movable, instant));
	}

	@Test
	public void testPresentInstantMiddle() throws ParseException {
		AttributeInstant instant = TestUtilAttributes.getInstant("04");
		assertTrue(OperatorPresent.operate(movable, instant));
	}

	@Test
	public void testPresentInstantEnd() throws ParseException {
		AttributeInstant instant = TestUtilAttributes.getInstant("07");
		assertTrue(OperatorPresent.operate(movable, instant));
	}

	@Test
	public void testPresentInstantNotPresent() throws ParseException {
		AttributeInstant instant = TestUtilAttributes.getInstant("09");
		assertFalse(OperatorPresent.operate(movable, instant));
	}

	@Test
	public void testPresentPeriodsIsPresent() {
		assertTrue(OperatorPresent.operate(movable, periods));
	}

	@Test
	public void testPresentPeriodsNotPresent() throws ParseException {
		Period tmp = periods.getPeriods().get(2);
		periods.getPeriods().remove(2);
		Period thirdPeriod = TestUtilAttributes.getPeriod("08", "09");
		periods.getPeriods().add(thirdPeriod);
		assertFalse(OperatorPresent.operate(movable, periods));
		periods.getPeriods().remove(2);
		periods.getPeriods().add(tmp);
	}

}
