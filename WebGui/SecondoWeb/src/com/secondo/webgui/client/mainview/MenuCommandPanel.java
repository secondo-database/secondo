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

import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.MenuBar;
import com.google.gwt.user.client.ui.MenuItem;

/**
*  This class represents the menubar of the commandpanel with functions concerning the commandpanel.
*  
*  @author Kristina Steiger
*  
**/
public class MenuCommandPanel extends Composite{
	
	/**The main panel of the menu*/
	private HorizontalPanel hPanel = new HorizontalPanel();
	
    /** The command history box*/
	private ListBox commandHistoryBox = new ListBox(false);
	
	/**The default text for the command history box*/
	private String command = "Command History...";
	
	/**The menubar object*/
	private MenuBar menuBar = new MenuBar();
	
	/**The hide terminal button*/
	private Button hideTerminalButton = new Button("<img src='resources/images/remove-icon.png' height='20px' width='20px'/>");	
    
    //elements of the optimizer menubar
    private MenuBar optimizerMenu = new MenuBar(true);
    private String optimizerIcon = "<img src='resources/images/tools-icon.png' height='25px' width='25px'/>";
	private MenuItem optimizerItemSettings;
	private Command optimizerSettings;
	private OptimizerSettingsDialog optimizerDialog = new OptimizerSettingsDialog();
	private MenuItem optimizerItemOn;
	private Command optimizerOnCommand;
	private String optimizerOnIcon = "<img src='resources/images/accept-icon.png' height='12px' width='12px'/>";
	private MenuItem optimizerItemOff;
	private Command optimizerOffCommand;
	private String optimizerOffIcon = "<img src='resources/images/delete-icon.png' height='12px' width='12px'/>";  
	
	//elements of the example command menubar
    private MenuBar commandMenu = new MenuBar(true);
	private String commandsIcon = "<img src='resources/images/plus-icon.png' height='25px' width='25px'/>";
    private MenuBar germanyMenu = new MenuBar(true); //submenu
    private MenuBar berlinMenu = new MenuBar(true); //submenu
    private MenuBar mvtripsMenu = new MenuBar(true); //submenu
	private MenuBar optimizerCommandMenu = new MenuBar(true); //submenu
    
    //Menu items and commands for example queries
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
    private MenuItem basicCommandItem6;
    private Command basicCommandGrenzenLineBerlin;
    private MenuItem basicCommandItem7;
    private Command basicCommandKinosBerlin;
    private MenuItem basicCommandItem8;
    private Command basicCommandUBahnBerlin;
    private MenuItem basicCommandItem9;
    private Command basicCommandMPointBerlin;
    private MenuItem basicCommandItem10;
    private Command basicCommandMPointMVTrips;
    private MenuItem basicCommandItem11;
    private Command basicCommandMPointsBerlin;
    private MenuItem basicCommandItem12;
    private Command basicCommandMPointsMVTrips;
    private MenuItem basicCommandItem13;
    private Command basicCommandOptimizerCountTrainsMehringdamm;
    private MenuItem basicCommandItem14;
    private Command basicCommandOptimizerGetTrainsMehringdamm;
    
    //elements of the help menubar
    private MenuBar helpMenu = new MenuBar(true);
	private String helpIcon = "<img src='resources/images/help-icon.png' height='25px' width='25px'/>";
	private MenuItem helpItem1;
	private SecondoSyntaxDialog secondoSyntaxDialog = new SecondoSyntaxDialog();
	private MenuItem helpItem2;
	private OptimizerSyntaxDialog optimizerSyntaxDialog = new OptimizerSyntaxDialog();
	
	private FlowPanel zoomPanel = new FlowPanel();
	private Button zoomAllButton = new Button("<img src='resources/images/world_add.png' height='20px' width='20px'/>");
	private Button zoomLastButton = new Button("<img src='resources/images/world_delete.png' height='20px' width='20px'/>");
	
	/**Value is true if optimizer is turned on*/
    private boolean optimizerTurnedOn = false; 
	
