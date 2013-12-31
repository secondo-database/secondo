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

package com.secondo.webgui.client;

import java.util.ArrayList;
import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.PopupPanel;
import com.google.gwt.user.client.ui.RootPanel;
import com.secondo.webgui.client.controller.RPCConnector;
import com.secondo.webgui.client.databaseview.DatabaseView;
import com.secondo.webgui.client.loginview.LoginView;
import com.secondo.webgui.client.mainview.MainView;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.client.rpc.SecondoServiceAsync;

/**
 * This is the Entry point class which defines the onModuleLoad()-Method to start the application, 
 * creates important objects and sets the content of the HTML-Page with the method setContent().
 * 
 * @author Kristina Steiger
 */
public class SecondoWeb implements EntryPoint {

	/** Main Elements of the Website which represent the divs in the HTML-Page*/
	private FlowPanel header  = new FlowPanel();
	private FlowPanel content = new FlowPanel();
	private FlowPanel footer = new FlowPanel();
	
	/**Element of the Loginview*/
	private LoginView loginView = new LoginView();
	
	/**Element of the database view*/
	private DatabaseView databaseView = new DatabaseView();
	
	/**Element of the main view*/
	private MainView mainView = new MainView();
	
	/**Object to execute RPC-Calls to the server*/
	private RPCConnector rpcConnector = new RPCConnector();
	
	/**Service for RPC-Calls to the server in this class*/
	private SecondoServiceAsync secondoService = (SecondoServiceAsync) GWT.create(SecondoService.class);
	
	/**Popup showing an animation while loading data from the database*/
	private PopupPanel loadingPopup = new PopupPanel(true);
    private Image loadingImage = new Image("resources/images/loader1.gif");
    
	/**The message displayed to the user when the server cannot be reached or returns an error.*/
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";
	
	/**Command for the optimizer*/
	private Command optimizerSettings;
	
	/* Storage for temporary data*/
	private String currentDatabase="";
	private ArrayList<String> logindata = new ArrayList<String>();
	private ArrayList<String> databases = new ArrayList<String>();


