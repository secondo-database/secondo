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

package unittests.mmdb.service;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.indices.sorted.IndexSimpleArray;
import mmdb.error.memory.MemoryException;
import mmdb.service.MemoryWatcher;
import unittests.mmdb.util.TestUtilRelation;

import org.junit.Test;

import sj.lang.ListExpr;

/**
 * Tests for class "MemoryWatcher".
 *
 * @author Alexander Castor
 */
public class MemoryWatcherTests {

	@Test
	public void testCheckMemoryStatus() {
		MemoryException exception = null;
		try {
			MemoryWatcher.getInstance().checkMemoryStatus();
		} catch (MemoryException e) {
			exception = e;
		}
		assertNull(exception);
	}

	@Test
	public void testGetMemoryStatistics() {
		String[] statistics = MemoryWatcher.getInstance().getMemoryStatistics(0);
		assertNotNull(statistics[0]);
		assertNotNull(statistics[1]);
		assertNotNull(statistics[2]);
	}

	@Test
	public void testGetObjectStatistics() {
		List<SecondoObject> objects = new ArrayList<SecondoObject>();
		SecondoObject firstObject = new SecondoObject("object1", new ListExpr());
		SecondoObject secondObject = new SecondoObject("object2", null);
		MemoryRelation relation = TestUtilRelation.getIntStringRelation(1, false, false);
		relation.getIndices().put("identifierInt", new IndexSimpleArray());
		secondObject.setMemoryObject(relation);
		objects.add(firstObject);
		objects.add(secondObject);
		String[][] statistics = MemoryWatcher.getInstance().getObjectStatistics(objects);
		assertEquals(statistics[0][0], "object1");
		assertEquals(statistics[0][1], "n/a");
		assertEquals(statistics[0][2], "X");
		assertEquals(statistics[0][3], "");
		assertEquals(statistics[0][4], "");
		assertEquals(statistics[1][0], "object2");
		assertEquals(statistics[1][1], "1");
		assertEquals(statistics[1][2], "");
		assertEquals(statistics[1][3], "X");
		assertEquals(statistics[1][4], "X");
	}

}
