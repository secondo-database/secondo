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

package mmdb.service;

import gui.CommandPanel;
import gui.ObjectList;
import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.util.ArrayList;
import java.util.List;

import javax.swing.JTextArea;

import mmdb.data.MemoryRelation;
import mmdb.error.convert.ConversionException;
import mmdb.error.load.LoadDatabaseException;
import mmdb.error.load.LoadException;
import mmdb.error.load.LoadFromExplorerException;
import mmdb.error.load.LoadFromQueryException;
import mmdb.error.memory.MemoryException;
import sj.lang.ListExpr;

/**
 * This class is responsible for loading objects to the MMDB either through a
 * user query via command panel or by converting an existing object in
 * nested-list representation from the object explorer.
 * 
 * @author Alexander Castor
 */
public final class ObjectLoader {

	/**
	 * The class' singleton instance.
	 */
	private static ObjectLoader instance = new ObjectLoader();

	/**
	 * Creates a new object loader object.
	 */
	private ObjectLoader() {
	}

	/**
	 * Retrieves the singleton instance.
	 * 
	 * @return the singleton instance
	 */
	public static ObjectLoader getInstance() {
		return instance;
	}

	/**
	 * Reads commands from the command panel. Command must end with ';'.
	 * 
	 * @param systemArea
	 *            the text area of command panel
	 * @return the command in string representation
	 * @throws LoadFromQueryException
	 * 
	 */
	public String readCommandFromPanel(JTextArea systemArea) throws LoadFromQueryException {
		String text = systemArea.getText();
		int startIndex = text.lastIndexOf("Sec>");
		int endIndex = text.lastIndexOf(";");
		if (startIndex > -1 && startIndex < endIndex - 4) {
			return text.substring(startIndex + 4, endIndex);
		} else {
			throw new LoadFromQueryException("-> Could not read query from command panel.");
		}
	}

	/**
	 * Executes a remote command at the connected SECONDO server.
	 * 
	 * @param command
	 *            the command that shall be executed
	 * @param commandPanel
	 *            the command panel which does the call
	 * @return the query result
	 * @throws LoadFromQueryException
	 * @throws MemoryException
	 */
	public ListExpr executeRemoteCommand(String command, CommandPanel commandPanel)
			throws LoadFromQueryException, MemoryException {
		MemoryWatcher.getInstance().checkMemoryStatus();
		commandPanel.addToHistory(command + ";");
		ListExpr queryResult = commandPanel.getCommandResult(command);
		if (queryResult != null) {
			return queryResult;
		} else {
			throw new LoadFromQueryException(
					"-> Could not execute remote command at SECONDO server.");
		}
	}

	/**
	 * Creates a secondo object from a query result containing both the nested
	 * list representation and the memory relation.
	 * 
	 * @param queryResult
	 *            the query result as nested list
	 * @param command
	 *            the command which serves as object description
	 * @return the secondo object
	 * @throws LoadException
	 * @throws MemoryException
	 */
	public SecondoObject createSecondoObject(ListExpr queryResult, String command)
			throws LoadException, MemoryException {
		SecondoObject secondoObject = new SecondoObject(IDManager.getNextID());
		secondoObject.fromList(queryResult);
		try {
			MemoryRelation relationObject = ObjectConverter.getInstance().convertListToObject(
					queryResult);
			secondoObject.setMemoryObject(relationObject);
			secondoObject.setName(command + "; [++]");
		} catch (ConversionException e) {
			throw new LoadException(
					"-> Could not convert nested list to memory object, caused by:\n"
							+ e.getMessage());
		}
		return secondoObject;
	}

	/**
	 * Adds the memory object to a given secondo object containing the nested
	 * list representation.
	 * 
	 * @param secondoObject
	 *            the secondo object containing the nested list
	 * @throws LoadFromExplorerException
	 * @throws MemoryException
	 */
	public void addMemoryObject(SecondoObject secondoObject) throws LoadFromExplorerException,
			MemoryException {
		try {
			MemoryRelation relationObject = ObjectConverter.getInstance().convertListToObject(
					secondoObject.toListExpr());
			secondoObject.setMemoryObject(relationObject);
			secondoObject.setName(secondoObject.getName() + " [++]");
		} catch (ConversionException e) {
			throw new LoadFromExplorerException("-> Error when loading object, caused by:\n"
					+ e.getMessage());
		}
	}

	/**
	 * Extracts all object names from nested list of query 'list objects'.
	 * 
	 * @param queryResultListObjects
	 *            the nested list to extract from
	 * @return the list of all object names
	 * @throws LoadDatabaseException
	 */
	public List<String> getObjectList(ListExpr queryResultListObjects) throws LoadDatabaseException {
		List<String> resultList = new ArrayList<String>();
		try {
			ListExpr tmp = queryResultListObjects;
			tmp = tmp.second().second().rest();
			while (!tmp.isEmpty()) {
				String objectName = tmp.first().second().symbolValue();
				if (!objectName.startsWith("SEC")) {
					resultList.add(objectName);
				}
				tmp = tmp.rest();
			}
		} catch (Throwable e) {
			throw new LoadDatabaseException("-> Could not extract object names from query.");
		}
		if (resultList.isEmpty()) {
			throw new LoadDatabaseException("-> Database does not contain any objects.");
		}
		return resultList;
	}

	/**
	 * Executes 'query object_name' for all objects that are passed via a list
	 * and loads the result objects to MMDB.
	 * 
	 * @param objects
	 *            the object names to load
	 * @param objectList
	 *            the object list where loaded objects are added
	 * @param commandPanel
	 *            the command panel which does the calls
	 * @return the list of failures
	 * @throws LoadDatabaseException
	 * @throws MemoryException
	 */
	public List<String> loadAllObjects(List<String> objects, CommandPanel commandPanel,
			ObjectList objectList) throws LoadDatabaseException, MemoryException {
		List<String> failures = new ArrayList<String>();
		for (String objectName : objects) {
			try {
				String command = "query " + objectName;
				ListExpr queryResult = executeRemoteCommand(command, commandPanel);
				SecondoObject object = createSecondoObject(queryResult, command);
				objectList.addEntry(object);
			} catch (LoadException e) {
				failures.add(objectName);
			}
		}
		if (objects.size() == failures.size()) {
			throw new LoadDatabaseException("-> Could not load any object from database.");
		}
		return failures;
	}

}
