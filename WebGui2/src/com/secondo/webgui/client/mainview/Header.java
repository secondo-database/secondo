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

import com.google.gwt.dom.client.Style;
import com.google.gwt.dom.client.Style.Unit;
import com.google.gwt.event.dom.client.MouseMoveEvent;
import com.google.gwt.event.dom.client.MouseOutEvent;
import com.google.gwt.event.dom.client.MouseOutHandler;
import com.google.gwt.event.dom.client.MouseOverEvent;
import com.google.gwt.event.dom.client.MouseOverHandler;
import com.google.gwt.event.shared.HandlerRegistration;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Anchor;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.Composite;
import com.google.gwt.user.client.ui.FocusWidget;
import com.google.gwt.user.client.ui.Grid;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.MenuBar;
import com.google.gwt.user.client.ui.MenuItem;


/**
*  This class represents the header of the main view.
*  
*  @author Kristina Steiger
*  
**/
public class Header extends Composite{
	
	/**The main panel of the header*/
	private HorizontalPanel hPanel = new HorizontalPanel();
	
	/**The main grid of the header*/
	private Grid g=new Grid();
	
	/**The Secondo logo image*/
	private Image logo = new Image("resources/images/secondo-logo.gif");
	
	/**The panel for buttons*/
	private HorizontalPanel buttonPanel = new HorizontalPanel();
	
	/**The label to show a name of actual database*/
	private Label labelWithDatabaseName= new Label();
	
	/**The close database button*/
	private Button closedatabaseButton = new Button("<img src='resources/images/close-database.png' height='30px' width='30px'/>");
	
	/**The logout button*/
	private Button logoutButton = new Button("<img src='resources/images/logout.gif' height='30px' width='30px'/>");
	
	/**Width of the header*/
    private int width=Window.getClientWidth();
    
    private MenuBar mainMenuBar = new MenuBar();
    private MenuItem legend= new MenuItem("Legend", mainMenuBar);
    private MenuItem export = new MenuItem("Export", mainMenuBar);
    private MenuItem plainTraj= new MenuItem("Plain trajectory", mainMenuBar);
    
    private PlainTrajDialog textViewOfTrajInDialog= new PlainTrajDialog();
    private DBSettingsDialog databaseInfo= new DBSettingsDialog();
    private LocationDialog locationDialog= new LocationDialog();
	
	public Header(){
		g.resize(4, 1);

		
		logo.getElement().getStyle().setMarginLeft(30, Unit.PX);
		logo.setSize("190px", "28px");
		g.setWidget(0,0,logo);
		g.setWidth(width + "px");
		g.setHeight("28px");
		g.getElement().setClassName("mainheader");
//		hPanel.add(logo);
//		hPanel.setWidth(width + "px");
		buttonPanel.getElement().getStyle().setMarginLeft(width-430, Unit.PX);
		
		
		MenuItem homeMenu = new MenuItem("Home", new Command() {
			
			@Override
			public void execute() {
				doClickOnLink();				
			}
		});
		
		homeMenu.ensureDebugId("HomeItem");		
		homeMenu.setTitle("link to the secondo web page");
		homeMenu.getElement().getStyle().setColor("white");
		
		Anchor home= new Anchor("Home", "http://dna.fernuni-hagen.de/Secondo.html/index.html");
		home.setVisible(false);
		home.ensureDebugId("LinkToHome");
		g.setWidget(2, 0, home);
		
		MenuItem symbTrajMenu = new MenuItem ("About symbolic trajectory", new Command() {
			
			@Override
			public void execute() {
				SymTrajDialog infoAboutSymTraj= new SymTrajDialog();
				infoAboutSymTraj.getHelpDialogBox().center();
				infoAboutSymTraj.getHelpDialogBox().show();
				
			}
		});
		symbTrajMenu.getElement().getStyle().setColor("white");
		
		MenuItem database = new MenuItem("Database", new Command() {

			@Override
			public void execute() {
				databaseInfo.getDbDialogBox().center();
				databaseInfo.getDbDialogBox().show();

			}
		});
		database.getElement().getStyle().setColor("white");	
		
		MenuItem patternMenu = new MenuItem("About pattern matching", new Command() {

			@Override
			public void execute() {
				databaseInfo.getDbDialogBox().center();
				databaseInfo.getDbDialogBox().show();

			}
		});
		patternMenu.getElement().getStyle().setColor("white");	
		
		MenuBar support = new MenuBar(true);
		support.getElement().getStyle().setColor("white");
		support.getElement().getStyle().setFontStyle(Style.FontStyle.NORMAL);
		support.getElement().getStyle().setFontWeight(Style.FontWeight.NORMAL);
		
		
		MenuBar menu = new MenuBar();		
	    menu.addItem(homeMenu);
	    menu.addItem(symbTrajMenu);
	    menu.addItem(patternMenu);
	    menu.addItem(database);
	    menu.addItem("Support", support);
	    menu.getElement().getStyle().setColor("white");
	    menu.getElement().getStyle().setBorderStyle(Style.BorderStyle.NONE);
	    g.setWidget(1,0, menu);
	    g.getCellFormatter().setStyleName(1, 0, "GreyMenubar");
	    
	    
	    
	    
	    final SymTrajDialog optimizerSyntaxDialog = new SymTrajDialog();
		  //command that will execute on selection of the optimizer info menu item
		    Command optimizerInfo = new Command() {
		      public void execute() {
		        optimizerSyntaxDialog.getHelpDialogBox().center();
		        optimizerSyntaxDialog.getHelpDialogBox().show();
		      }
		    };
	    
	    

	    MenuItem homeItem=new MenuItem("My location", new Command() {
			
			@Override
			public void execute() {				
				locationDialog.getLocationDialogBox().center();
				locationDialog.getLocationDialogBox().show();
				
			}
		});	
	    homeItem.setTitle("Define your location");
	    
	    homeItem.setStyleName("transparent1");
	    
	    mainMenuBar.addItem(homeItem); 	    

	    
	    legend.setStyleName("transparent2");
	    legend.setTitle("Legend to the symbolic trajectory");
	    mainMenuBar.addItem(legend);
	    
	    Command menuCommand = new Command(){
		      public void execute() {
			        optimizerSyntaxDialog.getHelpDialogBox().center();
			        optimizerSyntaxDialog.getHelpDialogBox().show();
			      }
			    };
	    MenuItem link = new MenuItem("Link", menuCommand);	    
	    link.setStyleName("transparent3");
	    mainMenuBar.addItem(link);
	    
	    MenuItem print = new MenuItem("Print", menuCommand);	    
	    print.setStyleName("transparent4");	   
	    mainMenuBar.addItem(print);
	    
	    
	    export.setStyleName("transparent5");
	    export.setTitle("Export raw data");
	    mainMenuBar.addItem(export);
	    
	    
	    plainTraj.setStyleName("transparent6");
	    plainTraj.setTitle("Symbolic trajectory\n in plain text");	    
	    mainMenuBar.addItem(plainTraj);
	    
	    
	    
	    
	    g.setWidget(3, 0, mainMenuBar);
	    g.getCellFormatter().setStyleName(3, 0, "toolbar_menu");
	    
	    
	    
	    
		
		//configure buttons
		closedatabaseButton.getElement().setClassName("closedatabasebutton");
		closedatabaseButton.getElement().getStyle().setBackgroundColor("white");
		closedatabaseButton.setWidth("40px");
		closedatabaseButton.setTitle("Close Database");
		logoutButton.getElement().setClassName("logoutbutton");
		logoutButton.setWidth("40px");
		logoutButton.setTitle("Logout");		
		
		buttonPanel.setWidth("100px");
		buttonPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		//TODO
//		buttonPanel.getElement().getStyle().setMarginLeft(windowWidth-430, Unit.PX);
		buttonPanel.add(labelWithDatabaseName);
		buttonPanel.add(closedatabaseButton);
		buttonPanel.add(logoutButton);
		
//		hPanel.add(buttonPanel);

//		hPanel.getElement().setClassName("mainheader");	
//		hPanel.setWidth(windowWidth + "px");
//		hPanel.setHeight("50px");
	}
	
