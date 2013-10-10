package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.TextBox;

public class StatusBar extends Composite{

	private HorizontalPanel hPanel = new HorizontalPanel();
	private Label secondoServerLabel = new Label("Secondo-Server-Address: ");
	private Label secondoServer = new Label("defaultaddress");
	private Label userNameLabel = new Label("UserName: ");
	private Label userName = new Label("defaultuser");
	private Label databaseLabel = new Label("Open Database: ");
	private Label openDatabase = new Label("defaultdatabase");
	
	
	public StatusBar(){
		hPanel.setSize("900px", "30px");
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
		hPanel.getElement().setClassName("statusbar");
	    
	    hPanel.add(secondoServerLabel);
	    hPanel.add(secondoServer);
		hPanel.add(userNameLabel);
	    hPanel.add(userName);
		hPanel.add(databaseLabel);
	    hPanel.add(openDatabase);
	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}

	public Label getSecondoServer() {
		return secondoServer;
	}

	public void setSecondoServer(Label secondoServer) {
		this.secondoServer = secondoServer;
	}

	public Label getUserName() {
		return userName;
	}

	public void setUserName(Label userName) {
		this.userName = userName;
	}

	public Label getOpenDatabase() {
		return openDatabase;
	}

	public void setOpenDatabase(Label openDatabase) {
		this.openDatabase = openDatabase;
	}


}
