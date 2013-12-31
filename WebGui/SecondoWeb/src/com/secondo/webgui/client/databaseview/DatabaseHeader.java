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

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.Image;

/**
 * This class represents the header of the database view.
 * 
 * @author Kristina Steiger
 */
public class DatabaseHeader extends Composite{
	
	/**The main panel of the header*/
	private FlowPanel headerPanel = new FlowPanel();
	
	/**The secondo logo image*/
	private Image logo = new Image("resources/images/secondo-logo.gif");
	
	/**The headline below the logo*/
	private HTML secondoHeadline = new HTML("<h3>An Extensible Database System </h3>");
	
	public DatabaseHeader(){
		
		logo.setWidth("300px");
		headerPanel.add(logo);
		headerPanel.add(secondoHeadline);		
	}

	/**Returns the main panel of the header
	 * 
	 * @return The main panel of the header
	 * */
	public FlowPanel getHeaderPanel() {
		return headerPanel;
	}
}
