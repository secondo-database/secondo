package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.MenuBar;
import com.google.gwt.user.client.ui.MenuItem;
import com.google.gwt.user.client.ui.RadioButton;

public class MenuBarCommandPanel extends Composite{
	
	private HorizontalPanel hPanel = new HorizontalPanel();
	private String phrase = "Thanks for selecting this menu item!";
	private MenuBar menuBar = new MenuBar();
    private MenuBar commandMenu = new MenuBar(true);
    private MenuBar basicCommandsMenu = new MenuBar(true); //submenu
    private MenuBar optimizerMenu = new MenuBar(true);
    private MenuBar helpMenu = new MenuBar(true);
    private MenuItem basicCommandItem1;
    private Command basicCommandPoint;
    private MenuItem basicCommandItem2;
    private Command basicCommandRegion;
    private MenuItem basicCommandItem3;
    private Command basicCommandLine;
    
    private HorizontalPanel commandBox = new HorizontalPanel();
	private ListBox querySelection = new ListBox(false);
	private String command = "Select Command...";
	private HorizontalPanel optimizerBox = new HorizontalPanel();
	private Label optimizerLabel = new Label("Optimizer ");
    private RadioButton rbOn = new RadioButton("optimizerGroup", "On");
    private RadioButton rbOff = new RadioButton("optimizerGroup", "Off");

	
	public MenuBarCommandPanel(){
		
		hPanel.setSize("900px", "30px");
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
	    hPanel.getElement().setClassName("menubar");
	    
        querySelection.setSize("340px", "30px");
        querySelection.addItem(command);
        commandBox.add(querySelection);
        commandBox.setWidth("360px");
        commandBox.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
        hPanel.add(commandBox);
        
        optimizerBox.add(optimizerLabel);
        optimizerBox.add(rbOn);
        optimizerBox.add(rbOff);
        rbOff.setValue(true);
        optimizerBox.setWidth("180px");
        optimizerBox.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_CENTER);
        hPanel.add(optimizerBox);
        
		
		 // Create a command that will execute on menu item selection
	    Command menuCommand = new Command() {

	      public void execute() {
	        Window.alert(phrase);
	      }
	    };

	    
	    // Configure the menu bar
	    menuBar.setAutoOpen(true);
	    menuBar.setAnimationEnabled(true);
	    menuBar.setWidth("360px");
	    menuBar.ensureDebugId("MenuBar");
	    menuBar.getElement().setClassName("menubar-commandpanel");
	          
	    // Add entries for basic commands to the submenu
	    basicCommandItem1 = new MenuItem("get Point Bremen from Germany Database", basicCommandPoint);
	    basicCommandItem2 = new MenuItem("get Region Rosenheim from Germany Database", basicCommandRegion);
	    basicCommandItem3 = new MenuItem("get Lines Autobahn containing 6 from Germany Database", basicCommandLine);
	    basicCommandsMenu.addItem(basicCommandItem1);
	    basicCommandsMenu.addItem(basicCommandItem2);
	    basicCommandsMenu.addItem(basicCommandItem3);
	    
	    //create a menu for commands
	    menuBar.addItem(new MenuItem("Command",commandMenu));
	    commandMenu.addItem("Basic Commands", basicCommandsMenu);
        commandMenu.addSeparator();
        commandMenu.addItem("Example", menuCommand);
        
        
        // create a menu for the optimizer
	    menuBar.addItem(new MenuItem("Optimizer", optimizerMenu));
	    optimizerMenu.addItem("Connect", menuCommand);
	    optimizerMenu.addItem("Settings", menuCommand);
	    
	    // create a menu for help options
	    menuBar.addItem(new MenuItem("Help", helpMenu));
	    helpMenu.addItem("Help1", menuCommand);
	    helpMenu.addItem("Help2", menuCommand);
	    	    
	    //add the menubar to the panel for navigation
		hPanel.add(menuBar);
		
	}

	
	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	public void sethPanel(HorizontalPanel hPanel) {
		this.hPanel = hPanel;
	}


	public ListBox getQuerySelection() {
		return querySelection;
	}


	public void setQuerySelection(ListBox querySelection) {
		this.querySelection = querySelection;
	}


	public MenuItem getBasicCommandItem1() {
		return basicCommandItem1;
	}


	public void setBasicCommandItem1(MenuItem basicCommandItem1) {
		this.basicCommandItem1 = basicCommandItem1;
	}


	public MenuItem getBasicCommandItem2() {
		return basicCommandItem2;
	}


	public void setBasicCommandItem2(MenuItem basicCommandItem2) {
		this.basicCommandItem2 = basicCommandItem2;
	}


	public MenuItem getBasicCommandItem3() {
		return basicCommandItem3;
	}


	public void setBasicCommandItem3(MenuItem basicCommandItem3) {
		this.basicCommandItem3 = basicCommandItem3;
	}
	
	
}