	/**************************************************************
	 * This is the entry point method which starts the application.
	 **************************************************************/
	public void onModuleLoad() {
					
	/*set default values in login textfields*/
		//this.loginView.getUsername().setText("ksteiger");
		//this.loginView.getPassword().setText("s3c0nd0");
		this.loginView.getIpadresse().setText("agnesi.fernuni-hagen.de");
		this.loginView.getPort().setText("1302");		

	/*initialize the loading popup*/
	    loadingPopup.setAnimationEnabled(true);
	    loadingPopup.ensureDebugId("imagePopup");
	    loadingPopup.setWidget(loadingImage);
	    loadingPopup.setGlassEnabled(true);
	    loadingPopup.getElement().setClassName("loadingpopup");
		
/* *** Eventhandler for elements that need to change the main content of the application 
 * *** or need the rpcConnector, which can only be done in this class */

		/* Adding an Eventhandler to the Enter-Key in the Commandpanel */
		this.mainView.getCommandPanel().getTextArea().addKeyPressHandler(new KeyPressHandler() {
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					
					// delete everything before >
					String command = mainView.getCommandPanel().getTextArea().getText();
					int splitIndex = command.indexOf(">");
					command = command.substring(splitIndex+1);
					
					if(mainView.getCommandPanel().getMenubarCP().isOptimizerTurnedOn()){
						//get the optimized command from the optimizer and send this one to secondo
						rpcConnector.sendOptimizedCommand(command, currentDatabase.toLowerCase(), false, mainView, loadingPopup);					
					}
					else{
						//send the command directly to secondo
						rpcConnector.sendCommand(command, mainView, loadingPopup);
					}

					mainView.getTextView().getResultListBox().addItem(command);
					
					//show the loading popup in the center of the application until the call is finished
			    	loadingPopup.center(); 				
				}
			}
		});
		
		/*Adding an Eventhandler to the Enter-Key in the Passwordfield to log in*/
		this.loginView.getPassword().addKeyPressHandler(new KeyPressHandler() {
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					if(!logindata.isEmpty()){
			        	  logindata.clear();
			        }
		            	  //get the content of the login text fields
		            	  logindata.add(loginView.getUsername().getText());
		            	  logindata.add(loginView.getPassword().getText());
		            	  logindata.add(loginView.getIpadresse().getText());
		            	  logindata.add(loginView.getPort().getText());
		            	  
		            	//connect to secondo with logindata 
		            	  sendLogin(logindata);	
				}
			}
		});
		
		/* Adding an Eventhandler to the Login-Button */
		this.loginView.getLoginbutton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
        	      
	        	 if(!logindata.isEmpty()){
	        	  logindata.clear();
	        	  }

            	  //get the content of the login text fields
            	  logindata.add(loginView.getUsername().getText());
            	  logindata.add(loginView.getPassword().getText());
            	  logindata.add(loginView.getIpadresse().getText());
            	  logindata.add(loginView.getPort().getText());  

	              sendLogin(logindata);	
	            }
	          });
		
		/* Adding an Eventhandler to the Open Database Button*/
		this.databaseView.getOpenDatabaseButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  	            	  
	            	int selectedDatabase = databaseView.getMultiBox().getSelectedIndex();
	            	
	            	currentDatabase = databaseView.getMultiBox().getItemText(selectedDatabase);
	            	  
                 	openDatabase(databaseView.getMultiBox().getItemText(selectedDatabase));
	            }
	          });
		
		/* Adding an Eventhandler to the Import Database Button*/
		this.databaseView.getImportDatabaseButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  Window.alert("Coming soon :)");            	  
	            	
	            }
	          });
	    
	    /*Adds an event handler on the closedatabase button to close the database*/
		this.mainView.getMainheader().getClosedatabaseButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
                closeDatabase(currentDatabase);  //database from sessiondata?
		    	rpcConnector.deleteCommandHistory(mainView);
	          }
		 });
	    
		/*Adds an event handler on the logout button of the mainview to log out of the application*/	    
	    this.mainView.getMainheader().getLogoutButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  logout();
	        	  rpcConnector.deleteCommandHistory(mainView);
	          }
		 });
	    
	    
	    /*Adds an event handler on the save Button in the optimizer dialog of the commandpanel menubar to save the new connection data*/	    
	    this.mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getSavebutton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	//check if textfields are empty
	            if (mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getHost().getText().length() == 0
	        		|| mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getPort().getText().length() == 0) {
	        		Window.alert("Hostname or Portnumber cannot be empty."); 
	        	 }

	            else{
	            	
	              loadingPopup.center();         	  
	        	  rpcConnector.setOptimizerConnection(mainView.getCommandPanel().getMenubarCP().
	        			  getOptimizerDialog().getHost().getText(), 
	        			  Integer.parseInt(mainView.getCommandPanel().getMenubarCP().
	        					  getOptimizerDialog().getPort().getText()), mainView, loadingPopup);
	              }
	          }
		 });
	    
	    /*Creates a command to execute when optimizer settings are selected in the menubar of the commandpanel*/
		this.optimizerSettings = new Command() {

		      public void execute() {
		    	  
		    	  rpcConnector.getOptimizerConnection(mainView);
		    	  
		    	  //show dialogbox
		    	  mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getOptimizerDialogBox().center();
		    	  mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getOptimizerDialogBox().show();
		      }
		    };    	 
        this.mainView.getCommandPanel().getMenubarCP().getOptimizerItemSettings().setScheduledCommand(optimizerSettings);
	    
	    /*Adds an event handler on the downloadtextButton of the toolbar to download the text into a file*/
	    this.mainView.getToolbox().getDownloadTextLink().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	rpcConnector.saveTextFile(mainView.getTextView().getTextOutput().getText(), "secondo-text.txt");
	          }
		 });
	    
	    /*Adds an event handler on the downloadRawDataButton of the toolbar to download the raw data into a file*/
	    this.mainView.getToolbox().getDownloadRawDataLink().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	rpcConnector.saveTextFile(mainView.getRawDataView().getRawDataOutput().getText(), "secondo-text.txt");
	          }
		 });
	   
        
		/*sets default content after starting the application, which is the login-page*/
        //check for ie8 and display default message
        if(Window.Navigator.getUserAgent().contains("MSIE 8")){
		    System.out.println("###Browser version: " + Window.Navigator.getUserAgent());
		    HTML defaultText = new HTML("<p><h3>Your browser does not support Scalable Vector Graphihcs (SVG).<br>" +
		    		" Please upgrade to a modern browser.</h3></p>");
		    loginView.getMainPanel().remove(2); 
		    loginView.getMainPanel().insert(defaultText, 2);
		}
		this.setContent(0);
	}
	
