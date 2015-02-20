//This file is part of SECONDO.

//Copyright (C) 2014, University in Hagen, 
//Faculty of Mathematics and Computer Science 
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
import com.secondo.webgui.utils.config.Resources;
import com.secondo.webgui.utils.config.SecondoConstants;

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
	

	
	/* Storage for temporary data*/
	private String currentDatabase="";
	private ArrayList<String> logindata = new ArrayList<String>();
	private ArrayList<String> databases = new ArrayList<String>();


	/**************************************************************
	 * This is the entry point method which starts the application.
	 **************************************************************/
	public void onModuleLoad() {	
		 Resources.INSTANCE.css().ensureInjected(); 
					
	/*set default values in login textfields using properties*/
		//this.loginView.getUsername().setText("testuser");
		//this.loginView.getPassword().setText("s3c0nd0");
//		this.loginView.getIpadresse().setText("agnesi.fernuni-hagen.de");
//		this.loginView.getPort().setText("1302");	
		SecondoConstants constantsToConnect = GWT.create(SecondoConstants.class);
//		this.loginView.getIpadresse().setText(constantsToConnect.IP());
//		this.loginView.getPort().setText(constantsToConnect.port());
		
		if(!logindata.isEmpty()){
      	  logindata.clear();
      }
      	  //get the content of the login text fields  
			logindata.add("");
			logindata.add("");
      	  logindata.add(constantsToConnect.IP());
      	  logindata.add(constantsToConnect.port());
      	  
      	//connect to secondo with logindata 
      	  sendLogin(logindata, constantsToConnect.DB());	
      	  
      	  

	/*initialize the loading popup*/
	    loadingPopup.setAnimationEnabled(true);
	    loadingPopup.ensureDebugId("imagePopup");
	    loadingPopup.setWidget(loadingImage);
	    loadingPopup.setGlassEnabled(true);
	    loadingPopup.getElement().setClassName("loadingpopup");
		
/* *** Eventhandler for elements that need to change the main content of the application 
 * *** or need the rpcConnector, which can only be done in this class */

		
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
//		            	  sendLogin(logindata);	
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

//	              sendLogin(logindata);	
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
//		    	rpcConnector.deleteCommandHistory(mainView);
	          }
		 });
	    
		/*Adds an event handler on the logout button of the mainview to log out of the application*/	    
	    this.mainView.getMainheader().getLogoutButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  logout();
//	        	  rpcConnector.deleteCommandHistory(mainView);
	          }
		 });
	    
	  
	    /*Adds an event handler on the downloadRawDataButton of the toolbar to download the raw data into a file*/
	    this.mainView.getToolbox().getDownloadRawDataLink().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	rpcConnector.saveTextFile(mainView.getRawDataView().getRawDataOutput().getText(), "secondo-text.txt");
	          }
		 });
	    
	    /*allows to download a raw data result*/
	    this.mainView.getMainheader().getExport().setScheduledCommand(new Command() {
			
			@Override
			public void execute() {
				rpcConnector.saveTextFile(mainView.getMainheader().getTextViewOfTrajInDialog().getTextView().getText(), "secondo-text.txt");
				
			}
		});
	    /*Adds an event handler on the button "create symtraj"*/
		this.mainView.getOptionsTabPanel().getCreateSymTrajButton()
				.addClickHandler(new ClickHandler() {
					public void onClick(ClickEvent event) {
						if (mainView.getOptionsTabPanel()
								.getOptionsForCreatingSymTraj()
								.getSelectedIndex() == 2
								&& mainView.getMapView().getMyLocation() == null) {
							Window.alert("Please select my location using menu");
						} else {
							rpcConnector
									.doGPXimport(mainView.getOptionsTabPanel()
											.getNameOfUploadedFile(), mainView
											.getOptionsTabPanel()
											.getOptionsForCreatingSymTraj()
											.getSelectedIndex(), mainView,
											loadingPopup);
						}

					}
				});
	    
	    /**Adds an event handler on the button "get relation" of the options tab panel */
		this.mainView.getOptionsTabPanel().getAnimateButton().addClickHandler(new ClickHandler() {
			public void onClick(ClickEvent event) {
				String command=mainView.getOptionsTabPanel().getCommandForQueryRelation();
				if(!command.isEmpty()){
					mainView.resetMapView();
					
					//send the command directly to secondo
					rpcConnector.sendCommandAndUpdateHistory(command, mainView, loadingPopup);
					
					//show the loading popup in the center of the application until the call is finished
			    	loadingPopup.center(); 	

				}
				else{
					Window.alert("Please select relation");
				}
				
				
			}
		});
		
		//Adds an event handler on the button "match pattern" to match a defined pattern to the loaded relation
		this.mainView.getOptionsTabPanel().getMatchButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {
						if (!mainView.getOptionsTabPanel()
								.isUnsuccessfulVerification()) {
							String command = mainView.getOptionsTabPanel()
									.getCommandForPatternMatching();
							mainView.getOptionsTabPanel()
									.setPatternMatchingIsInitiated(true);
							if (!command.isEmpty()) {
								mainView.resetMapView();

								// send the command directly to secondo
								rpcConnector.sendCommandAndUpdateHistory(
										command, mainView, loadingPopup);

								// show the loading popup in the center of the
								// application until the call is finished
								loadingPopup.center();
							} else {
								Window.alert("Please select relation and load it");
							}

						} else {
							Window.alert("Please correct your pattern statement!");
						}
					}
				});
		
		//Adds an event handler on the button "get GPX coordinate" to define a location from address
		this.mainView.getMainheader().getLocationDialog().getGetCoordinateButton().addClickHandler(new ClickHandler() {
						@Override
			public void onClick(ClickEvent event) {
				String command=mainView.getMainheader().getCommandForGeocode();
				//send the command directly to secondo
				rpcConnector.getCoordinateFromAddress(command, mainView, loadingPopup);				
				
			}
		});
		
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesPanel().getQueryButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {

						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryPasses();
						if (!command.isEmpty()) {
							rpcConnector.sendSimpleQuery(command, "passes", mainView);
						} else {
							Window.alert("Please select relation and load it");
						}
					}
				});

		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getDeftimePanel().getQueryButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {

						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryDeftime();
						if (!command.isEmpty()) {
							rpcConnector.sendSimpleQuery(command, "deftime",mainView);
						} else {
							Window.alert("Please select relation and load it");
						}
					}
				});
		
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
		.getAtinstantPanel().getQueryButton()
		.addClickHandler(new ClickHandler() {

			@Override
			public void onClick(ClickEvent event) {

				String command = mainView.getOptionsTabPanel()
						.getCommandForSimpleQueryAtinstant();
				if (!command.isEmpty()) {
					rpcConnector.sendSimpleQuery(command, "atinstant",mainView);
				} else {
					Window.alert("Please select relation and load it");
				}
			}
		});
		
		this.mainView.getOptionsTabPanel().getSimpleQueriesStackPanel()
				.getPassesThroughRegionPanel().getQueryButton()
				.addClickHandler(new ClickHandler() {

					@Override
					public void onClick(ClickEvent event) {
						String command = mainView.getOptionsTabPanel()
								.getCommandForSimpleQueryPassesThroughRegion(mainView.getMapView().getDrawLayer());
						if (!command.isEmpty()) {
							rpcConnector.sendSimpleQuery(command, "passesThrough", mainView);							
						} else {
							Window.alert("Please select relation and load it");
						}

					}
				});
	   
	   
	    
		/*sets default content after starting the application, which is the login-page*/
        //check for ie8 and display default message
        if(Window.Navigator.getUserAgent().contains("MSIE 8")){
		    System.out.println("###Browser version: " + Window.Navigator.getUserAgent());
		    HTML defaultText = new HTML("<p><h3>Your browser does not support Scalable Vector Graphics (SVG).<br>" +
		    		" Please upgrade to a modern browser.</h3></p>");
		    loginView.getMainPanel().remove(2); 
		    loginView.getMainPanel().insert(defaultText, 2);
		}
