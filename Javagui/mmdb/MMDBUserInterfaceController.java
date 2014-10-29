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

package mmdb;

import gui.CommandPanel;
import gui.ObjectList;
import gui.SecondoObject;
import gui.idmanager.IDManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.indices.MemoryIndex;
import mmdb.error.convert.ConversionException;
import mmdb.error.index.IndexingException;
import mmdb.error.load.LoadException;
import mmdb.error.memory.MemoryException;
import mmdb.gui.HelpWindow;
import mmdb.gui.IndexDialog;
import mmdb.gui.MMDBMenu;
import mmdb.gui.MMDBMenu.MenuEntry;
import mmdb.gui.MemoryDialog;
import mmdb.gui.QueryDialog;
import mmdb.service.MemoryWatcher;
import mmdb.service.ObjectConverter;
import mmdb.service.ObjectLoader;
import sj.lang.ListExpr;
import tools.Reporter;

/**
 * This class is responsible for controlling all UI elements that need to be
 * integrated into MainWindow for MMDB functionality.
 *
 * @author Alexander Castor
 */
public final class MMDBUserInterfaceController {

	/**
	 * The MMDB menu which is the entry point for all user interaction.
	 */
	private JMenu menu;

	/**
	 * The command panel where users can input queries.
	 */
	private CommandPanel commandPanel;

	/**
	 * The object list from the object explorer which contains all objects.
	 */
	private ObjectList objectList;

	/**
	 * Singleton instance that can be retrieved from other objects.
	 */
	private static MMDBUserInterfaceController instance = new MMDBUserInterfaceController();

	/**
	 * Creates a new instance of the user interface controller and generates the
	 * menu.
	 */
	private MMDBUserInterfaceController() {
		menu = new MMDBMenu();
	}

	/**
	 * Returns the initialized singleton instance.
	 * 
	 * @return the singleton instance
	 */
	public static MMDBUserInterfaceController getInstance() {
		return instance;
	}

	/**
	 * Adds the new MMDB menu to the main menu.
	 * 
	 * @param mainMenu
	 *            the main menu
	 */
	public void addMMDBMenu(JMenuBar mainMenu) {
		mainMenu.add(menu);
	}

	/**
	 * This method is called by MainWindow in order to inject references for
	 * required UI elements that are needed for integration.
	 * 
	 * @param objectList
	 *            the main window's object explorer
	 * @param comPanel
	 *            the main window's command panel
	 */
	public void injectElementsToMMDB(ObjectList objectList, CommandPanel commandPanel) {
		this.objectList = objectList;
		this.commandPanel = commandPanel;
	}

	/**
	 * Dispatches action events of menu items for further processing.
	 * 
	 * @param entry
	 *            The menu entry on which an action was performed.
	 */
	public void dispatchMenuEvent(MenuEntry entry) {
		try {
			switch (entry) {
			case LOAD_QUERY:
				processLoadQueryEvent();
				break;
			case LOAD_EXPLORER:
				processLoadExplorerEvent();
				break;
			case LOAD_DATABASE:
				processLoadDatabaseEvent();
				break;
			case CONVERT_ONE:
				processConvertOneEvent();
				break;
			case CONVERT_ALL:
				processConvertAllEvent();
				break;
			case INDEX:
				processIndexEvent();
				break;
			case QUERY:
				processQueryEvent();
				break;
			case MEMORY:
				processMemoryEvent();
				break;
			case TYPES:
				processTypesEvent();
				break;
			case HELP:
				processHelpEvent();
				break;
			}
		} catch (Throwable e) {
			e.printStackTrace();
			Reporter.showError("An unexpected error in the MMDB module has occured.\nSee console for details.");
		}
	}

