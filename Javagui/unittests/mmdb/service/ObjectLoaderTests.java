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
import static org.junit.Assert.assertTrue;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import javax.swing.JTextArea;

import mmdb.error.load.LoadDatabaseException;
import mmdb.error.load.LoadException;
import mmdb.error.load.LoadFromExplorerException;
import mmdb.error.load.LoadFromQueryException;
import mmdb.service.ObjectLoader;

import org.junit.Test;

import sj.lang.ListExpr;
import unittests.mmdb.util.TestUtilMocks.CommandPanelMock;
import unittests.mmdb.util.TestUtilMocks.ObjectListMock;
import unittests.mmdb.util.TestUtilRelation;

/**
 * Tests for class "ObjectConverter".
 *
 * @author Alexander Castor
 */
public class ObjectLoaderTests {

	@Test
	public void testReadFromCommandPanelValid() throws Exception {
		JTextArea textArea = new JTextArea();
		textArea.setText("Sec>command;");
		String command = ObjectLoader.getInstance().readCommandFromPanel(textArea);
		assertEquals("command", command);
	}

	@Test(expected = LoadFromQueryException.class)
	public void testReadFromCommandPanelInvalid() throws Exception {
		JTextArea textArea = new JTextArea();
		textArea.setText("invalid");
		ObjectLoader.getInstance().readCommandFromPanel(textArea);
	}

	@Test
	public void testExecuteRemoteCommandValid() throws Exception {
		CommandPanelMock commandPanel = new CommandPanelMock();
		ListExpr result = ObjectLoader.getInstance().executeRemoteCommand("command", commandPanel);
		assertNotNull(result);
		assertTrue(commandPanel.history.contains("command;"));
	}

	@Test(expected = LoadFromQueryException.class)
	public void testExecuteRemoteCommandInvalid() throws Exception {
		CommandPanelMock commandPanel = new CommandPanelMock();
		ObjectLoader.getInstance().executeRemoteCommand(null, commandPanel);
	}

	@Test
	public void testCreateSecondoObjectValid() throws Exception {
		ListExpr queryResult = TestUtilRelation.getValidRelationList();
		SecondoObject object = ObjectLoader.getInstance().createSecondoObject(queryResult,
				"command");
		assertNotNull(object.toListExpr());
		assertNotNull(object.getMemoryObject());
		assertEquals("command; [++]", object.getName());
	}

	@Test(expected = LoadException.class)
	public void testCreateSecondoObjectInvalid() throws Exception {
		ListExpr queryResult = TestUtilRelation.getInvalidRelationList();
		ObjectLoader.getInstance().createSecondoObject(queryResult, "command");
	}

	@Test
	public void testAddMemoryObjectToSecondoObjectValid() throws Exception {
		SecondoObject object = new SecondoObject("name", TestUtilRelation.getValidRelationList());
		ObjectLoader.getInstance().addMemoryObject(object);
		assertNotNull(object.getMemoryObject());
		assertEquals("name [++]", object.getName());
	}

	@Test(expected = LoadFromExplorerException.class)
	public void testAddMemoryObjectToSecondoObjectInvalid() throws Exception {
		SecondoObject object = new SecondoObject("name", TestUtilRelation.getInvalidRelationList());
		ObjectLoader.getInstance().addMemoryObject(object);
	}

	@Test
	public void testGetObjectListValidList() throws Exception {
		ListExpr firstObject = ListExpr.twoElemList(ListExpr.symbolAtom("OBJECT"),
				ListExpr.symbolAtom("object"));
		ListExpr secondObject = ListExpr.twoElemList(ListExpr.symbolAtom("OBJECT"),
				ListExpr.symbolAtom("SEC_object"));
		ListExpr OBJECTS = ListExpr.threeElemList(ListExpr.symbolAtom("OBJECTS"), firstObject,
				secondObject);
		ListExpr objects = ListExpr.twoElemList(ListExpr.symbolAtom("objects"), OBJECTS);
		ListExpr inquiry = ListExpr.twoElemList(ListExpr.symbolAtom("inquiry"), objects);
		List<String> result = ObjectLoader.getInstance().getObjectList(inquiry);
		assertEquals(1, result.size());
		assertEquals("object", result.get(0));
	}

	@Test(expected = LoadDatabaseException.class)
	public void testGetObjectListEmptyResult() throws Exception {
		ListExpr object = ListExpr.twoElemList(ListExpr.symbolAtom("OBJECT"),
				ListExpr.symbolAtom("SEC_object"));
		ListExpr OBJECTS = ListExpr.twoElemList(ListExpr.symbolAtom("OBJECTS"), object);
		ListExpr objects = ListExpr.twoElemList(ListExpr.symbolAtom("objects"), OBJECTS);
		ListExpr inquiry = ListExpr.twoElemList(ListExpr.symbolAtom("inquiry"), objects);
		ObjectLoader.getInstance().getObjectList(inquiry);
	}

	@Test(expected = LoadDatabaseException.class)
	public void testGetObjectListInvalidList() throws Exception {
		ObjectLoader.getInstance().getObjectList(new ListExpr());
	}

	@Test
	public void testLoadAllObjectsNoFailure() throws Exception {
		List<String> objects = new ArrayList<String>();
		objects.add("object1");
		objects.add("object2");
		ObjectListMock objectList = new ObjectListMock();
		List<String> failures = ObjectLoader.getInstance().loadAllObjects(objects,
				new CommandPanelMock(), objectList);
		assertTrue(failures.isEmpty());
		assertEquals(2, objectList.objects.size());
	}

	@Test
	public void testLoadAllObjectsOneFailure() throws Exception {
		List<String> objects = new ArrayList<String>();
		objects.add("object1");
		objects.add(null);
		ObjectListMock objectList = new ObjectListMock();
		List<String> failures = ObjectLoader.getInstance().loadAllObjects(objects,
				new CommandPanelMock(), objectList);
		assertEquals(1, failures.size());
		assertEquals(1, objectList.objects.size());
	}

	@Test(expected = LoadDatabaseException.class)
	public void testLoadAllObjectsTwoFailures() throws Exception {
		List<String> objects = new ArrayList<String>();
		objects.add(null);
		objects.add(null);
		ObjectListMock objectList = new ObjectListMock();
		ObjectLoader.getInstance().loadAllObjects(objects, new CommandPanelMock(), objectList);
	}

}
