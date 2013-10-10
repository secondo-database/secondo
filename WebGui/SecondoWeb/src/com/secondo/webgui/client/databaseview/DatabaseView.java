package com.secondo.webgui.client.databaseview;

import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.VerticalPanel;

public class DatabaseView extends Composite{
	
	// Create a panel to align the Widgets
    private HorizontalPanel hPanel = new HorizontalPanel();
	 // Add a list box with multiple selection enabled
    private ListBox multiBox = new ListBox(true);
    private VerticalPanel multiBoxPanel = new VerticalPanel();
    private Button openDatabaseButton = new Button("Open Database");
	private boolean openDatabase = false;
	
	public DatabaseView(){
		
		hPanel.setSpacing(20);

	    multiBox.ensureDebugId("ListBox-multiBox");
	    multiBox.setWidth("20em");
	    multiBox.setVisibleItemCount(10);
	    multiBox.addItem("Test Database");

	    multiBoxPanel.setSpacing(4);
	    multiBoxPanel.add(new HTML("Select a Database:"));
	    multiBoxPanel.add(multiBox);
	    multiBoxPanel.add(openDatabaseButton);
	    hPanel.add(multiBoxPanel);

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
}