	/**
	 * Executes a secondo query from command panel, creates a memory object and
	 * adds a new entry to the object explorer.
	 */
	private void processLoadQueryEvent() {
		String command;
		ListExpr queryResult;
		SecondoObject object;
		try {
			command = ObjectLoader.getInstance().readCommandFromPanel(commandPanel.SystemArea);
			queryResult = ObjectLoader.getInstance().executeRemoteCommand(command, commandPanel);
			object = ObjectLoader.getInstance().createSecondoObject(queryResult, command);
			objectList.addEntry(object);
			commandPanel.SystemArea.append("\nObject successfully loaded to MMDB!");
		} catch (LoadException e) {
			Reporter.reportWarning("Error when loading object from query, caused by:", e, false,
					false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		} finally {
			commandPanel.showPrompt();
		}
	}

	/**
	 * Loads a selected secondo object from the browser into the MMDB.
	 */
	private void processLoadExplorerEvent() {
		SecondoObject object = objectList.getSelectedObject();
		if (object == null) {
			Reporter.showInfo("Please select an object to be loaded.");
			return;
		}
		if (object.getMemoryObject() != null) {
			Reporter.showInfo("Object already contains a memory representation.");
			return;
		}
		try {
			objectList.removeObject(object);
			ObjectLoader.getInstance().addMemoryObject(object);
		} catch (LoadException e) {
			Reporter.reportWarning("Error when loading object from explorer, caused by:", e, false,
					false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		} finally {
			objectList.addEntry(object);
		}
	}

	/**
	 * Loads all objects from the currently opened database into the MMDB.
	 */
	private void processLoadDatabaseEvent() {
		try {
			ListExpr queryResultListObjects = ObjectLoader.getInstance().executeRemoteCommand(
					"list objects", commandPanel);
			List<String> objects = ObjectLoader.getInstance().getObjectList(queryResultListObjects);
			int reply = JOptionPane.showConfirmDialog(null, "There are " + objects.size()
					+ " objects in the current database.\n"
					+ "Do you really want to load all of them?", "Please confirm operation",
					JOptionPane.YES_NO_OPTION);
			if (reply == JOptionPane.YES_OPTION) {
				List<String> failures = ObjectLoader.getInstance().loadAllObjects(objects,
						commandPanel, objectList);
				String message = (objects.size() - failures.size())
						+ " objects successfully loaded to MMDB. See results in object explorer.";
				if (!failures.isEmpty()) {
					message = createExtendedErrorMessage(message, failures);
				}
				Reporter.showInfo(message);
			}
		} catch (LoadException e) {
			Reporter.reportWarning("Error when loading all objects from database, caused by:", e,
					false, false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		}
	}

	/**
	 * Converts a selected secondo object from the browser into nested list
	 * representation.
	 */
	private void processConvertOneEvent() {
		SecondoObject object = objectList.getSelectedObject();
		if (object == null) {
			Reporter.showInfo("Please select an object to be converted.");
			return;
		}
		if (object.toListExpr() != null) {
			Reporter.showInfo("Object already contains a nested list representation.");
			return;
		}
		try {
			objectList.removeObject(object);
			ObjectConverter.getInstance().addNestedListToSecondoObject(object);
		} catch (ConversionException e) {
			Reporter.reportWarning(
					"Error when converting memory object to nested list, caused by:", e, false,
					false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		} finally {
			objectList.addEntry(object);
		}
	}

	/**
	 * Converts all secondo objects from the browser into nested list
	 * representation.
	 */
	private void processConvertAllEvent() {
		try {
			List<SecondoObject> objects = new ArrayList<SecondoObject>();
			for (SecondoObject object : objectList.getAllObjects()) {
				if (object.toListExpr() == null) {
					objects.add(object);
				}
			}
			List<String> failures = ObjectConverter.getInstance().convertAllObjects(objects,
					objectList);
			String message = (objects.size() - failures.size())
					+ " objects successfully converted. See results in object explorer.";
			if (!failures.isEmpty()) {
				message = createExtendedErrorMessage(message, failures);
			}
			Reporter.showInfo(message);
		} catch (ConversionException e) {
			Reporter.reportWarning(
					"Error when converting memory objects to nested lists, caused by:", e, false,
					false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		}
	}

	/**
	 * Opens the user dialog for creating indices.
	 */
	private void processIndexEvent() {
		Map<String, MemoryRelation> relations = getAllMemoryRelations();
		if (relations.isEmpty()) {
			Reporter.showInfo("There are currently no memory relations available.");
			return;
		}
		String[] dialogAnswer = IndexDialog.showDialog(relations, commandPanel);
		if (dialogAnswer[0] == null) {
			return;
		}
		MemoryRelation relation = relations.get(dialogAnswer[0]);
		try {
			relation.createIndex(dialogAnswer[1], dialogAnswer[2]);
			Reporter.showInfo("Index successfully created.");
		} catch (IndexingException e) {
			Reporter.reportWarning("Error when creating index, caused by:", e, false, false, false);
		} catch (MemoryException e) {
			processMemoryException(e);
		}
	}

	/**
	 * Opens the user dialog for executing queries.
	 */
	private void processQueryEvent() {
		Map<String, MemoryRelation> relations = getAllMemoryRelations();
		if (relations.isEmpty()) {
			Reporter.showInfo("There are currently no memory relations available.");
			return;
		}
		try {
			Object[] dialogAnswer = QueryDialog.showDialog(relations, commandPanel);
			if (dialogAnswer[0] == null) {
				return;
			}
			MemoryRelation resultRelation = (MemoryRelation) dialogAnswer[0];
			String objectName = (String) dialogAnswer[1];
			Boolean convertToList = (Boolean) dialogAnswer[2];
			SecondoObject secondoObject = new SecondoObject(IDManager.getNextID());
			secondoObject.setMemoryObject(resultRelation);
			if (convertToList) {
				try {
					ObjectConverter.getInstance().addNestedListToSecondoObject(secondoObject);
					secondoObject.setName(objectName + "; [++]");
				} catch (ConversionException e) {
					Reporter.reportWarning(
							"Error when converting memory object to nested list, caused by:", e,
							false, false, false);
					secondoObject.setName(objectName + "; [+]");
				}
			} else {
				secondoObject.setName(objectName + "; [+]");
			}
			objectList.addEntry(secondoObject);
			Reporter.showInfo("Query successfully executed.\nResult relation contains "
					+ resultRelation.getTuples().size()
					+ " tuple(s).\nSee result in object explorer.");
		} catch (Exception e) {
			Reporter.reportWarning("Error when executing query, caused by:", e, false, false, false);
		}
	}

	/**
	 * Opens the user dialog for managing the memory.
	 */
	private void processMemoryEvent() {
		while (true) {
			if (objectList.getAllObjects().isEmpty()) {
				Reporter.showInfo("There are currently no objects available.");
				break;
			}
			String[][] objectStatistics = MemoryWatcher.getInstance().getObjectStatistics(
					objectList.getAllObjects());
			String[] dialogAnswer = MemoryDialog.showDialog(objectStatistics, commandPanel);
			if (dialogAnswer[1] == null) {
				break;
			}
			SecondoObject object = objectList.getSingleObject(dialogAnswer[0]);
			if (object == null) {
				break;
			}
			switch (dialogAnswer[1]) {
			case ("OBJ"):
				objectList.removeObject(object.getName(), false);
				break;
			case ("NES"):
				removeNestedList(object);
				break;
			case ("REL"):
				removeRelation(object);
				break;
			case ("IDX"):
				removeIndices(object.getMemoryObject());
				break;
			}
		}
	}

	/**
	 * Displays a window showing all supported types.
	 */
	private void processTypesEvent() {
		List<String> types = new ArrayList<String>(MemoryAttribute.getAllTypeNames());
		Collections.sort(types);
		String message = "SUPPORTED TYPES:\n";
		int counter = 0;
		for (String type : types) {
			if (counter < 4) {
				message = message + type + ", ";
				counter++;
			} else {
				message = message + type + "\n";
				counter = 0;
			}
		}
		if (counter == 0) {
			message = message.substring(0, message.lastIndexOf("\n"));
		} else {
			message = message.substring(0, message.lastIndexOf(", "));
		}
		Reporter.showInfo(message);
	}

	/**
	 * Displays the help window.
	 */
	private void processHelpEvent() {
		(new HelpWindow()).setVisible(true);
	}

	/**
	 * Retrieves all memory relations from the object list.
	 * 
	 * @return a list containing all memory relations
	 */
	private Map<String, MemoryRelation> getAllMemoryRelations() {
		Map<String, MemoryRelation> relations = new HashMap<String, MemoryRelation>();
		for (SecondoObject object : objectList.getAllObjects()) {
			String name = object.getName();
			MemoryObject memoryObject = object.getMemoryObject();
			if (memoryObject != null && memoryObject instanceof MemoryRelation) {
				MemoryRelation relation = (MemoryRelation) object.getMemoryObject();
				relations.put(name, relation);
			}
		}
		return relations;
	}

	/**
	 * Creates a extended error message for loading or converting many objects.
	 * 
	 * @param message
	 *            the message to be extended
	 * @param failures
	 *            the failures that occurred
	 * @return the extended message
	 */
	private String createExtendedErrorMessage(String message, List<String> failures) {
		String theMessage = message;
		theMessage += "\nErrors occured for the following objects: ";
		boolean moreThanTen = false;
		int counter = 0;
		for (String object : failures) {
			if (counter == 10) {
				moreThanTen = true;
				break;
			}
			theMessage += "\n-> " + object;
			counter++;
		}
		if (moreThanTen) {
			theMessage += "\n... (" + (failures.size() - 10) + " more errors)";
		}
		return theMessage;
	}

	/**
	 * Removes the memory relation from the given secondo object. If the object
	 * only consists of a memory relation, the whole object is deleted.
	 * 
	 * @param object
	 *            the object whose memory relation shall be removed.
	 */
	private void removeRelation(SecondoObject object) {
		if (object.toListExpr() == null) {
			objectList.removeObject(object.getName(), false);
		} else {
			String newObjectName = object.getName().replace(" [++]", "");
			object.setName(newObjectName);
			object.setMemoryObject(null);
			objectList.updateMarks();
		}
	}

	/**
	 * Removes the nested list from the given secondo object. If the object only
	 * consists of a nested list, the whole object is deleted.
	 * 
	 * @param object
	 *            the object whose nested list shall be removed.
	 */
	private void removeNestedList(SecondoObject object) {
		if (object.getMemoryObject() == null) {
			objectList.removeObject(object.getName(), false);
		} else {
			String newObjectName = object.getName().replace("[++]", "[+]");
			object.setName(newObjectName);
			object.fromList(null);
			objectList.updateMarks();
		}
	}

	/**
	 * Removes all indices from a given memoryObject in case it is a relation.
	 * 
	 * @param memoryObject
	 *            the memoryObject whose indices shall be removed
	 */
	private void removeIndices(MemoryObject memoryObject) {
		if (memoryObject != null && memoryObject instanceof MemoryRelation) {
			MemoryRelation memoryRelation = (MemoryRelation) memoryObject;
			Map<String, MemoryIndex<?>> indices = memoryRelation.getIndices();
			if (indices != null && !indices.isEmpty()) {
				indices.clear();
			}
		}
	}

	/**
	 * Getter for the command panel. Needed to load single objects in the
	 * selection query panel.
	 * 
	 * @return
	 */
	public CommandPanel getCommandPanel() {
		return commandPanel;
	}

	/**
	 * Processes memory exceptions.
	 * 
	 * @param exception
	 *            the memory exception which was thrown
	 */
	public void processMemoryException(MemoryException exception) {
		Reporter.showWarning(exception.getMessage());
		processMemoryEvent();
	}

}