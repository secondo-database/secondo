package com.secondo.webgui.client.loginview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.DecoratorPanel;
import com.google.gwt.user.client.ui.DisclosurePanel;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlexTable.FlexCellFormatter;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.PasswordTextBox;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;

public class LoginView extends Composite{
	
	private Image logo = new Image("resources/images/secondo-logo.gif");
	private HTML secondoHeadline = new HTML("<h1>An Extensible Database System </h1>");
    private String headline = "Log into Secondo Database";	
	private String usernameLabel = "Username: ";
    private String passwordLabel = "Password: ";
    private String ipLabel = "Server-IP-Adresse: ";
    private String portLabel = "Port: ";
    private TextBox username = new TextBox();
    private PasswordTextBox password = new PasswordTextBox();
    private TextBox ipadresse = new TextBox();
    private TextBox port = new TextBox();
    private Button loginbutton = new Button("Login");
    private HTML forgotPassword = new HTML("Forgot your Password? Click here to reset it.");
    private FlexTable layout = new FlexTable();
    private DecoratorPanel decPanel = new DecoratorPanel();
    private VerticalPanel panel = new VerticalPanel();
	private boolean loggedin = false;
    
    public LoginView (){
    	
        layout.setCellSpacing(6);
        FlexCellFormatter cellFormatter = layout.getFlexCellFormatter();

        // Add a title to the form
        layout.setHTML(0, 0, this.headline);
        cellFormatter.setColSpan(0, 0, 2);
        cellFormatter.setHorizontalAlignment(
            0, 0, HasHorizontalAlignment.ALIGN_CENTER);

        // Add username and password fields
        username.setWidth("150px");
        password.setWidth("150px");
        layout.setHTML(1, 0, this.usernameLabel);
        layout.setWidget(1, 1, username);
        layout.setHTML(2, 0, passwordLabel);
        layout.setWidget(2, 1, password);
        
        //Add the loginbutton to the form
        layout.setWidget(3, 0, loginbutton);
        cellFormatter.setColSpan(3, 0, 2);
        cellFormatter.setHorizontalAlignment(
            3, 0, HasHorizontalAlignment.ALIGN_CENTER);
        
        // Create some advanced options
        Grid advancedOptions = new Grid(2, 2);
        advancedOptions.setCellSpacing(6);
        advancedOptions.setHTML(0, 0, ipLabel);
        advancedOptions.setWidget(0, 1, ipadresse);
        advancedOptions.setHTML(1, 0, portLabel);
        advancedOptions.setWidget(1, 1, port);
        
        // Add advanced options to form in a disclosure panel
        DisclosurePanel advancedDisclosure = new DisclosurePanel(
           "Advanced Settings");
        advancedDisclosure.setAnimationEnabled(true);
        advancedDisclosure.ensureDebugId("DisclosurePanel");
        advancedDisclosure.setContent(advancedOptions);
        
        layout.setWidget(4, 0, advancedDisclosure);
        cellFormatter.setColSpan(4, 0, 2);
        

        // Wrap the content in a DecoratorPanel
        decPanel.setWidget(layout);
        
        panel.setSize("884px", "490px");
        panel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_CENTER);
        panel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
        panel.add(logo);
        panel.add(secondoHeadline);
        panel.add(decPanel);
        panel.add(forgotPassword);
        

    }


	public VerticalPanel getPanel() {
		return panel;
	}


	public void setPanel(VerticalPanel panel) {
		this.panel = panel;
	}


	public TextBox getUsername() {
		return username;
	}

	public void setUsername(TextBox username) {
		this.username = username;
	}

	public PasswordTextBox getPassword() {
		return password;
	}

	public void setPassword(PasswordTextBox password) {
		this.password = password;
	}


	public Button getLoginbutton() {
		return loginbutton;
	}


	public void setLoginbutton(Button loginbutton) {
		this.loginbutton = loginbutton;
	}


	public TextBox getIpadresse() {
		return ipadresse;
	}


	public void setIpadresse(TextBox ipadresse) {
		this.ipadresse = ipadresse;
	}


	public TextBox getPort() {
		return port;
	}


	public void setPort(TextBox port) {
		this.port = port;
	}


	public boolean isLoggedin() {
		return loggedin;
	}


	public void setLoggedin(boolean loggedin) {
		this.loggedin = loggedin;
	}


	
}
