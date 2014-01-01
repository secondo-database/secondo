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
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;

/**
*  This class represents the statusbar element of the application, which contains current status information.
*  
*  @author Kristina Steiger
*  
**/
public class StatusBar extends Composite{

	/**The main panel of the statusbar*/
	private HorizontalPanel hPanel = new HorizontalPanel();
	
	/**The labelbox containing all labels and text*/
	private HorizontalPanel labelBox = new HorizontalPanel();
	
	//label and text of current status information
	private HTML secondoServerLabel = new HTML("<b>Secondo-URL : &nbsp;</b> ");
	private Label secondoServer = new Label("no server connection");
	private HTML userNameLabel = new HTML("<b>|&nbsp;&nbsp;User : &nbsp;</b>");
	private Label userName = new Label("no user");
	private HTML databaseLabel = new HTML("<b>|&nbsp;&nbsp;DB : &nbsp;</b>");
	private Label openDatabase = new Label("no database open");
	private HTML optimizerLabel = new HTML("<b>|&nbsp;&nbsp;Optimizer-URL : &nbsp;</b>");
	private Label optimizer = new Label("no optimizer connected");	
	private HTML optimizerStatusLabel = new HTML("<b>|&nbsp;&nbsp;Status : &nbsp;</b>");
	private Image onIcon = new Image("resources/images/bullet-green.png");
	private Image offIcon = new Image("resources/images/bullet-red.png");
	
	/**Width of the statusbar view*/
    private int width=Window.getClientWidth();
	
	public StatusBar(){
		
		hPanel.setHeight("28px");
		hPanel.getElement().setClassName("statusbar");
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
	    hPanel.getElement().getStyle().setFontSize(8, Unit.PX);
	    hPanel.setWidth(width-70 + "px");
	    
	    //configure labels and text
	    secondoServerLabel.setSize("95px", "20px");
	    secondoServerLabel.getElement().getStyle().setFontSize(11, Unit.PX);
	    secondoServerLabel.getElement().getStyle().setMarginTop(3, Unit.PX);
	    secondoServer.setSize("170px", "20px");
	    secondoServer.getElement().getStyle().setFontSize(11, Unit.PX);
	    secondoServer.getElement().getStyle().setMarginTop(3, Unit.PX);
	    userNameLabel.setSize("60px", "20px");
	    userNameLabel.getElement().getStyle().setFontSize(11, Unit.PX);
	    userNameLabel.getElement().getStyle().setMarginTop(3, Unit.PX);
	    userName.setSize("80px", "20px");
	    userName.getElement().getStyle().setFontSize(11, Unit.PX);
	    userName.getElement().getStyle().setMarginTop(3, Unit.PX);
	    databaseLabel.setSize("50px", "20px");
	    databaseLabel.getElement().getStyle().setFontSize(11, Unit.PX);
	    databaseLabel.getElement().getStyle().setMarginTop(3, Unit.PX);
	    openDatabase.setSize("100px", "20px");
	    openDatabase.getElement().getStyle().setFontSize(11, Unit.PX);
	    openDatabase.getElement().getStyle().setMarginTop(3, Unit.PX);
	    openDatabase.getElement().getStyle().setProperty("textAlign", "left");
	    optimizerLabel.setSize("120px", "20px");
	    optimizerLabel.getElement().getStyle().setFontSize(11, Unit.PX);
	    optimizerLabel.getElement().getStyle().setMarginTop(3, Unit.PX);
	    optimizer.setSize("170px", "20px");
	    optimizer.getElement().getStyle().setFontSize(11, Unit.PX);
	    optimizer.getElement().getStyle().setMarginTop(3, Unit.PX);
	    optimizerStatusLabel.setSize("70px", "20px");
	    optimizerStatusLabel.getElement().getStyle().setFontSize(11, Unit.PX);
	    optimizerStatusLabel.getElement().getStyle().setMarginTop(3, Unit.PX);
	    onIcon.setSize("20px", "20px");
	    onIcon.getElement().getStyle().setMarginTop(1, Unit.PX);
	    offIcon.setSize("20px", "20px");
	    offIcon.getElement().getStyle().setMarginTop(1, Unit.PX);
	    
	    //add labels and text to labelbox
	    labelBox.add(secondoServerLabel);
	    labelBox.add(secondoServer);
	    labelBox.add(userNameLabel);
	    labelBox.add(userName);
	    labelBox.add(databaseLabel);
	    labelBox.add(openDatabase);
	    labelBox.add(optimizerLabel);
	    labelBox.add(optimizer);
	    labelBox.add(optimizerStatusLabel);
	    labelBox.add(offIcon);
	    labelBox.setHeight("26px");
	    
	    hPanel.add(labelBox);
	}
	
	/**On resizing of the browser window the width of the main panel is readjusted
	 * 
	 * @param width The new width of the main panel
	 * */
	public void resizeWidth(int width){
		if(width > 1000){
			hPanel.setWidth(width-70 + "px");
		}		
		else{
			hPanel.setWidth(1000-70 + "px");
		}
	}

	/**Returns the main panel of the statusbuar
	 * 
	 * @return The main panel of the statusbar
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}
	
	/**Returns the labelbox panel of the statusbuar
	 * 
	 * @return The labelbox panel of the statusbar
	 * */
	public HorizontalPanel getLabelBox() {
		return labelBox;
	}

	/**Returns the label for the server URL of the statusbuar
	 * 
	 * @return The label for the server URL of the statusbar
	 * */
	public Label getSecondoServer() {
		return secondoServer;
	}

	/**Returns the label for the username of the statusbuar
	 * 
	 * @return The label for the username of the statusbar
	 * */
	public Label getUserName() {
		return userName;
	}

	/**Returns the label for the open database of the statusbuar
	 * 
	 * @return The label for the open database of the statusbar
	 * */
	public Label getOpenDatabase() {
		return openDatabase;
	}

	/**Returns the label for the optimizer of the statusbuar
	 * 
	 * @return The label for the optimizer of the statusbar
	 * */
	public Label getOptimizer() {
		return optimizer;
	}

	/**Returns the image for the optimizer on status
	 * 
	 * @return The image for the optimizer on status
	 * */
	public Image getOnIcon() {
		return onIcon;
	}

	/**Returns the image for the optimizer off status
	 * 
	 * @return The image for the optimizer off status
	 * */
	public Image getOffIcon() {
		return offIcon;
	}
}