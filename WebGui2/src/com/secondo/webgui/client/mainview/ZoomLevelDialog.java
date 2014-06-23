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
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.RadioButton;
import com.google.gwt.user.client.ui.VerticalPanel;

/**
 * This class contains a dialog box with options to change the zoomlevel of the view.
 *  
 * @author Kristina Steiger
 */
public class ZoomLevelDialog {
	
	private DialogBox dialogBox = new DialogBox();
    private FlowPanel dialogContent = new FlowPanel();
	private Button closeButton = new Button("Close");
	private Button saveButton = new Button("Save");
	private HorizontalPanel buttonPanel = new HorizontalPanel();
    private VerticalPanel boxPanel = new VerticalPanel();
    private RadioButton zoomAll = new RadioButton("zoomLevel", "Zoom to all queries");
    private RadioButton zoomLast = new RadioButton("zoomLevel", "Zoom to last query");

	public ZoomLevelDialog(){
		
		dialogBox.setText("Choose the default zoomlevel");

	    dialogContent.getElement().getStyle().setPadding(5, Unit.PX);
	    dialogContent.setSize("160px", "110px");
	    dialogBox.setWidget(dialogContent);	
	    
	    zoomLast.setValue(true);
	    
	 // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            dialogBox.hide();}
	        });
	    
	    closeButton.getElement().getStyle().setMarginLeft(20, Unit.PX);
	    buttonPanel.getElement().getStyle().setPadding(10, Unit.PX);
	    buttonPanel.add(saveButton);
	    buttonPanel.add(closeButton);	    

	    boxPanel.add(zoomLast);
	    boxPanel.add(zoomAll);
	    boxPanel.getElement().getStyle().setMarginBottom(20, Unit.PX);  
	    
	    dialogContent.add(boxPanel);
	    dialogContent.add(buttonPanel);		
	}
	
	public RadioButton getZoomAll() {
		return zoomAll;
	}

	public RadioButton getZoomLast() {
		return zoomLast;
	}

	/**Returns the save button of the dialog box
	 * 
	 * @return The save Button of the dialog box
	 * */
	public Button getSaveButton() {
		return saveButton;
	}

	/**Returns the dialog box for the color chooser
	 * 
	 * @return the dialog box for the color chooser
	 * */
	public DialogBox getDialogBox() {
		return dialogBox;
	}
}
