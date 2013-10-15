package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.VerticalPanel;

public class DatabaseView extends Composite{
	
	// Create a panel to align the Widgets
    private HorizontalPanel hPanel = new HorizontalPanel();
    private DatabaseHeader databaseHeader = new DatabaseHeader();
    private DatabaseFooter databaseFooter = new DatabaseFooter();

	 // Add a list box with multiple selection enabled
    private ListBox multiBox = new ListBox(true);
    private FlowPanel contentPanel = new FlowPanel();
    private Button openDatabaseButton = new Button("Open Database");
	private boolean openDatabase = false;
	
	private Button logoutButton = new Button("Logout");
	private HorizontalPanel buttonBox = new HorizontalPanel();
	
	public DatabaseView(){
		
		hPanel.setSpacing(20);

	    multiBox.ensureDebugId("ListBox-multiBox");
	    multiBox.setWidth("20em");
	    multiBox.setVisibleItemCount(10);
	    
	    buttonBox.add(openDatabaseButton);
	    buttonBox.add(logoutButton);
	    buttonBox.setSpacing(10);

	    //multiBoxPanel.setSpacing(4);
	    contentPanel.add(new HTML("Select a Database:"));
	    contentPanel.add(multiBox);
	    contentPanel.add(buttonBox);
	    //contentPanel.add(openDatabaseButton);
    
	    hPanel.add(contentPanel);
	    

	}
	
	public void addDatabase(String item){
		multiBox.addItem(item);
	}

	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}

	public Button getOpenDatabaseButton() {
		return openDatabaseButton;
	}

	public void setOpenDatabaseButton(Button openDatabaseButton) {
		this.openDatabaseButton = openDatabaseButton;
	}

	public boolean isOpenDatabase() {
		return openDatabase;
	}

	public void setOpenDatabase(boolean openDatabase) {
		this.openDatabase = openDatabase;
	}

	public ListBox getMultiBox() {
		return multiBox;
	}

	public void setMultiBox(ListBox multiBox) {
		this.multiBox = multiBox;
	}

	public void setDatabaseEntry(String item){
		this.multiBox.addItem(item);
	}
	
	public void removeDatabaseEntry(int index){
		this.multiBox.removeItem(index);
	}

	public DatabaseFooter getDatabaseFooter() {
		return databaseFooter;
	}

	public void setDatabaseFooter(DatabaseFooter databaseFooter) {
		this.databaseFooter = databaseFooter;
	}

	public DatabaseHeader getDatabaseHeader() {
		return databaseHeader;
	}

	public void setDatabaseHeader(DatabaseHeader databaseHeader) {
		this.databaseHeader = databaseHeader;
	}

	public Button getLogoutButton() {
		return logoutButton;
	}

	public void setLogoutButton(Button logoutButton) {
		this.logoutButton = logoutButton;
	}

}
