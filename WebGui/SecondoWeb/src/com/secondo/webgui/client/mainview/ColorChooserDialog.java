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

import java.util.ArrayList;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.RadioButton;

/**
 * This class contains a dialog box with a color chooser to change the color of graphical elements displayed in the view.
 *  
 * @author Kristina Steiger
 */
public class ColorChooserDialog {

	private DialogBox dialogBox = new DialogBox();
    private FlowPanel dialogContent = new FlowPanel();
	private Button closeButton = new Button("Close");
	private Button saveButton = new Button("Save");
	private HorizontalPanel buttonPanel = new HorizontalPanel();
    private HorizontalPanel boxPanel = new HorizontalPanel();
	private ListBox queryBox = new ListBox(true);
	private FlowPanel colorBox = new FlowPanel();
	private HorizontalPanel yellowBox = new HorizontalPanel();
	private FlowPanel yellow = new FlowPanel();
	private RadioButton yellowButton = new RadioButton("color");
	private HorizontalPanel orangeBox = new HorizontalPanel();
	private FlowPanel orange = new FlowPanel();
	private RadioButton orangeButton = new RadioButton("color");
	private HorizontalPanel redBox = new HorizontalPanel();
	private FlowPanel red = new FlowPanel();
	private RadioButton redButton = new RadioButton("color");
	private HorizontalPanel purpleBox = new HorizontalPanel();
	private FlowPanel purple = new FlowPanel();
	private RadioButton purpleButton = new RadioButton("color");
	private HorizontalPanel blueBox = new HorizontalPanel();
	private FlowPanel blue = new FlowPanel();
	private RadioButton blueButton = new RadioButton("color");
	private HorizontalPanel greenBox = new HorizontalPanel();
	private FlowPanel green = new FlowPanel();
	private RadioButton greenButton = new RadioButton("color");
	private HorizontalPanel brownBox = new HorizontalPanel();
	private FlowPanel brown = new FlowPanel();
	private RadioButton brownButton = new RadioButton("color");
	private HorizontalPanel blackBox = new HorizontalPanel();
	private FlowPanel black = new FlowPanel();
	private RadioButton blackButton = new RadioButton("color");
	private ArrayList<RadioButton> buttonList = new ArrayList<RadioButton>();
	
	public ColorChooserDialog(){
		
		dialogBox.setText("Choose color for a query");

	    dialogContent.getElement().getStyle().setPadding(5, Unit.PX);
	    dialogContent.setSize("250px", "250px");
	    dialogBox.setWidget(dialogContent);	    
	    
	    //configure color elements
	    yellow.getElement().getStyle().setBackgroundColor("yellow");
	    yellow.setSize("25px", "15px");
	    yellow.getElement().getStyle().setMargin(5, Unit.PX);
	    yellowBox.add(yellowButton);
	    yellowBox.add(yellow);
	    colorBox.add(yellowBox);
	    buttonList.add(yellowButton);
	    orange.getElement().getStyle().setBackgroundColor("orange");
	    orange.setSize("25px", "15px");
	    orange.getElement().getStyle().setMargin(5, Unit.PX);
	    orangeBox.add(orangeButton);
	    orangeBox.add(orange);
	    colorBox.add(orangeBox);
	    buttonList.add(orangeButton);
	    red.getElement().getStyle().setBackgroundColor("#FF0000");
	    red.setSize("25px", "15px");
	    red.getElement().getStyle().setMargin(5, Unit.PX);
	    redBox.add(redButton);
	    redBox.add(red);
	    colorBox.add(redBox);
	    buttonList.add(redButton);
	    purple.getElement().getStyle().setBackgroundColor("#660066");
	    purple.setSize("25px", "15px");
	    purple.getElement().getStyle().setMargin(5, Unit.PX);
	    purpleBox.add(purpleButton);
	    purpleBox.add(purple);
	    colorBox.add(purpleBox);
	    buttonList.add(purpleButton);
	    blue.getElement().getStyle().setBackgroundColor("#0000FF");
	    blue.getElement().getStyle().setMargin(5, Unit.PX);
	    blue.setSize("25px", "15px");
	    blueBox.add(blueButton);
	    blueBox.add(blue);
	    colorBox.add(blueBox);
	    buttonList.add(blueButton);
	    green.getElement().getStyle().setBackgroundColor("green");
	    green.getElement().getStyle().setMargin(5, Unit.PX);
	    green.setSize("25px", "15px");
	    greenBox.add(greenButton);
	    greenBox.add(green);
	    colorBox.add(greenBox);
	    buttonList.add(greenButton);
	    brown.getElement().getStyle().setBackgroundColor("#660000");
	    brown.getElement().getStyle().setMargin(5, Unit.PX);
	    brown.setSize("25px", "15px");
	    brownBox.add(brownButton);
	    brownBox.add(brown);
	    colorBox.add(brownBox);
	    buttonList.add(brownButton);
	    black.getElement().getStyle().setBackgroundColor("black");
	    black.getElement().getStyle().setMargin(5, Unit.PX);
	    black.setSize("25px", "15px");
	    blackBox.add(blackButton);
	    blackBox.add(black);
	    colorBox.add(blackBox);	    
	    boxPanel.add(colorBox);  
	    buttonList.add(blackButton);
	    
	    queryBox.setWidth("150px");
	    queryBox.setVisibleItemCount(10);
	    queryBox.getElement().getStyle().setMarginLeft(20, Unit.PX);  
	    boxPanel.add(queryBox);

	    // Add a close button at the bottom of the dialog
	    closeButton.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	            dialogBox.hide();}
	        });
	    
	    closeButton.getElement().getStyle().setMarginLeft(20, Unit.PX);
	    buttonPanel.getElement().getStyle().setPadding(10, Unit.PX);
	    buttonPanel.add(saveButton);
	    buttonPanel.add(closeButton);
	    
	    dialogContent.add(boxPanel);
	    dialogContent.add(buttonPanel);		
	}
	

	/**Returns the listbox for querys
	 * 
	 * @return The listbox for querys
	 * */
	public ListBox getQueryBox() {
		return queryBox;
	}

	/**Returns a list with all radiobuttons belonging to the colors
	 * 
	 * @return The list with radiobuttons for colors
	 * */
	public ArrayList<RadioButton> getButtonList() {
		return buttonList;
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
