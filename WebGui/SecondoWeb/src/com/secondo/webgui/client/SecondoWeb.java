package com.secondo.webgui.client;

import java.util.ArrayList;
import java.util.Date;
import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.RootPanel;
import com.secondo.webgui.client.databaseview.DatabaseView;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.client.datatypes.Line;
import com.secondo.webgui.client.datatypes.Polyline;
import com.secondo.webgui.client.loginview.LoginView;
import com.secondo.webgui.client.mainview.MainView;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.client.rpc.SecondoServiceAsync;

/**
 * Entry point classes define onModuleLoad().
 */
public class SecondoWeb implements EntryPoint {

	/* Main Elements of the Website which represent the divs in the HTML-Page*/
	private FlowPanel header  = new FlowPanel();
	private FlowPanel content = new FlowPanel();
	private FlowPanel footer = new FlowPanel();
	
	/*Element of the Loginview*/
	private LoginView loginView = new LoginView();
	
	/*Element of the database view*/
	private DatabaseView databaseView = new DatabaseView();
	
	/*Elements of the main view*/
	private MainView mainView = new MainView();
	
	/* Storage for temporary data*/
	private int status=0;
	private boolean loggedin = false;
	private String currentDatabase="";
	private ArrayList<String> logindata = new ArrayList<String>();
	private ArrayList<String> databases = new ArrayList<String>();
	private ArrayList<String> currentFirstTuplesList = new ArrayList<String>();
	private String currentRawData = "";
	
	/*Commands for the Navigation bar*/
	private Command closeDatabaseCommand;
	private Command logoutCommand;
	
	/*Commands for MenuBar of the CommandPanel*/
	private Command basicCommand1;
	private Command basicCommand2;
	private Command basicCommand3;
	private Command basicCommand4;
	private Command basicCommand5;
	
	/*Service for RPC-Calls to the server*/
	private SecondoServiceAsync secondoService = (SecondoServiceAsync) GWT.create(SecondoService.class);

	/*The message displayed to the user when the server cannot be reached or returns an error.*/
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";


	/**
	 * This is the entry point method which starts the application.
	 */
	public void onModuleLoad() {
					
		//set default values in login textfields
		//this.loginView.getUsername().setText("ksteiger");
		//this.loginView.getPassword().setText("s3c0nd0");
		//this.loginView.getIpadresse().setText("192.168.186.138");
		this.loginView.getIpadresse().setText("agnesi.fernuni-hagen.de");
		//this.loginView.getPort().setText("1234");
		this.loginView.getPort().setText("1302");

		
/**#################### Eventhandler for Buttons, Tabs and Navigation that need to change the Contentview or make an RPC-Call, which can only be done in this class###########################*/

		/* Adding an Eventhandler to the Enter-Key in the Commandpanel */
		this.mainView.getCommandPanel().getTextArea().addKeyPressHandler(new KeyPressHandler() {
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					
					// delete everything before >
					String command = mainView.getCommandPanel().getTextArea().getText();
					/*String[] splits = command.split("Sec >");
					String split1 = splits[0]; //empty if nothing comes before
					String split2 = splits[1];//command without Sec >
					System.out.println("splits.size: " + splits.length + "splits 0: "+ split1 + "splits 1: "+split2 ); //Lenght=2
					command = split2;*/ //klappt so nicht
					int splitIndex = command.indexOf(">");
					command = command.substring(splitIndex+1);

					sendCommand(command);
					
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
            	  
            	            	  
            	  //check if username or password are empty
	              if (loginView.getUsername().getText().length() == 0
	        		|| loginView.getPassword().getText().length() == 0) {
	        		Window.alert("Username or password is empty."); 
	        	}

	              else{
	            	  //connect to secondo with logindata and return names of available databases if connection is successful  
	            	  sendLogin(logindata);	
	            	  
	              }
	            }
	          });
		
		/* Adding an Eventhandler to the Open Database Button*/
		this.databaseView.getOpenDatabaseButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  	            	  
	            	int selectedDatabase = databaseView.getMultiBox().getSelectedIndex();
	            	
	            	currentDatabase = databaseView.getMultiBox().getItemText(selectedDatabase); //TODO: not here but set in sessiondata
	            	  
