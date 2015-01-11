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

package mmdb.gui;

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ScrollPaneConstants;

/**
 * This class represents the help window that opens if a user clicks on "MMDB ->
 * Help menu item."
 *
 * @author Alexander Castor
 */
public class HelpWindow extends JFrame {

	private static final long serialVersionUID = -7418050523768935855L;

	/**
	 * The text area where the message is displayed.
	 */
	private JTextArea textArea;

	/**
	 * The actual message to be displayed.
	 */
	private String message;

	/**
	 * Row count to determine the window's height.
	 */
	private final int rows = 30;

	/**
	 * Row count to determine the windows's width.
	 */
	private final int columns = 50;

	/**
	 * Creates a new HelpWindow instance.
	 */
	public HelpWindow() {
		super();
		initializeText();
		buildWindow();
	}

	/**
	 * Assembles the window's components.
	 */
	private void buildWindow() {
		textArea = new JTextArea(message, rows, columns);
		textArea.setWrapStyleWord(true);
		textArea.setLineWrap(true);
		textArea.setEditable(false);
		textArea.setFocusable(false);
		textArea.setOpaque(false);
		JScrollPane scrollPane = new JScrollPane(textArea,
				ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
				ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
		this.add(scrollPane);
		this.setTitle("MMDB HELP");
		this.pack();
		this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		this.setLocationRelativeTo(null);
	}

	/**
	 * Sets the message text.
	 */
	private void initializeText() {
		message = "THE MMDB MENU ALLOWS LOADING RELATIONAL OBJECTS FROM A SECONDO KERNEL INTO A CLIENT-SIDE "
				+ "MAIN MEMORY DATABASE, EXECUTING QUERIES ON THESE MEMORY OBJECTS AND CONVERTING THE RESULT "
				+ "RELATIONS TO NESTED LISTS IN ORDER TO DISPLAY THEM WITH VIEWERS.\n\n"
				+ "\n\n<<LOAD OBJECT FROM QUERY>>\n"
				+ "Relational objects can be loaded by executing queries on a Secondo kernel. Just like in "
				+ "standard GUI mode, you can enter a query in the command panel, but instead of pressing enter "
				+ "a click on this menu item loads the retrieved object to the MMDB and not passes it on to a "
				+ "viewer. Keep in mind that the query result needs to be a relation object and the query has "
				+ "to end with a semicolon. If an object is also available in the MMDB, there is a '[++]' after "
				+ "its name. If it is only available in the MMDB (i.e. there is no nested list representation) "
				+ "the name will be followed by '[+]'."
				+ "\n\n-----------------------\n\n<<LOAD OBJECT FROM EXPLORER>>\n"
				+ "Objects that are already available in the GUI and therefore visible in the explorer, can be "
				+ "loaded to the MMDB by selecting the corresponding object (only single selection is possible) "
				+ "and afterwards clicking on this menu item."
				+ "\n\n-----------------------\n\n<<LOAD OBJECTS FROM DATABASE>>\n"
				+ "It is possible to load all objects from the currently opened database to the MMDB. The backend "
				+ "will automatically execute a 'list objects' query and subsequently execute 'query object_name' "
				+ "for all retrieved objects from the first query. After this operation is finished, a window "
				+ "listing the failures (objects that could not be loaded) will be displayed. If you want detailed "
				+ "error messages, you need to load the corresponding single object via the above mentioned options."
				+ "\n\n-----------------------\n\n<<CONVERT SELECTED OBJECT TO NL>>\n"
				+ "Objects that are only available in the MMDB (the ones followed by '[+]') must be converted in "
				+ "nested list representation to display them with viewers. Just select the object in the explorer "
				+ "and click on this menu item."
				+ "\n\n-----------------------\n\n<<CONVERT ALL OBJECTS TO NL>>\n"
				+ "Instead of converting single objects in nested list representation, you can also convert all "
				+ "currently available objects from the explorer at one go. After this operation is finished, a window "
				+ "listing the failures (objects that could not be converted) will be displayed. If you want detailed "
				+ "error messages, you need to convert the corresponding single object via the above mentioned option."
				+ "\n\n-----------------------\n\n<<GENERATE INDEX>>\n"
				+ "It is possible to create indices on certain attributes. These will be used automatically during "
				+ "query executing in order to accelerate processing time. In the index creation dialog, first select "
				+ "a relation, afterwards an attribute which shall be used for indexing and finally the type of index "
				+ "you want to create. Only attributes that are suited for indexing will be displayed. It is not "
				+ "possible to create several indices on one attribute."
				+ "\n\n-----------------------\n\n<<EXECUTE QUERY>>\n"
				+ "There are five top level query operations available:"
				+ "\n1) SELECTION: Select a tuple subset depending on a condition."
				+ "\n2) PROJECTION: Select an attribute subset from the relation's tuples."
				+ "\n3) EXTENSION: Add new attributes to the relation's tuples via an operation."
				+ "\n4) UNION: Merge the tuples of two relations. Attribute sets must be identical."
				+ "\n5) JOIN: Merge attributes of two different relations depending on a condition."
				+ "\n6) AGGREGATION: Calculate aggregations of a certain attribute for all tuples of a relation."
				+ "\n\n-----------------------\n\n<<MANAGE MEMORY>>\n"
				+ "Since all relations are stored in main memory it might occur that the JVM is running out of "
				+ "memory which means that the application will crash. To prevent these OutOfMemoryErrors memory "
				+ "is permanently monitored. If there is an impending overflow you will be given the chance to "
				+ "remove objects that are not needed anymore to free memory. This can also be done any time "
				+ "by selecting this menu item. Besides the dialog runs an asynchronous thread which performs "
				+ "garbage collection once at startup and at specified time intervals (default = 60 sec)."
				+ "\n\n-----------------------\n\n<<SUPPRTED TYPES>>\n"
				+ "The client-side MMDB only supports certain attribute types. This window will display a list of all "
				+ "types that are supported.";
	}

}
