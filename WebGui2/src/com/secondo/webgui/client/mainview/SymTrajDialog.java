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
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.ScrollPanel;

/**
*  This class contains the dialog for the optimizer syntax in the help menu.
*  
*  @author Irina Russkaya
*  
**/
public class SymTrajDialog {
	
	private DialogBox helpDialogBox = new DialogBox();
    private FlowPanel dialogContents = new FlowPanel();
    private ScrollPanel scrollContent = new ScrollPanel();
	private HTML symTrajInfo;
	private Button closeButton = new Button("Close");
	
	public SymTrajDialog(){
		
		helpDialogBox.setText("Info about Symbolic Trajectory");

	    // Create a table to layout the content
	    dialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    helpDialogBox.setWidget(dialogContents);
		
		symTrajInfo = new HTML("<h3>Information about the Symbolic Trajectory</h3>" +
				"A symbolic trajectory is a sequence of temporally annotated labels each of "+
				"which is a semantically meaningful description, e.g., a street or city name, an "+
				"activity, or a means of transportation.<p>" +
				"Inside Secondo a symbolic trajectory is called a moving label, since similar data types (moving "+
				"point, moving real, etc.) are supported."+
				"<h3>Sample Symbolic Trajectory</h3><p>" +
				"< ( (2013-01-17-9:02:30, 2013-01-17-9:05:51, TRUE, FALSE), \"Queen Anne St\"), <br>" +
				"( (2013-01-17-9:05:51, 2013-01-17-9:10:16, TRUE, FALSE), \"Wimpole St\"),<br>" +
				"... <br>" +
				"( (2013-01-17-9:18:44, 2013-01-17-9:20:10, TRUE, FALSE), \"Queen Anne St\") ><p>" +
				"Each line of the quoted moving label corresponds to a so-called unit label, i.e., "+
				"a combination of a time interval and a description. The boolean expressions "+
				"indicate whether the start and end instant belong to the interval, respectively.");
		
		// Add the text to the dialog
	    scrollContent.add(symTrajInfo);
	    dialogContents.add(scrollContent);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            helpDialogBox.hide();}
	        });
	    closeButton.setStyleName("right-floated-button");
	    dialogContents.add(closeButton);
	}
	
	/**Returns the dialog box containing the optimizer info text
	 * 
	 * @return The dialog box containing the optimizer info text
	 * */
	public DialogBox getHelpDialogBox() {
		return helpDialogBox;
	}
}