/* ***Methods with RPC-Calls to the Application-Server for getting server-side Data which need 
 * ***the setContent-Method to change the page content*/

	
	/** Starts an RPC call to the server to send the logindata to secondo and verify it 
	 * 
	 * @param userDataList List with logindata of the user
	 * */
	public void sendLogin(ArrayList<String> userDataList) {
						  
		  AsyncCallback<Void> callback = new AsyncCallback<Void>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert("An error occurred while "
							+ "attempting to contact the server. Please check your logindata and try again.");					
				}

				@Override
				public void onSuccess(Void result) { 
						
						updateDatabaseList();		          			
				}
			  };
		  		 
	       // Make the call. Control flow will continue immediately and later 'callback' will be invoked when the RPC completes.
		  secondoService.setSecondoConnectionData(userDataList, callback); 
	}
	
	/** Starts an RPC call to the server to get the list of currently available databases */
	public void updateDatabaseList() {
						  
		  AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert("Your attempt to connect to Secondo was not successful. " +
							"\n\n Please check your logindata and try again.");					
				}

				@Override
				public void onSuccess(ArrayList<String> databaselist) {

					databaseView.getMultiBox().clear();				
					loginView.getPassword().setText("");
						            	  
	            	//set databasenames in the list
	                  for (String database:databaselist){
	          			databaseView.getMultiBox().addItem(database);
	          			databases.add(database);
	          		}		                  
					setContent(1);	
				}
			  };
		 secondoService.updateDatabaseList(callback); 
	}

	
	/** Starts an RPC call to the server to send the name of the database to secondo and open the database 
	 * 
	 * @param database The database to be opened
	 * */
	public void openDatabase(String database) {
						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(String openDatabase) { //result here is return from secondoserviceimpl method

          		Window.alert("Database " + openDatabase + " is opened successfully!");	
          		
				//set status info to status bar
          		mainView.getStatusBar().getSecondoServer().setText(logindata.get(2) + " : " + logindata.get(3));
          		mainView.getStatusBar().getUserName().setText(logindata.get(0));
          		mainView.getStatusBar().getOpenDatabase().setText(openDatabase );
          		
          		//set optimizerurl
          		rpcConnector.getOptimizerConnection(mainView);
          		 		
          		//delete data from last actions
      			mainView.getCommandPanel().getTextArea().setText("Sec >");	
      			mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().clear();
      			mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().addItem("Select Command...");
     			
      			rpcConnector.deleteCommandHistory(mainView);
      			
				//reset text views
      			mainView.getRawDataView().resetData();
          		mainView.getTextView().resetData();
      			
      			//delete data from map and graphical views
      			mainView.getGraphicalView().resetData();
      			mainView.getGraphicalView().getMpointController().stopAllAnimations();
      			mainView.getMapView().resetData();
      			mainView.getMapView().getMpointController().stopAllAnimations();
      			
      			//delete data from toolbar
      			mainView.getToolbox().resetData();
      			
      			rpcConnector.resetObjectCounter();
      			
				mainView.showGraphicalView();
          		
          		setContent(2);			
			}
		  };		  
		secondoService.openDatabase(database, callback); 
	}
	
	/** Starts an RPC call to the server to send the name of the database to secondo and close the database 
	 * 
	 * @param database The database to be closed
	 * */
	public void closeDatabase(String database) {
						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);			
			}

			@Override
			public void onSuccess(String database) { //result here is return from secondoserviceimpl method
          		
          		if(!database.equals("failed")){
          			Window.alert("Database " + database + " is successfully closed!");
    				setContent(1);	
    				
    				//delete data from map and graphical views
          			mainView.getGraphicalView().resetData();
          			mainView.getMapView().resetData();
          		}			
			}
		  };		  
		secondoService.closeDatabase(database, callback); 
	}
	
	/** Starts an RPC call to the server to log out of the application */
	public void logout() {
						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);	
			}

			@Override
			public void onSuccess(String result) { //result here is return from secondoserviceimpl method
          		
          		if(result.equals("ok")){
          			Window.alert("Thanks for using Secondo! Good bye!");

    				setContent(0);	
    				rpcConnector.deleteCommandHistory(mainView);
    				databaseView.getMultiBox().clear();

    				//reset text views		
    				mainView.getTextView().getTextOutput().setText("");
    				mainView.getRawDataView().getRawDataOutput().setText("");
    				
    				//reset graphical and map view
    				mainView.getMapView().resetData();
    				mainView.getGraphicalView().resetData();
          		}			
			}
		  };	  
		secondoService.logout(callback); 
	}
	
	/**Method to set 3 different contents to the HTML-Page, depending on the userstatus and the chosen database
	 * 
	 * @param status Integer value with the status number, 0 for loginview, 1 for databaseview and 2 for mainview
	 * */
	public void setContent(int status){
		switch(status){
        case 0:
            System.out.println("User is not logged in");
            header.clear();
			header.add(loginView.getLoginheader().gethPanel());
			
			content.clear();
			content.add(loginView.getMainPanel());
			
			footer.clear();
			footer.add(loginView.getLoginfooter().getHpanel());
			
			/* Associate the panels with the HTML host page.*/
			RootPanel.get("content").add(content);

			RootPanel.get("header").add(header);

			RootPanel.get("footer").add(footer);
            
            break;
        case 1:
            System.out.println("User is logged in but has not chosen a database");

            header.clear();
			header.add(databaseView.getDatabaseHeader().getHeaderPanel());
			
			content.clear();
			content.add(databaseView.gethPanel());
			
			footer.clear();
			footer.add(databaseView.getDatabaseFooter().getHpanel());		

		/* Associate the panels with the HTML host page.*/
		RootPanel.get("content").add(content);

		RootPanel.get("header").add(header);

		RootPanel.get("footer").add(footer);
		
         break;
        case 2:
            System.out.println("User is logged in and has chosen a database");
            header.clear();
			header.add(mainView.getMainheader().gethPanel());
			
			content.clear();
			content.add(mainView.getMainPanel());
			
			footer.clear();

		/* Associate the panels with the HTML host page.*/
		RootPanel.get("content").add(content);

		RootPanel.get("header").add(header);

            break;
        default:
            System.out.println("switch-case-defaulttext");
        } 		
	}	
}