	public MenuCommandPanel(){
		
		hPanel.setHeight("30px");		
	    hPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_LEFT);
	    hPanel.setVerticalAlignment(HasVerticalAlignment.ALIGN_MIDDLE);
	    hPanel.getElement().setClassName("menubar");
	    
	    //initialize the command history box
        commandHistoryBox.setWidth("300px");
        commandHistoryBox.addItem(command);
        hPanel.add(commandHistoryBox);
        
	    zoomAllButton.setTitle("Zoom to all queries");
		zoomAllButton.getElement().setClassName("zoomallbutton");
		zoomLastButton.setTitle("Zoom to the last query");
		zoomLastButton.getElement().setClassName("zoomlastbutton");
        zoomPanel.add(zoomAllButton);
		hPanel.add(zoomPanel);
        
	    //Configure the main menu bar
	    menuBar.setAutoOpen(true);
	    menuBar.setAnimationEnabled(true);
	    menuBar.ensureDebugId("MenuBar");
	    menuBar.getElement().setClassName("menubarpopup");
	    hPanel.add(menuBar);
	    
        //create optimizer menu items
        optimizerItemSettings = new MenuItem("Optimizer Settings", optimizerSettings);
	    optimizerMenu.addItem(optimizerItemSettings);	    
	    optimizerItemOn = new MenuItem("Turn Optimizer On &nbsp;&nbsp;" + optimizerOnIcon, true, optimizerOnCommand);
		optimizerMenu.addItem(optimizerItemOn);		        
		optimizerItemOff = new MenuItem("Turn Optimizer Off &nbsp;&nbsp;" + optimizerOffIcon, true, optimizerOffCommand);            
	    menuBar.addItem(new MenuItem(optimizerIcon, true, optimizerMenu));
	          
	    //create menu items for germany database
	    basicCommandItem1 = new MenuItem("get Point for Stadt Bremen", basicCommandPoint);
	    basicCommandItem4 = new MenuItem("get Points for Staedte containing \"Bre\"", basicCommandPoints);
	    basicCommandItem2 = new MenuItem("get Region for Kreis containing \"LK Rosenheim\"", basicCommandRegion);
	    basicCommandItem3 = new MenuItem("get Lines for Autobahnen containing \"6\"", basicCommandLine);
	    //add germany examples
	    germanyMenu.addItem(basicCommandItem1);
	    germanyMenu.addItem(basicCommandItem4);
	    germanyMenu.addItem(basicCommandItem3);
	    germanyMenu.addItem(basicCommandItem2);
	    
	    //create menu items for berlin database
	    basicCommandItem5 = new MenuItem("get Region Grunewald", basicCommandRegionBerlin);
	    basicCommandItem6 = new MenuItem("get Line BerlinGrenzenLine", basicCommandGrenzenLineBerlin);
	    basicCommandItem7 = new MenuItem("get Points for all Kinos", basicCommandKinosBerlin);
	    basicCommandItem8 = new MenuItem("get Lines for all UBahn lines", basicCommandUBahnBerlin);
	    basicCommandItem9 = new MenuItem("get Moving Point for train7", basicCommandMPointBerlin);
	    basicCommandItem11 = new MenuItem("get All Moving Points for UBahnen passing Tiergarten", basicCommandMPointsBerlin);
	    //add berlin examples
	    berlinMenu.addItem(basicCommandItem7);
	    berlinMenu.addItem(basicCommandItem6);
	    berlinMenu.addItem(basicCommandItem5);	
	    berlinMenu.addItem(basicCommandItem8);
	    berlinMenu.addItem(basicCommandItem9);
	    berlinMenu.addItem(basicCommandItem11);
	    
	    //create menu items for mvtrips database
	    basicCommandItem10 = new MenuItem("get One Moving Point for one Daytrip", basicCommandMPointMVTrips);
	    basicCommandItem12 = new MenuItem("get All Moving Points for all DayTrips 2011", basicCommandMPointsMVTrips);	    
	    //add mvtrips examples
	    mvtripsMenu.addItem(basicCommandItem10);
	    mvtripsMenu.addItem(basicCommandItem12);
	    
