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

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;

/**
*  This class represents the sidebar element of the application, which contains buttons to hide and show elements in the main view.
*  
*  @author Kristina Steiger
*  
**/
public class SideBar extends Composite{
	
	/**The main panel of the sidebar*/
	private FlowPanel sidebar = new FlowPanel();
	
	//the buttons in the sidebar, from each button one to show and one to hide the element
	private Button showTerminalButton = new Button("<img src='resources/images/terminal-plus.png' height='30px' width='30px'/>");
	private Button hideTerminalButton = new Button("<img src='resources/images/terminal-minus.png' height='30px' width='30px'/>");
	private Button showTextButton = new Button("<img src='resources/images/text-plus.png' height='30px' width='30px'/>");
	private Button hideTextButton = new Button("<img src='resources/images/text-minus.png' height='30px' width='30px'/>");
	private Button showMapButton = new Button("<img src='resources/images/map-plus.png' height='30px' width='30px'/>");
	private Button hideMapButton = new Button("<img src='resources/images/map-minus.png' height='30px' width='30px'/>");
	private Button showRawdataButton = new Button("<img src='resources/images/binary-plus-icon.png' height='30px' width='30px'/>");
	private Button hideRawdataButton = new Button("<img src='resources/images/binary-minus-icon.png' height='30px' width='30px'/>");
	
	public SideBar(){
		
		//configure buttons
		sidebar.add(hideTerminalButton);
		hideTerminalButton.setTitle("Hide Command Panel");
		hideTerminalButton.setSize("50px", "50px");
		showTerminalButton.setTitle("Show Command Panel");
		showTerminalButton.setSize("50px", "50px");
		
		sidebar.add(showTextButton);
		showTextButton.setTitle("Show Text Panel");
		showTextButton.setSize("50px", "50px");
		hideTextButton.setTitle("Hide Text Panel");
		hideTextButton.setSize("50px", "50px");
		
		sidebar.add(showMapButton);
		showMapButton.setTitle("Show Map Background");
		showMapButton.setSize("50px", "50px");
		hideMapButton.setTitle("Hide Map Background");
		hideMapButton.setSize("50px", "50px");

		sidebar.add(showRawdataButton);
		showRawdataButton.setTitle("Show Raw Data");
		showRawdataButton.setSize("50px", "50px");
		hideRawdataButton.setTitle("Hide Raw Data");
		hideRawdataButton.setSize("50px", "50px");
		
		//configure main panel
		sidebar.setWidth("50px");
		sidebar.getElement().setClassName("sidebar");	
	}
	
	/**On resizing of the browser window the height of the main panel is readjusted
	 * 
	 * @param height The new height of the main panel
	 * */
	public void resizeHeight(int height){
		if(height > 650){
			sidebar.setHeight(height-60 + "px");
		}	
		else{
			sidebar.setHeight(650-60 + "px");
		}
	}

	/**Returns the main panel of the sidebar
	 * 
	 * @return The main panel of the sidebar
	 * */
	public FlowPanel getSidebar() {
		return sidebar;
	}

	/**Returns the show terminal button
	 * 
	 * @return The show terminal button
	 * */
	public Button getShowTerminalButton() {
		return showTerminalButton;
	}

	/**Returns the hide terminal button
	 * 
	 * @return The hide terminal button
	 * */
	public Button getHideTerminalButton() {
		return hideTerminalButton;
	}

	/**Returns the show map button
	 * 
	 * @return The show map button
	 * */
	public Button getShowMapButton() {
		return showMapButton;
	}

	/**Returns the hide map button
	 * 
	 * @return The hide map button
	 * */
	public Button getHideMapButton() {
		return hideMapButton;
	}

	/**Returns the show text button
	 * 
	 * @return The show text button
	 * */
	public Button getShowTextButton() {
		return showTextButton;
	}

	/**Returns the hide text button
	 * 
	 * @return The hide text button
	 * */
	public Button getHideTextButton() {
		return hideTextButton;
	}
	
	/**Returns the show raw data view button
	 * 
	 * @return The show raw data view button
	 * */
	public Button getShowRawdataButton() {
		return showRawdataButton;
	}

	/**Returns the hide raw data view button
	 * 
	 * @return The hide raw data view button
	 * */
	public Button getHideRawdataButton() {
		return hideRawdataButton;
	}
}
