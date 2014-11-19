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

package com.secondo.webgui.client.mainview;

import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;

/**
*  This class contains the information about DB settings.
*  
*  @author Irina Russkaya
*  
**/
public class DBSettingsDialog{
	
	private DialogBox dbDialogBox = new DialogBox();
	private FlowPanel dbDialogContents = new FlowPanel();
	private Button closeButton = new Button("Close");
	private String headline = "Database Connection:";	
	private String hostLabel = "Host: ";
    private String portLabel = "Port: ";
    private String dbLabel = "DB: ";
    private TextBox host = new TextBox();
    private TextBox port = new TextBox();
    private TextBox db = new TextBox();   
    private FlexTable layout = new FlexTable();
    private DecoratorPanel decPanel = new DecoratorPanel();
	
	public DBSettingsDialog(){
		
		dbDialogBox.setText("Settings of DB Connection");

	    // Create a table to layout the content
	    dbDialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    dbDialogBox.setWidget(dbDialogContents);
	    
	    layout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = layout.getFlexCellFormatter();

        // Add a title to the form
        layout.setHTML(0, 0, this.headline);
        cellFormatter.setColSpan(0, 0, 2);
        cellFormatter.setHorizontalAlignment(
            0, 0, HasHorizontalAlignment.ALIGN_CENTER);

        // Add username and password fields
        host.setWidth("150px");
        host.setEnabled(false);
        port.setWidth("150px");
        port.setEnabled(false);
        db.setWidth("150px");
        db.setEnabled(false);
        layout.setHTML(1, 0, this.hostLabel);
        layout.setWidget(1, 1, host);
        layout.setHTML(2, 0, portLabel);
        layout.setWidget(2, 1, port);
        layout.setHTML(3, 0, dbLabel);
        layout.setWidget(3, 1, db);        
       
        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(layout);
        
        dbDialogContents.add(decPanel);     

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  dbDialogBox.hide();}
	        });
	    closeButton.setStyleName("right-floated-button");
	    closeButton.getElement().setAttribute("margin-top", "3px");
	    dbDialogContents.add(closeButton);		
	}

	/**Returns the text box for the host
	 * 
	 * @return The textbox for the host
	 * */
	public TextBox getHost() {
		return host;
	}

	/**Returns the text box for the port
	 * 
	 * @return The textbox for the port
	 * */
	public TextBox getPort() {
		return port;
	}

	/**Returns the optimizer dialog box
	 * 
	 * @return The optimizer dialog box
	 * */
	public DialogBox getDbDialogBox() {
		return dbDialogBox;
	}

	/**
	 * @return the db
	 */
	public TextBox getDb() {
		return db;
	}


}
