package com.secondo.webgui.client.mainview;

import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
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
	private Button hideTerminalButton = new Button("<img src='resources/images/red-cross-icon.png' height='20px' width='20px'/>");
	private Button clearTerminalButton = new Button("<img src='resources/images/clear-terminal-icon.png' height='20px' width='20px'/>");
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
    private MenuItem basicCommandItem4;
    private Command basicCommandPoints;
    private MenuItem basicCommandItem5;
    private Command basicCommandRegionBerlin;
    
    private HorizontalPanel commandBox = new HorizontalPanel();
	private ListBox querySelection = new ListBox(false);
	private String command = "Command History...";
	private HorizontalPanel optimizerBox = new HorizontalPanel();
	private Label optimizerLabel = new Label("Optimizer ");
    private RadioButton rbOn = new RadioButton("optimizerGroup", "On");
    private RadioButton rbOff = new RadioButton("optimizerGroup", "Off");

	
	public MenuBarCommandPanel(){
		
		hPanel.setHeight("30px");
		
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
	    hPanel.getElement().setClassName("menubar");
	    
        querySelection.setWidth("300px");
        querySelection.addItem(command);
        commandBox.add(querySelection);
        commandBox.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
        hPanel.add(commandBox);
        
        optimizerBox.add(optimizerLabel);
        optimizerBox.add(rbOn);
        optimizerBox.add(rbOff);
        rbOff.setValue(true);
        optimizerBox.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_CENTER);
        //hPanel.add(optimizerBox);
        
		 //Example for a command that will execute on menu item selection
	    Command menuCommand = new Command() {

	      public void execute() {
	        Window.alert(phrase);
	      }
	    };
  
	    // Configure the menu bar
	    menuBar.setAutoOpen(true);
	    menuBar.setAnimationEnabled(true);
	    menuBar.ensureDebugId("MenuBar");
	    menuBar.getElement().setClassName("menubaritems");
	          
	    // Add entries for basic commands to the submenu
	    basicCommandItem1 = new MenuItem("get Point Bremen from Germany Database", basicCommandPoint);
	    basicCommandItem4 = new MenuItem("get Points from Germany Database", basicCommandPoints);
	    basicCommandItem2 = new MenuItem("get Region Rosenheim from Germany Database", basicCommandRegion);
	    basicCommandItem3 = new MenuItem("get Lines Autobahn containing 6 from Germany Database", basicCommandLine);
	    basicCommandItem5 = new MenuItem("get Region Grunewald from Berlin Database", basicCommandRegionBerlin);

	    commandMenu.addItem(basicCommandItem1);
	    commandMenu.addItem(basicCommandItem4);
	    commandMenu.addItem(basicCommandItem2);
	    commandMenu.addItem(basicCommandItem3);
	    commandMenu.addItem(basicCommandItem5);
	    
	    //create a menu for commands
	    menuBar.addItem(new MenuItem("Example Commands",commandMenu));
	    //commandMenu.addItem("Basic Commands", basicCommandsMenu); //add submenu
       // commandMenu.addSeparator();
       // commandMenu.addItem("Example", menuCommand);   
        
        // create a menu for the optimizer
	   /* menuBar.addItem(new MenuItem("Optimizer", optimizerMenu));
	    optimizerMenu.addItem("Connect", menuCommand);
	    optimizerMenu.addItem("Settings", menuCommand);*/
	    
	    // create a menu for help options
	   /* menuBar.addItem(new MenuItem("Help", helpMenu));
	    helpMenu.addItem("Help1", menuCommand);
	    helpMenu.addItem("Help2", menuCommand);*/
  	    
	    //add the menubar to the panel for navigation
		hPanel.add(menuBar);
		
		hideTerminalButton.setSize("30px", "30px");
		hideTerminalButton.setTitle("Hide Commandpanel");
		//clearTerminalButton.setSize("30px", "30px");
		//clearTerminalButton.setTitle("Clear Commandpanel");
		hPanel.add(hideTerminalButton);
		//hPanel.add(clearTerminalButton);
		
	}
	
	//add 20 pixel for the padding
	public void resize(int width){
		
		hPanel.setWidth(width-80 + "px");
		commandBox.setWidth((width-110)*0.5 + "px"); //without buttons
		menuBar.setWidth((width-110)*0.5 + "px");
		//optimizerBox.setWidth((width-90)*0.2 + "px");

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


	public MenuItem getBasicCommandItem4() {
		return basicCommandItem4;
	}


	public void setBasicCommandItem4(MenuItem basicCommandItem4) {
		this.basicCommandItem4 = basicCommandItem4;
	}
	
	public MenuItem getBasicCommandItem5() {
		return basicCommandItem5;
	}

	public void setBasicCommandItem5(MenuItem basicCommandItem5) {
		this.basicCommandItem5 = basicCommandItem5;
	}

	public Button getClearTerminalButton() {
		return clearTerminalButton;
	}

	public void setClearTerminalButton(Button clearTerminalButton) {
		this.clearTerminalButton = clearTerminalButton;
	}

	public Button getHideTerminalButton() {
		return hideTerminalButton;
	}

	public void setHideTerminalButton(Button hideTerminalButton) {
		this.hideTerminalButton = hideTerminalButton;
	}


}
