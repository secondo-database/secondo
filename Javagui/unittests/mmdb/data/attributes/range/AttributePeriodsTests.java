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

package unittests.mmdb.data.attributes.range;

import static org.junit.Assert.assertEquals;

import java.text.ParseException;

import mmdb.data.attributes.range.AttributePeriods;
import mmdb.error.convert.ConversionException;

import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributePeriods".
 *
 * @author Alexander Castor
 */
public class AttributePeriodsTests {

	@Test
	public void testFromList() throws ConversionException {
		AttributePeriods periods = new AttributePeriods();
		periods.fromList(ListExpr.twoElemList(TestUtilAttributes.getPeriodList("01", "02"),
				TestUtilAttributes.getPeriodList("03", "04")));
		assertEquals(2, periods.getPeriods().size());
	}

	@Test
	public void testToList() throws ParseException {
		AttributePeriods periods = new AttributePeriods();
		periods.addPeriod(TestUtilAttributes.getPeriod("01", "02"));
		periods.addPeriod(TestUtilAttributes.getPeriod("03", "04"));
		ListExpr list = periods.toList();
		assertEquals(2, list.listLength());
	}

}
