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
import mmdb.data.attributes.util.SpatialObjects.Face;
import mmdb.data.attributes.util.SpatialObjects.Segment;

import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilAttributes;

/**
 * Tests for class "SpatialObjects".
 *
 * @author Alexander Castor
 */
public class SpatialObjectsTests {

	/**
	 * Tests for class "Segment".
	 *
	 * @author Alexander Castor
	 */
	public static class SegmentTests {

		@Test
		public void testFromList() {
			Segment segment = new Segment();
			segment.fromList(TestUtilAttributes.getSegmentList());
			assertEquals(1.0f, segment.getxValue1(), Float.MIN_VALUE);
			assertEquals(2.0f, segment.getyValue1(), Float.MIN_VALUE);
			assertEquals(3.0f, segment.getxValue2(), Float.MIN_VALUE);
			assertEquals(4.0f, segment.getyValue2(), Float.MIN_VALUE);
		}

		@Test
		public void testToList() {
			Segment segment = TestUtilAttributes.getSegment();
			ListExpr list = segment.toList();
			assertEquals(1.0, list.first().realValue(), Float.MIN_VALUE);
			assertEquals(2.0, list.second().realValue(), Float.MIN_VALUE);
			assertEquals(3.0, list.third().realValue(), Float.MIN_VALUE);
			assertEquals(4.0, list.fourth().realValue(), Float.MIN_VALUE);
		}

	}

	/**
	 * Tests for class "Face".
	 *
	 * @author Alexander Castor
	 */
	public static class FaceTests {

		@Test
		public void testFromList() {
			Face face = new Face();
			face.fromList(TestUtilAttributes.getFaceList());
			assertNotNull(face.getOuterCycle());
			assertEquals(2, face.getHoleCycles().size());
		}

		@Test
		public void testToList() {
			ListExpr list = TestUtilAttributes.getFace().toList();
			assertEquals(3, list.listLength());
		}

	}

}