	/**On resizing of the browser window the width of the main panel and button panel is readjusted
	 * 
	 * @param width The new width of the main panel and the button panel
	 * */
	public void resizeWidth(int width){
		hPanel.setWidth(width + "px");
		if(width > 1000){
			buttonPanel.getElement().getStyle().setMarginLeft(width-430, Unit.PX);
		}		
		else{
			buttonPanel.getElement().getStyle().setMarginLeft(1000-430, Unit.PX);
		}
	}

	/**Returns the main panel of the header
	 * 
	 * @return The main panel of the header
	 * */
	public HorizontalPanel gethPanel() {
		return hPanel;
	}
	
	/**Returns the close database button
	 * 
	 * @return The close database button
	 * */
	public Button getClosedatabaseButton() {
		return closedatabaseButton;
	}

	/**Returns the logout button
	 * 
	 * @return The logout button
	 * */
	public Button getLogoutButton() {
		return logoutButton;
	}
	
	/**Returns the label with the currently selected database
	 * 
	 * @return The label
	 * */
	public Label getLabelWithDatabaseName(){
		return labelWithDatabaseName;
	}
	
	public Grid getGrid(){
		return g;
	}
	
	/**
	 * @return the legend
	 */
	public MenuItem getLegendMenuItem() {
		return legend;
	}
	

	/**
	 * @return the export
	 */
	public MenuItem getExport() {
		return export;
	}

	/**
	 * @return the plainTraj
	 */
	public MenuItem getPlainTraj() {
		return plainTraj;
	}

	/**
	 * @return the textViewOfTrajInDialog
	 */
	public PlainTrajDialog getTextViewOfTrajInDialog() {
		return textViewOfTrajInDialog;
	}
	
	public static native void doClickOnLink() /*-{
	  $doc.getElementById('gwt-debug-LinkToHome').click();
	}-*/;

	/**
	 * @return the databaseInfo
	 */
	public DBSettingsDialog getDatabaseInfo() {
		return databaseInfo;
	}

	/**
	 * @return the infoToLocation
	 */
	public LocationDialog getLocationDialog() {
		return locationDialog;
	}
	
	public String getCommandForGeocode(){
		String command="query geocode (\"";
		command+=getLocationDialog().getStreet().getText()+"\",";
		command+=getLocationDialog().getBuilding().getText()+",";
		command+=getLocationDialog().getZip().getText()+", \"";
		command+=getLocationDialog().getCity().getText()+"\")";
		return command;
		
	}
}