	    //create menu items for optimizer
	    basicCommandItem13 = new MenuItem("Count all trains passing mehringdamm", basicCommandOptimizerCountTrainsMehringdamm);
	    basicCommandItem14 = new MenuItem("Get all trains passing mehringdamm", basicCommandOptimizerGetTrainsMehringdamm);
	    //add optimizer examples
	    optimizerCommandMenu.addItem(basicCommandItem13);
	    optimizerCommandMenu.addItem(basicCommandItem14);
	    
	    //Add submenus of example commands to the menubar
	    commandMenu.addItem(new MenuItem("Commands for Berlintest Database", berlinMenu));
	    commandMenu.addSeparator();
	    commandMenu.addItem(new MenuItem("Commands for Germany Database", germanyMenu));
	    commandMenu.addSeparator();
	    commandMenu.addItem(new MenuItem("Commands for MVTrips Database", mvtripsMenu));
	    commandMenu.addSeparator();
	    commandMenu.addItem(new MenuItem("Commands with Optimizer", optimizerCommandMenu));	           
	    menuBar.addItem(new MenuItem(commandsIcon, true , commandMenu));
	    
         //Create a menu for help options     
	    menuBar.addItem(new MenuItem(helpIcon, true, helpMenu));
	    
	    //command that will execute on selection of the secondo syntax menu item
	    Command secondoInfo = new Command() {
	      public void execute() {
	        secondoSyntaxDialog.getHelpDialogBox().center();
	        secondoSyntaxDialog.getHelpDialogBox().show();
	      }
	    };
	    helpItem1 = new MenuItem("Secondo Syntax", secondoInfo);
	    helpMenu.addItem(helpItem1);
	    
