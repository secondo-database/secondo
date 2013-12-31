//This file is part of SECONDO.

//Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ListBox;

/**
 * This class represents the database view of the application and includes all elements that are displayed in the
 * database view, which is used to choose a database with which the application can be opened.
 *  
 * @author Kristina Steiger
 */
public class DatabaseView extends Composite{
	
	/**Main panel of the database view*/
    private HorizontalPanel hPanel = new HorizontalPanel();
    
    /**Header object*/
    private DatabaseHeader databaseHeader = new DatabaseHeader();
    
    /**Footer object*/
    private DatabaseFooter databaseFooter = new DatabaseFooter();

	 /**List box with single selection mode for databases*/
    private ListBox multiBox = new ListBox();
    
    /**Panel for listbox and buttons*/
    private FlowPanel contentPanel = new FlowPanel();
	
    /**Panel for buttons*/
	private HorizontalPanel buttonBox = new HorizontalPanel();
    private Button openDatabaseButton = new Button("Open Database");
	private Button importDatabaseButton = new Button("Import Database");
	
	public DatabaseView(){
		
		hPanel.setSpacing(20);

	    multiBox.ensureDebugId("ListBox-multiBox");
	    multiBox.setWidth("20em");
	    multiBox.setVisibleItemCount(15);
	    
	    buttonBox.add(openDatabaseButton);
	    buttonBox.add(importDatabaseButton);
	    buttonBox.setSpacing(10);

	    contentPanel.add(new HTML("Select a Database:"));
	    contentPanel.add(multiBox);
	    contentPanel.add(buttonBox);
    
	    hPanel.add(contentPanel);
	}
	
	/**Adds a database to the database list
	 * 
	 * @param item The database item to be added to the list
	 * */
	public void addDatabase(String item){
		multiBox.addItem(item);
	}
	
	/**Removes an entry of the database list
	 * 
	 * @param index The index of the databaseentry to be removed from the list
	 * */
	public void removeDatabaseEntry(int index){
		this.multiBox.removeItem(index);
	}

	/**Returns the open database button
	 * 
	 * @return The open database button
	 * */
	public Button getOpenDatabaseButton() {
		return openDatabaseButton;
	}	

	/**Returns the import database button
	 * 
	 * @return The import database button
	 * */
	public Button getImportDatabaseButton() {
		return importDatabaseButton;
	}

	/**Returns the listbox for databases
	 * 
	 * @return The listbox for databases
	 * */
	public ListBox getMultiBox() {
		return multiBox;
	}

	/**Returns the footer of the database view
	 * 
	 * @return The footer of the database view
	 * */
	public DatabaseFooter getDatabaseFooter() {
		return databaseFooter;
	}

	/**Returns the header of the database view
	 * 
	 * @return The header of the database view
	 * */
	public DatabaseHeader getDatabaseHeader() {
		return databaseHeader;
	}

	/**Returns the main panel of the database view
	 * 
	 * @return The main panel of the database view
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}
}