//		this.setContent(0);
	}
	
/* ***Methods with RPC-Calls to the Application-Server for getting server-side Data which need 
 * ***the setContent-Method to change the page content*/

	
	/** Starts an RPC call to the server to send the logindata to secondo and verify it 
	 * 
	 * @param userDataList List with logindata of the user
	 * */
	public void sendLogin(ArrayList<String> userDataList, final String db) {
						  
		  AsyncCallback<Void> callback = new AsyncCallback<Void>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert("An error occurred while "
							+ "attempting to contact the server. Please check your logindata and try again.");					
				}

				@Override
				public void onSuccess(Void result) { 
						
//						updateDatabaseList();
					openDatabase(db);
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

//          		Window.alert("Database " + openDatabase + " is opened successfully!");	
          		
				//set database info to info dialog         		
				mainView.getMainheader().getDatabaseInfo().getHost()
						.setText(logindata.get(2));
				mainView.getMainheader().getDatabaseInfo().getPort()
						.setText(logindata.get(3));
				mainView.getMainheader().getDatabaseInfo().getDb()
						.setText(openDatabase);
          		
          		//set optimizerurl
          		rpcConnector.getOptimizerConnection(mainView);
          		 		
          		//delete data from last actions
//      			mainView.getCommandPanel().getTextArea().setText("Sec >");	
//      			mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().clear();
//      			mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().addItem("Select Command...");
     			
//      			rpcConnector.deleteCommandHistory(mainView);
      			
				//reset text views
      			mainView.getRawDataView().resetData();
          		
          		mainView.getMainheader().getTextViewOfTrajInDialog().resetData();
      			
      			//delete data from map and graphical views
      			mainView.getGraphicalView().resetData();
      			mainView.getGraphicalView().getMpointController().stopAllAnimations();
      			mainView.getMapView().resetData();
      			mainView.getMapView().getMpointController().stopAllAnimations();
      			
      			//delete data from toolbar
      			mainView.getToolbox().resetData();
      			
      			rpcConnector.resetObjectCounter();
      			
//				mainView.showGraphicalView();
				mainView.showMapView();
          		
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
//    				rpcConnector.deleteCommandHistory(mainView);
    				databaseView.getMultiBox().clear();

    				//reset text views		
    				
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
            mainView.getMainheader().getLabelWithDatabaseName().setText(currentDatabase);
            header.add(mainView.getMainheader().getGrid());
//			header.add(mainView.getMainheader().gethPanel());
						
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