                 	openDatabase(databaseView.getMultiBox().getItemText(selectedDatabase));
	            }
	          });

		
		/*Adding a ClickHandler to the selection box of the commandpanel to show the chosen query*/
		this.mainView.getCommandPanel().getMenubarCP().getQuerySelection().addChangeHandler(new ChangeHandler() {
		      public void onChange(ChangeEvent event) {
	        	  
	        	  int selectedCommandIndex = mainView.getCommandPanel().getMenubarCP().getQuerySelection().getSelectedIndex();
	        	  
	        	  if(mainView.getCommandPanel().getMenubarCP().getQuerySelection().getItemText(selectedCommandIndex).equals("Command History...")){			
					}
					else{	  						
						updateResult(mainView.getCommandPanel().getMenubarCP().getQuerySelection().getItemText(selectedCommandIndex));

					}       
	              }
	          });
		

	    /*Adds an event handler to the terminal button to show the terminal*/
	    this.mainView.getSidebar().getShowTerminalButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  	        	  
	        	  if(mainView.isTextTurnedOn()){
	        		  setFormattedData();
	        	  }
	        	  if(mainView.isMapTurnedOn()){
		        	  //setOsmMapView();
	        		  mainView.getOsmView().updateCurrentResult();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  }  
	        	  mainView.showCommandPanel();
	          }
		 });	
	    
	    /*Adds an event handler to the terminal button to hide the terminal*/
	    this.mainView.getSidebar().getHideTerminalButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {	        	 
	        	 
	        	  if(mainView.isMapTurnedOn()){
		        	  //setOsmMapView();
	        		  mainView.getOsmView().updateCurrentResult();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  } 
	        	  mainView.hideCommandPanel();
	          }
		 });
	    
	    /*Adds an event handler to the raw data button to show the raw data*/
	    this.mainView.getSidebar().getRawdataButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	  mainView.showRawDataView();
	      	  
	          }
		 });
	    
	    /*Adds an event handler to the show text button to show the text in the view*/
	    this.mainView.getSidebar().getShowTextButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {        	 

	        	  setFormattedData();

	        	  if(mainView.isMapTurnedOn()){
		        	  //setOsmMapView();
	        		  mainView.getOsmView().updateCurrentResult();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  } 
	        	  mainView.showTextView();
	          }
		 });
	    
	    /*Adds an event handler to the show text button to show the text in the view*/
	    this.mainView.getSidebar().getHideTextButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	  
	        	  if(mainView.isMapTurnedOn()){
		        	  //setOsmMapView();
	        		  mainView.getOsmView().updateCurrentResult();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  } 
	        	  mainView.hideTextView();
	          }
		 });
	    
	    /*Adds an event handler to the map button to show the map view*/
	    this.mainView.getSidebar().getMapButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	          	  
	        	  //setOsmMapView();
	        	  mainView.getOsmView().updateCurrentResult();
	        	  
	        	  if(mainView.isTextTurnedOn()){
	        		  setFormattedData();
	        	  }  
	        	  mainView.showMapView();
	          }
		 });
	        
	    
	    /*Adds an event handler to the geometry button to show the graphical view*/
	    this.mainView.getSidebar().getGeometryButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {

	        	  setGraphicalView();
	        	  if(mainView.isTextTurnedOn()){
	        		  setFormattedData();
	        	  } 
	        	  //getFirstTuplesList();
	        	  mainView.showGraphicalView();
	      	  
	          }
		 });
	    
	    
	    /*Adds an event handler on the closedatabase button to close the database*/
		this.mainView.getMainheader().getClosedatabaseButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
                closeDatabase(currentDatabase);  //database from sessiondata?
		    	deleteCommandHistory();
	          }
		 });
	    
		/*Adds an event handler on the logout button of the mainview to log out of the application*/	    
	    this.mainView.getMainheader().getLogoutButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  logout();
	        	  deleteCommandHistory();
	          }
		 });
	    
	    
	    /*Adds an event handler on the hideTerminalButton of the commandpanel menubar to hide the commandpanel*/	    
	    this.mainView.getCommandPanel().getMenubarCP().getHideTerminalButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  if(mainView.isMapTurnedOn()){
		        	  //setOsmMapView();
	        		  mainView.getOsmView().updateCurrentResult();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  } 
	        	  mainView.hideCommandPanel();
	        	  
	          }
		 });
	    
	    /*Adds an event handler on the clearTerminalButton of the commandpanel menubar to clear the commandpanel*/	    
	    this.mainView.getCommandPanel().getMenubarCP().getClearTerminalButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	  mainView.getCommandPanel().getTextArea().setText("Sec >");
	        	  
	          }
		 });
	    
	    /*Adds an event handler on the resetButton of the toolbar to clear the view*/
	    this.mainView.getToolbar().getResetButton().addClickHandler(new ClickHandler() {
	          public void onClick(ClickEvent event) {
	        	  
	        	 mainView.getGraphicalView().resetData();
	        	 mainView.getOsmView().resetData();
	        	  
	          }
		 });
	    	    
	    
		/*creates a command to execute when the first basic command is selected in the menubar of the commandpanel*/
		this.basicCommand1 = new Command() {

		      public void execute() {
		    	  
		    	  mainView.getCommandPanel().getTextArea().setText("Sec >query Stadt feed filter[.SName contains \"Bremen\"] consume");
		    	  
		    	//sendCommand("query Stadt feed filter[.SName contains \"Bremen\"] consume");
		      }
		    };    	 
        this.mainView.getCommandPanel().getMenubarCP().getBasicCommandItem1().setScheduledCommand(basicCommand1);
       
       /*creates a command to execute when the second basic command is selected in the menubar of the commandpanel*/
		this.basicCommand2 = new Command() {

		      public void execute() {
		    	  
		    	  mainView.getCommandPanel().getTextArea().setText("Sec >query Kreis feed filter[.KName contains \"LK Rosenheim\"] consume");
		    	  
		    	// sendCommand("query Kreis feed filter[.KName contains \"Rosenheim\"] consume");
		      }
		    };    		 
        this.mainView.getCommandPanel().getMenubarCP().getBasicCommandItem2().setScheduledCommand(basicCommand2);
      
      /*creates a command to execute when the third basic command is selected in the menubar of the commandpanel*/
		this.basicCommand3 = new Command() {

		      public void execute() {
		    	  
		    	  mainView.getCommandPanel().getTextArea().setText("Sec >query Autobahn feed filter[.AName contains \"6\"] consume");
		    	  
		    	  //sendCommand("query Autobahn feed filter[.AName contains \"6\"] consume");
		      }
		    };    		 
        this.mainView.getCommandPanel().getMenubarCP().getBasicCommandItem3().setScheduledCommand(basicCommand3);
        
        /*creates a command to execute when the third basic command is selected in the menubar of the commandpanel*/
		this.basicCommand4 = new Command() {

		      public void execute() {
		    	  
		    	  mainView.getCommandPanel().getTextArea().setText("Sec >query Stadt feed filter[.SName contains \"Bre\"] consume");
		    	  
		    	  //sendCommand("query Stadt feed filter[.SName contains \"Bre\"] consume");
		      }
		    };    		 
        this.mainView.getCommandPanel().getMenubarCP().getBasicCommandItem4().setScheduledCommand(basicCommand4);
        
        /*creates a command to execute when the third basic command is selected in the menubar of the commandpanel*/
		this.basicCommand5 = new Command() {

		      public void execute() {
		    	  
		    	  mainView.getCommandPanel().getTextArea().setText("Sec >query Flaechen feed filter[.Name contains \"Grunewald\"] consume");
		    	  
		    	  //sendCommand("query Stadt feed filter[.SName contains \"Bre\"] consume");
		      }
		    };    		 
        this.mainView.getCommandPanel().getMenubarCP().getBasicCommandItem5().setScheduledCommand(basicCommand5);

		
		/*sets default content after starting the application, which is the login-page, if the user is not logged in*/
		if(isLoggedIn() == false){
		this.setContent(0);
		}
		

	}
	
