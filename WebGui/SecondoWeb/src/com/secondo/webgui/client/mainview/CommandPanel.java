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

import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.secondo.webgui.client.controller.BasicCommandBuilder;

/**
 * This class represents the commandpanel of the application, containing a menubar and a textarea to enter commands.
 *  
 * @author Kristina Steiger
 */
public class CommandPanel extends Composite {

	/**The main panel of the commandpanel*/
	private VerticalPanel commandPanel = new VerticalPanel();
	
	/**The menubar object*/
	private MenuCommandPanel menubarCP = new MenuCommandPanel();
	
	/**The textarea to enter commands*/
	private TextArea textArea = new TextArea();
	
	/**Object which creates commands for all example commands in the menubar*/
	private BasicCommandBuilder basicCommandBuilder;
	
	/**Width of the commandpanel view*/
    private int width=Window.getClientWidth();

	public CommandPanel() {

		textArea.setVisibleLines(5);
		textArea.setHeight("200px");
		textArea.setWidth(width-70 + "px");
		
		textArea.getElement().setClassName("commandpanel");
		
		commandPanel.add(menubarCP.gethPanel());
		commandPanel.add(textArea);
		commandPanel.getElement().setId("command");
		commandPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		
	    //initialize the commandbuilder with this commandpanel instance
	    basicCommandBuilder = new BasicCommandBuilder(this);

	}
	
	/**On resizing of the browser window the width of the textarea is readjusted
	 * 
	 * @param width The new width of the textarea
	 * */
	public void resizeWidth(int width){
		if(width > 1000){
			textArea.setWidth(width-70 + "px");
		}		
		else{
			textArea.setWidth(1000-70 + "px");
		}
	}

	/**Returns the main panel of the commandpanel
	 * 
	 * @return The main panel of the commandpanel
	 * */
	public VerticalPanel getCommandPanel() {
		return commandPanel;
	}

	/**Returns the textarea of the commandpanel
	 * 
	 * @return The textarea of the commandpanel
	 * */
	public TextArea getTextArea() {
		return textArea;
	}

	/**Returns the menubar of the commandpanel
	 * 
	 * @return The menubar of the commandpanel
	 * */
	public MenuCommandPanel getMenubarCP() {
		return menubarCP;
	}
}
