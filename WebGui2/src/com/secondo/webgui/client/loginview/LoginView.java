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

package com.secondo.webgui.client.loginview;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.DisclosurePanel;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.PasswordTextBox;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;

/**
 * This class represents the login view of the application and includes all elements that are displayed in the
 * login view, which is used to log into the application.
 *  
 * @author Kristina Steiger
 */
public class LoginView extends Composite{
	
	/**Main panel of the login view*/
    private VerticalPanel mainpanel = new VerticalPanel();
	
	/**The header object*/
	private LoginHeader loginheader = new LoginHeader();
	
	/**The footer object*/
	private LoginFooter loginfooter = new LoginFooter();
	
	/**The secondo logo image*/
	private Image logo = new Image("resources/images/secondo-logo.gif");
	
	/**The headline below the logo*/
	private HTML secondoHeadline = new HTML("<h1>An Extensible Database System </h1>");
	
	/**Decorator panel for the login form*/
    private DecoratorPanel decPanel = new DecoratorPanel();
    
    /**Grid for login form elements*/
    private FlexTable loginLayout = new FlexTable();
	
    //elements in login grid
    private String headline = "Log into Secondo Database";	
	private String usernameLabel = "Username: ";
    private String passwordLabel = "Password: ";
    private String ipLabel = "Server-URL: ";
    private String portLabel = "Port: ";
    private TextBox username = new TextBox();
    private PasswordTextBox password = new PasswordTextBox();
    private TextBox ipadresse = new TextBox();
    private TextBox port = new TextBox();
    private Button loginbutton = new Button("Login");    

    /**Link to open the imprint dialog*/
	private Anchor imprint = new Anchor("Impressum");
	
	/**The imprint dialog*/
	private ImprintDialog imprintDialog = new ImprintDialog();
    
    public LoginView (){
    	
    	int windowHeight = Window.getClientHeight();
		int windowWidth = Window.getClientWidth();
    	
        loginLayout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = loginLayout.getFlexCellFormatter();

        // Add a title to the form
        loginLayout.setHTML(0, 0, this.headline);
        cellFormatter.setColSpan(0, 0, 2);
        cellFormatter.setHorizontalAlignment(0, 0, HasHorizontalAlignment.ALIGN_CENTER);

        // Add username and password fields
        username.setWidth("150px");
        password.setWidth("150px");
        loginLayout.setHTML(1, 0, this.usernameLabel);
        loginLayout.setWidget(1, 1, username);
        loginLayout.setHTML(2, 0, passwordLabel);
        loginLayout.setWidget(2, 1, password);
        
        //Add the loginbutton to the form
        loginLayout.setWidget(3, 0, loginbutton);
        cellFormatter.setColSpan(3, 0, 2);
        cellFormatter.setHorizontalAlignment(3, 0, HasHorizontalAlignment.ALIGN_CENTER);
        
        // Create some advanced options
        Grid advancedOptions = new Grid(2, 2);
        advancedOptions.setCellSpacing(6);
        advancedOptions.setHTML(0, 0, ipLabel);
        advancedOptions.setWidget(0, 1, ipadresse);
        advancedOptions.setHTML(1, 0, portLabel);
        advancedOptions.setWidget(1, 1, port);
        
        // Add advanced options to form in a disclosure panel
        DisclosurePanel advancedDisclosure = new DisclosurePanel("Advanced Settings");
        advancedDisclosure.setAnimationEnabled(true);
        advancedDisclosure.ensureDebugId("DisclosurePanel");
        advancedDisclosure.setContent(advancedOptions);
        
        loginLayout.setWidget(4, 0, advancedDisclosure);
        cellFormatter.setColSpan(4, 0, 2);
        
        /*Add an event handler on the link to show the imprint dialog*/
	    this.imprint.addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	  
	        	  imprintDialog.getImprintDialogBox().center();
	  	          imprintDialog.getImprintDialogBox().show();
	          }
		 });     

        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(loginLayout);
        
        mainpanel.setWidth(windowWidth/2 + "px");
		mainpanel.setHeight(windowHeight*0.6 + "px");      
        mainpanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_CENTER);
        mainpanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
        mainpanel.add(logo);
        mainpanel.add(secondoHeadline);
        mainpanel.add(decPanel);
        mainpanel.add(imprint);
    }

    /**Returns the textbox for the username
	 * 
	 * @return The textbox for the username
	 * */
	public TextBox getUsername() {
		return username;
	}

	/**Returns the password textbox for the password
	 * 
	 * @return The password textbox for the password
	 * */
	public PasswordTextBox getPassword() {
		return password;
	}

	/**Returns the login button
	 * 
	 * @return The login button
	 * */
	public Button getLoginbutton() {
		return loginbutton;
	}

	/**Returns the textbox for the URL
	 * 
	 * @return The textbox for the URL
	 * */
	public TextBox getIpadresse() {
		return ipadresse;
	}

	/**Returns the textbox for the port
	 * 
	 * @return The textbox for the port
	 * */
	public TextBox getPort() {
		return port;
	}

	/**Returns the header of the login view
     * 
     * @return The header of the login view
     * */
	public LoginHeader getLoginheader() {
		return loginheader;
	}

	/**Returns the footer of the login view
     * 
     * @return The footer of the login view
     * */
	public LoginFooter getLoginfooter() {
		return loginfooter;
	}

    /**Returns the main panel of the login view
     * 
     * @return The main panel of the login view
     * */
	public VerticalPanel getMainPanel() {
		return mainpanel;
	}
}