/**############################### Methods with RPC-Calls to the Application-Server for getting server-side Data #########################################*/

	/** Starts an RPC call to the server to send the command from the commandpanel to secondo and get the result back */
	public String sendCommand(String command) {
								
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				//Window.alert(SERVER_ERROR);			
				mainView.getCommandPanel().getTextArea().setText("Your command was not accepted."  + "\n" + "Sec >");		
				System.out.println("Query failed: " + caught.getMessage());
			}

			@Override
			public void onSuccess(String result) { //result here is return from secondoserviceimpl method sendCommand

				currentRawData = result;
				
				String command = mainView.getCommandPanel().getTextArea().getText();
				int splitIndex = command.indexOf(">");
				command = command.substring(splitIndex+1);
				
				mainView.getRawDataView().getRawDataOutput().setText(
						"Raw Data in Nested List Format from Secondo Database System \n\n" 
				         +"The result for your query to Secondo is : "+ "\n" + result) ;
				
				//move all scrollpanels to the top and reset data
				mainView.getRawDataView().getScrollPanel().scrollToTop();

				//mainView.getCommandPanel().getTextArea().setText(command + "\nSec >");
				mainView.getCommandPanel().getTextArea().setText("Sec >");
				//mainView.getCommandPanel().getTextArea().appendText("\nSec>");
				mainView.getTextView().getTextOutput().setText("");
				
				updateCommandHistory();
				
				  if(mainView.isTextTurnedOn()){
	        		  setFormattedData();
	        	  }

		         setOsmMapView();
		         
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  }
			}
		  };
		  		 
		secondoService.sendCommand(command, callback); 

		return "";
	}
	
	/** Starts an RPC call to the server, gets the formatted result and displays it in the view */
	public String setFormattedData() {
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<String> formattedResultList) { //result here is return from secondoserviceimpl method
				
				mainView.getTextView().setCurrentTextList(formattedResultList);

				mainView.getTextView().updateCurrentResult();
				
				//reset text views
				/*mainView.getTextView().getTextOutput().setText("");

				String currentResult = "";
				
				//add data from formatted result list to all 3 formatted views
				for (String data : formattedResultList){	
					
					currentResult = mainView.getTextView().getTextOutput().getText();
					mainView.getTextView().getTextOutput().setText(currentResult + data);

				}*/
			}
		  };
		  
		secondoService.getFormattedResult(callback); 

		return "";
	}
	
	/** Starts an RPC call to the server, gets the formatted result of the first tuples and displays it in the celllist text view */
	public String getFirstTuplesList() {
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<String> firstTuplesList) {
				
				currentFirstTuplesList = firstTuplesList;
				
				//clear the celllist
				mainView.getCellListTextView().getFormattedList().clear();

				//add data from formatted result list to all 3 formatted views
				for (String data : firstTuplesList){	

					mainView.getCellListTextView().getFormattedList().add(data);				

				}
				
				// Add the new data to the dataProvider. Only needed for paging
			     mainView.getCellListTextView().getDataProvider().updateRowCount(mainView.getCellListTextView().getFormattedList().size(), true);
			     mainView.getCellListTextView().getDataProvider().updateRowData(0, mainView.getCellListTextView().getFormattedList());
			     mainView.getCellListTextView().getTextCellList().redraw();

			}
		  };
		  
		secondoService.getFirstTuplesOfValues(callback); 

		return "";
	}
	
	/** Starts an RPC call to the server to send the logindata to secondo and verify it */
	public String sendLogin(ArrayList<String> userDataList) {
						  
		  AsyncCallback<ArrayList<String>> callback3 = new AsyncCallback<ArrayList<String>>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert(SERVER_ERROR);					
				}

				@Override
				public void onSuccess(ArrayList<String> databaselist) { //databaselist is returned from secondoserviceimpl method getSecondoConnection

					databaseView.getMultiBox().clear();
						            	  
	            	//set databasenames in the list
	                  for (String database:databaselist){
	          			databaseView.getMultiBox().addItem(database);
	          			databases.add(database);
	          		}	                 
						loggedin = true; 
						loginView.getPassword().setText("");
	      	    	    setContent(1);		
	          			
				}
			  };
		  		 
			// Make the call. Control flow will continue immediately and later 'callback' will be invoked when the RPC completes.
			  secondoService.setSecondoConnectionData(userDataList, callback3); 

		return "";
	}
	
	/** Starts an RPC call to the server to send the name of the database to secondo and open the database */
	public String openDatabase(String database) {
						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(String openDatabase) { //result here is return from secondoserviceimpl method

          		Window.alert("Database " + openDatabase + " is opened successfully!");	
				
				//reset text views
          		mainView.getRawDataView().getRawDataOutput().setText("");
				//mainView.getCellListTextView().getFormattedList().clear();		
				mainView.getTextView().getTextOutput().setText("");
				
          	    //enter some default text to the raw data view
          		/*mainView.getRawDataView().getRawDataOutput().setText(
          				"\n" + 
          				"Welcome " + logindata.get(0) + "!" + 
          				"\n \n Database " + openDatabase + " is opened successfully!");*/
          		
				//set status info to status bar
          		mainView.getStatusBar().getSecondoServer().setText(logindata.get(2) + " : " + logindata.get(3));
          		mainView.getStatusBar().getUserName().setText(logindata.get(0));
          		mainView.getStatusBar().getOpenDatabase().setText(openDatabase);
          		
          		//delete data from last actions
      			mainView.getCommandPanel().getTextArea().setText("Sec >");	
      			mainView.getCommandPanel().getMenubarCP().getQuerySelection().clear();
      			mainView.getCommandPanel().getMenubarCP().getQuerySelection().addItem("Select Command...");
     			
      			deleteCommandHistory();
          		
          		setContent(2);			
			}
		  };
		  
		secondoService.openDatabase(database, callback); 

		return "";
	}
	
	/** Starts an RPC call to the server to check if the user is logged in */
	public boolean isLoggedIn() {

						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR + caught);
				
			}

			@Override
			public void onSuccess(String userstatus) { 
          		
          		if(userstatus.equals("true")){
    				loggedin = true;
          		}			
			}
		  };
		  
		secondoService.isLoggedIn(callback); 

		return loggedin;
	}
	
	/** Starts an RPC call to the server to send the name of the database to secondo and close the database */
	public String closeDatabase(String database) {
						
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
          		}			
			}
		  };
		  
		secondoService.closeDatabase(database, callback); 

		return "";
	}
	
	/** Starts an RPC call to the server to log out of the application */
	public String logout() {
						
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
    				deleteCommandHistory();

    				//reset text views
    				//mainView.getCellListTextView().getFormattedList().clear();			
    				mainView.getTextView().getTextOutput().setText("");
    				mainView.getRawDataView().getRawDataOutput().setText("");
          		}			
			}
		  };
		  
		secondoService.logout(callback); 
		return "";
	}
	
	/** Starts an RPC call to the server to get the command history and update the dropdownlist for command history*/
	public String updateCommandHistory() {
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<String> commandHistory) { //result here is return from secondoserviceimpl method
				
				mainView.getCommandPanel().getMenubarCP().getQuerySelection().clear();
				mainView.getCommandPanel().getMenubarCP().getQuerySelection().addItem("Command History...");
							
				for (String command : commandHistory){		
					mainView.getCommandPanel().getMenubarCP().getQuerySelection().addItem(command);
				}
				
			}
		  };
		  
		secondoService.getCommandHistory(callback); 

		return "";
	}
	
	/** Starts an RPC call to the server to delete the command history in the sessiondata-object and clear the dropdownlist for command history*/
	public String deleteCommandHistory(){
		
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(String result) { //result here is return from secondoserviceimpl method
				
				mainView.getCommandPanel().getMenubarCP().getQuerySelection().clear();
				mainView.getCommandPanel().getMenubarCP().getQuerySelection().addItem("Command History...");
				
				System.out.println("#######CommandHistory has been deleted.");
			}
		  };
		  
		secondoService.deleteCommandHistory(callback); 

		return "";
	}
	
	/** Starts an RPC call to the server to get the result history and update the result in Standardview */
	public String updateResult(String selectedCommand) {
						
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(String result) { //result here is return from secondoserviceimpl method

				mainView.getRawDataView().getRawDataOutput().setText(
						"Raw Data in Nested List Format from Secondo Database System \n\n" 
				         +"The result for your query to Secondo is : "+ "\n" + result) ;
				
				mainView.getCommandPanel().getTextArea().setText("Sec >");	
				
				  if(mainView.isTextTurnedOn()){
	        		  setFormattedData();
	        	  }
	        	  if(mainView.isMapTurnedOn()){
		        	  setOsmMapView();
	        	  }
	        	  if(!mainView.isMapTurnedOn()){
		        	  setGraphicalView();
	        	  }
			}
		  };
		  
		secondoService.updateResult(selectedCommand, callback); 		

		return "";
	}
	
	/** Starts an RPC call to the server to get a list of datatypes from the secondoresult and sets them on the map */
	public String setOsmMapView() {
						
		AsyncCallback<ArrayList<DataType>> callback = new AsyncCallback<ArrayList<DataType>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<DataType> result) { //result here is return from secondoserviceimpl method
				
				mainView.getOsmView().setCurrentResultTypeList(result);
				
				mainView.getOsmView().updateCurrentResult();

				/*if(!result.isEmpty()){
					System.out.println("##############if of setmapview, size of resultlist: "+ result.size());

					mainView.getOsmView().resetData();
									
					for(DataType data : result){

						if(data.getName().equals("Point")){

							mainView.getOsmView().setMarker(((Point) data).getY(), ((Point) data).getX());
							mainView.getOsmView().addMarker(((Point) data).getY(), ((Point) data).getX());
							
						}
						if(data.getName().equals("Line")){
							
							mainView.getOsmView().addLine(((Line) data).getA().getY(), ((Line) data).getA().getX(), ((Line) data).getB().getY(), ((Line) data).getB().getX());
						}
                       if(data.getName().equals("Polyline")){
                    	   
                    	   mainView.getOsmView().deleteAllPoints();
                    	   
							for (Point point: ((Polyline) data).getPath()){
								mainView.getOsmView().addPoint(point.getY(), point.getX());	
							}	
							mainView.getOsmView().addPolygon();	
						
						}
					}
					//get the first element of the list and check its type
					if(result.get(0).getName().equals("Point")){
					    mainView.getOsmView().showMarkerOverlay();
					    }
					if(result.get(0).getName().equals("Line")){
					    mainView.getOsmView().showLineOverlay();
					    }
					if(result.get(0).getName().equals("Polyline")){
					    mainView.getOsmView().showPolygonOverlays();
					    }
				}
				
				else{
				
				System.out.println("##############else of setmapview");
				
				mainView.getOsmView().resetData();				
				}*/
			}
		  };
		  
		secondoService.getResultTypeList(callback); 
		
		return "";
	}
	
	/** Starts an RPC call to the server to get a list of datatypes from the secondoresult and sets them on the map */
	public String setGraphicalView() {
						
		AsyncCallback<ArrayList<DataType>> callback = new AsyncCallback<ArrayList<DataType>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<DataType> result) { //result here is return from secondoserviceimpl method

				if(!result.isEmpty()){
					
					System.out.println("##########if from getgraphicalview");
						
					//initializes the d3view
					mainView.getGraphicalView().resetData();
					
					//mainView.getGraphicalView().createSVG(); //wird nicht angesprochen von drawpolygon, und axen fehlen

					
					for(DataType data : result){
						if(data.getName().equals("Point")){

							mainView.getGraphicalView().addPointToDataset(((Point) data).getX(), ((Point) data).getY());
							
						}
						if(data.getName().equals("Line")){
							
							mainView.getGraphicalView().addLineToDataset(((Line) data).getA().getX(), ((Line) data).getA().getY(), ((Line) data).getB().getX(), ((Line) data).getB().getY());
						}
						
						if(data.getName().equals("Polyline")){
	                    	   
							//mainView.getGraphicalView().deleteAllPoints();
							//mainView.getGraphicalView().deleteAllPathPoints();
	                    	   
								for (Point point: ((Polyline) data).getPath()){
									mainView.getGraphicalView().addPointToDataset(point.getX(), point.getY());	//for all polygons to calculate the axis and scale
									mainView.getGraphicalView().addPointToPath(point.getX(), point.getY());	 //just for one polygon
								}	
								mainView.getGraphicalView().addPolygon();
								//mainView.getGraphicalView().drawPolygon(); //draws all polygons in 3 svgs!!
								
							}
					}
					
					//get the first element of the list and check its type
					if(result.get(0).getName().equals("Point")){
						mainView.getGraphicalView().showPointArray();
					}
					if(result.get(0).getName().equals("Line")){
						mainView.getGraphicalView().showLineArray();
					}
					if(result.get(0).getName().equals("Polyline")){
						mainView.getGraphicalView().showPaths();
						//mainView.getGraphicalView().drawPolygon(); // Funktioniert, zeichnet alle Polygone als 1 array
					}
					
				}
				
				else{
					
					System.out.println("##########else from getgraphicalview");
				
					mainView.getGraphicalView().resetData();
				
				}
			}
		  };
		  
		secondoService.getResultTypeList(callback); 
		
		return "";
	}
	
	/** Starts an RPC call to the server to get a list of datatypes from the secondoresult and sets them on the map */
	/*public String setMapView() {
						
		AsyncCallback<ArrayList<DataType>> callback = new AsyncCallback<ArrayList<DataType>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<DataType> result) { //result here is return from secondoserviceimpl method

				if(!result.isEmpty()){
					System.out.println("##############if of setmapview, size of resultlist: "+ result.size());//klappt auch beim 2. Mal
					//deletes former data from googlemap
					mainView.getMapView().deleteAllMarkers(); 
					mainView.getMapView().deleteAllLocations(); 
					mainView.getMapView().deleteAllLines();
					mainView.getMapView().deleteAllPolylines();
					mainView.getMapView().deleteAllPolygons();
					mainView.getMapView().resetBounds();
					mainView.getMapView().resetMap(); //delete all overlays
									
					for(DataType data : result){

						if(data.getName().equals("Point")){

							mainView.getMapView().addMarker(((Point) data).getY(), ((Point) data).getX()); //add coordinates of point to the map
							
						}
						if(data.getName().equals("Line")){
							
							mainView.getMapView().addLine(((Line) data).getA().getY(), ((Line) data).getA().getX(), ((Line) data).getB().getY(), ((Line) data).getB().getX());
						}
                       if(data.getName().equals("Polyline")){
                    	   
                    	   mainView.getMapView().deleteAllLocations();
                    	   
                    	   //System.out.println("############## Datatype is Polyline");
                    	   
							for (Point point: ((Polyline) data).getPath()){
								mainView.getMapView().addLocation(point.getY(), point.getX()); //add point 	
								//System.out.println("##############point added to polyline");// klappt!
							}
							
							//tabs.getMapView().addPolyline();
							//tabs.getMapView().addPolygon();
							mainView.getMapView().drawPolygon();
							
						}
					}
					mainView.getMapView().showMarkerOverlays();
					mainView.getMapView().showLineOverlays();
					//tabs.getMapView().showPolylineOverlays();
					//tabs.getMapView().showPolygonOverlays();
					//tabs.getMapView().drawPolyline();
					//tabs.getMapView().drawPolygon();

				}
				
				else{
					mainView.getMapView().deleteAllMarkers(); 
					mainView.getMapView().deleteAllLocations();
					mainView.getMapView().deleteAllLines();
					mainView.getMapView().deleteAllPolylines();
					mainView.getMapView().deleteAllPolygons();
					mainView.getMapView().resetMap();
				
				System.out.println("##############else of setmapview");
		
				}

			}
		  };
		  
		secondoService.getResultTypeList(callback); 
		
		return "";
	}*/
	
	
/**################ Method to set 3 different contents to the website, depending on the userstatus and the chosen database ##############*/
	
	
	/**Sets the content on the HTML-Page depending on the userstatus*/
	public void setContent(int status){
		switch(status){
        case 0:
            System.out.println("User is not logged in");
            header.clear();
			header.add(loginView.getLoginheader().gethPanel());
			
			content.clear();
			content.add(loginView.getPanel());
			
			footer.clear();
			footer.add(loginView.getLoginfooter().getImpressum());
			
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
			//footer.add(mainView.getFooter().getText());


		/* Associate the panels with the HTML host page.*/
		RootPanel.get("content").add(content);

		RootPanel.get("header").add(header);

		//RootPanel.get("footer").add(footer);

            break;
        default:
            System.out.println("switch-case-defaulttext");
        } 
		
	}

	/**pauses the application for a number of seconds to wait for the server to answer for example*/
	public void pause(int seconds){
	    Date start = new Date();
	    Date end = new Date();
	    while(end.getTime() - start.getTime() < seconds * 1000){
	        end = new Date();
	    }
	}
	
	
	/**restarts the whole application*/
	public void restartApp(){
		Window.Location.reload(); 
		
	}
}
