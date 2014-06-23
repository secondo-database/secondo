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
*  This class contains the dialog to change the optimizer settings.
*  
*  @author Kristina Steiger
*  
**/
public class OptimizerSettingsDialog{
	
	private DialogBox optimizerDialogBox = new DialogBox();
	private FlowPanel optimizerDialogContents = new FlowPanel();
	private Button closeOptimizerButton = new Button("Close");
	private String headline = "Enter Optimizer Connection Data:";	
	private String hostLabel = "Host: ";
    private String portLabel = "Port: ";
    private TextBox host = new TextBox();
    private TextBox port = new TextBox();
    private Button savebutton = new Button("Save");
    private FlexTable layout = new FlexTable();
    private DecoratorPanel decPanel = new DecoratorPanel();
	
	public OptimizerSettingsDialog(){
		
		optimizerDialogBox.setText("Change Settings for Optimizer");

	    // Create a table to layout the content
	    optimizerDialogContents.getElement().getStyle().setPadding(5, Unit.PX);
	    optimizerDialogBox.setWidget(optimizerDialogContents);
	    
	    layout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = layout.getFlexCellFormatter();

        // Add a title to the form
        layout.setHTML(0, 0, this.headline);
        cellFormatter.setColSpan(0, 0, 2);
        cellFormatter.setHorizontalAlignment(
            0, 0, HasHorizontalAlignment.ALIGN_CENTER);

        // Add username and password fields
        host.setWidth("150px");
        port.setWidth("150px");
        layout.setHTML(1, 0, this.hostLabel);
        layout.setWidget(1, 1, host);
        layout.setHTML(2, 0, portLabel);
        layout.setWidget(2, 1, port);
        
        //Add the loginbutton to the form
        layout.setWidget(3, 0, savebutton);
        cellFormatter.setColSpan(3, 0, 2);
        cellFormatter.setHorizontalAlignment(3, 0, HasHorizontalAlignment.ALIGN_CENTER);
        
        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(layout);
        
        optimizerDialogContents.add(decPanel);     

	    // Add a close button at the bottom of the dialog
	    closeOptimizerButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  optimizerDialogBox.hide();}
	        });
	    optimizerDialogContents.add(closeOptimizerButton);		
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
	public DialogBox getOptimizerDialogBox() {
		return optimizerDialogBox;
	}

	/**Returns the save button of the dialog
	 * 
	 * @return The save button of the dialog
	 * */
	public Button getSavebutton() {
		return savebutton;
	}
}
