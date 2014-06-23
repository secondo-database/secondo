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
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;

/**
*  This class represents the header of the main view.
*  
*  @author Kristina Steiger
*  
**/
public class Header extends Composite{
	
	/**The main panel of the header*/
	private HorizontalPanel hPanel = new HorizontalPanel();
	
	/**The Secondo logo image*/
	private Image logo = new Image("resources/images/secondo-logo.gif");
	
	/**The panel for buttons*/
	private HorizontalPanel buttonPanel = new HorizontalPanel();
	
	/**The close database button*/
	private Button closedatabaseButton = new Button("<img src='resources/images/close-database.png' height='30px' width='30px'/>");
	
	/**The logout button*/
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='30px' width='30px'/>");
	
	/**Width of the header*/
    private int width=Window.getClientWidth();
	
	public Header(){

		int windowWidth = Window.getClientWidth();
		
		logo.getElement().getStyle().setMarginLeft(30, Unit.PX);
		logo.setWidth("250px");
		hPanel.add(logo);
		hPanel.setWidth(width + "px");
		buttonPanel.getElement().getStyle().setMarginLeft(width-430, Unit.PX);
		
		//configure buttons
		closedatabaseButton.getElement().setClassName("closedatabasebutton");
		closedatabaseButton.getElement().getStyle().setBackgroundColor("white");
		closedatabaseButton.setWidth("40px");
		closedatabaseButton.setTitle("Close Database");
		logoutButton.getElement().setClassName("logoutbutton");
		logoutButton.setWidth("40px");
		logoutButton.setTitle("Logout");
		
		buttonPanel.setWidth("100px");
		buttonPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		buttonPanel.getElement().getStyle().setMarginLeft(windowWidth-430, Unit.PX);
		buttonPanel.add(closedatabaseButton);
		buttonPanel.add(logoutButton);
		hPanel.add(buttonPanel);

		hPanel.getElement().setClassName("mainheader");	
		hPanel.setWidth(windowWidth + "px");
		hPanel.setHeight("50px");
	}
	
	/**On resizing of the browser window the width of the main panel and button panel is readjusted
	 * 
	 * @param width The new width of the main panel and the button panel
	 * */
	public void resizeWidth(int width){
		hPanel.setWidth(width + "px");
		if(width > 1000){
			buttonPanel.getElement().getStyle().setMarginLeft(width-430, Unit.PX);
		}		
		else{
			buttonPanel.getElement().getStyle().setMarginLeft(1000-430, Unit.PX);
		}
	}

	/**Returns the main panel of the header
	 * 
	 * @return The main panel of the header
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}
	
	/**Returns the close database button
	 * 
	 * @return The close database button
	 * */
	public Button getClosedatabaseButton() {
		return closedatabaseButton;
	}

	/**Returns the logout button
	 * 
	 * @return The logout button
	 * */
	public Button getLogoutButton() {
		return logoutButton;
	}
}