	    //command that will execute on selection of the optimizer info menu item
	    Command optimizerInfo = new Command() {
	      public void execute() {
	        optimizerSyntaxDialog.getHelpDialogBox().center();
	        optimizerSyntaxDialog.getHelpDialogBox().show();
	      }
	    };
	    helpItem2 = new MenuItem("Optimizer Syntax", optimizerInfo);
	    helpMenu.addItem(helpItem2);
	  
		
	    //configure the button to hide the commandpanel
		hideTerminalButton.setSize("28px", "28px");
		hideTerminalButton.setTitle("Hide Commandpanel");
		hideTerminalButton.getElement().setClassName("hideterminalbutton");
		hPanel.add(hideTerminalButton);
	}
	
 
	/**On resizing of the browser window the width of the menubar is readjusted
	 * 
	 * @param width The new width of the menubar
	 * */
	public void resizeWidth(int width){
		if(width > 1000){
			hPanel.setWidth(width-70 + "px");
			//add 60 px for 2 buttons + 300px for history +40 padding
			menuBar.setWidth((width-470) + "px");
		}
		else{
			hPanel.setWidth(1000-70 + "px");
			menuBar.setWidth((1000-470) + "px");
		}
	}
	
	/**Returns the main panel of the menu
	 * 
	 * @return The main panel of the menu
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}

	/**Returns the command history box
	 * 
	 * @return The command history box
	 * */
	public ListBox getCommandHistoryBox() {
		return commandHistoryBox;
	}

	/**Returns the basic command item no. 1
	 * 
	 * @return The basic command item no. 1
	 * */
	public MenuItem getBasicCommandItem1() {
		return basicCommandItem1;
	}

	/**Returns the basic command item no. 2
	 * 
	 * @return The basic command item no. 2
	 * */
	public MenuItem getBasicCommandItem2() {
		return basicCommandItem2;
	}

	/**Returns the basic command item no. 3
	 * 
	 * @return The basic command item no. 3
	 * */
	public MenuItem getBasicCommandItem3() {
		return basicCommandItem3;
	}

	/**Returns the basic command item no. 4
	 * 
	 * @return The basic command item no. 4
	 * */
	public MenuItem getBasicCommandItem4() {
		return basicCommandItem4;
	}
	
	/**Returns the basic command item no. 5
	 * 
	 * @return The basic command item no. 5
	 * */
	public MenuItem getBasicCommandItem5() {
		return basicCommandItem5;
	}

	/**Returns the basic command item no. 6
	 * 
	 * @return The basic command item no. 6
	 * */
	public MenuItem getBasicCommandItem6() {
		return basicCommandItem6;
	}

	/**Returns the basic command item no. 7
	 * 
	 * @return The basic command item no. 7
	 * */
	public MenuItem getBasicCommandItem7() {
		return basicCommandItem7;
	}

	/**Returns the basic command item no. 8
	 * 
	 * @return The basic command item no. 8
	 * */
	public MenuItem getBasicCommandItem8() {
		return basicCommandItem8;
	}

	/**Returns the basic command item no. 9
	 * 
	 * @return The basic command item no. 9
	 * */
	public MenuItem getBasicCommandItem9() {
		return basicCommandItem9;
	}

	/**Returns the basic command item no. 10
	 * 
	 * @return The basic command item no. 10
	 * */
	public MenuItem getBasicCommandItem10() {
		return basicCommandItem10;
	}

	/**Returns the basic command item no. 11
	 * 
	 * @return The basic command item no. 11
	 * */
	public MenuItem getBasicCommandItem11() {
		return basicCommandItem11;
	}

	/**Returns the basic command item no. 12
	 * 
	 * @return The basic command item no. 12
	 * */
	public MenuItem getBasicCommandItem12() {
		return basicCommandItem12;
	}

	/**Returns the basic command item no. 13
	 * 
	 * @return The basic command item no. 13
	 * */
	public MenuItem getBasicCommandItem13() {
		return basicCommandItem13;
	}

	/**Returns the basic command item no. 14
	 * 
	 * @return The basic command item no. 14
	 * */
	public MenuItem getBasicCommandItem14() {
		return basicCommandItem14;
	}

	/**Returns the hide terminal button
	 * 
	 * @return The hide terminal button
	 * */
	public Button getHideTerminalButton() {
		return hideTerminalButton;
	}

	/**Returns true if the optimizer is turned on
	 * 
	 * @return True if the optimizer is turned on
	 * */
	public boolean isOptimizerTurnedOn() {
		return optimizerTurnedOn;
	}

	/**Sets the optimizerTurnedOn attribute to the given value
	 * 
	 * @param optimizerTurnedOn True if the optimizer is turned on, false if its turned off
	 * */
	public void setOptimizerTurnedOn(boolean optimizerTurnedOn) {
		this.optimizerTurnedOn = optimizerTurnedOn;
	}

	/**Returns the optimizer dialog
	 * 
	 * @return The optimizer dialog
	 * */
	public OptimizerSettingsDialog getOptimizerDialog() {
		return optimizerDialog;
	}

	/**Returns the optimizer settings menu item
	 * 
	 * @return The optimizer settings menu item
	 * */
	public MenuItem getOptimizerItemSettings() {
		return optimizerItemSettings;
	}

	/**Returns the turn on optimizer menu item
	 * 
	 * @return The turn on optimizer menu item
	 * */
	public MenuItem getOptimizerItemOn() {
		return optimizerItemOn;
	}

	/**Returns the turn off optimizer menu item
	 * 
	 * @return The turn off optimizer menu item
	 * */
	public MenuItem getOptimizerItemOff() {
		return optimizerItemOff;
	}

	/**Returns the optimizer menubar
	 * 
	 * @return The optimizer menubar
	 * */
	public MenuBar getOptimizerMenu() {
		return optimizerMenu;
	}

	public FlowPanel getZoomPanel() {
		return zoomPanel;
	}

	public Button getZoomAllButton() {
		return zoomAllButton;
	}

	public Button getZoomLastButton() {
		return zoomLastButton;
	}
	
}