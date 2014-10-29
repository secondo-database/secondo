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
import mmdb.data.attributes.spatial.AttributePoints;

import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "AttributePoints".
 *
 * @author Alexander Castor
 */
public class AttributePointsTests {

	@Test
	public void testFromList() {
		AttributePoints points = new AttributePoints();
		points.fromList(TestUtilAttributes.getPointsList());
		assertEquals(2, points.getPoints().size());
	}

	@Test
	public void testToList() {
		ListExpr list = TestUtilAttributes.getPoints().toList();
		assertEquals(2, list.listLength());
	}

}
