package com.secondo.webgui.client;

import java.util.ArrayList;
import java.util.Date;
import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.core.client.Scheduler;
import com.google.gwt.core.client.Scheduler.RepeatingCommand;
import com.google.gwt.core.client.Scheduler.ScheduledCommand;
import com.google.gwt.event.dom.client.ChangeEvent;
import com.google.gwt.event.dom.client.ChangeHandler;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyPressEvent;
import com.google.gwt.event.dom.client.KeyPressHandler;
import com.google.gwt.event.dom.client.MouseEvent;
import com.google.gwt.event.logical.shared.SelectionEvent;
import com.google.gwt.event.logical.shared.SelectionHandler;
import com.google.gwt.user.client.Command;
import com.google.gwt.user.client.DeferredCommand;
import com.google.gwt.user.client.IncrementalCommand;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.Window.Location;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.RootPanel;
import com.secondo.webgui.client.databaseview.DatabaseView;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.client.datatypes.Line;
import com.secondo.webgui.client.datatypes.Polyline;
import com.secondo.webgui.client.loginview.LoginFooter;
import com.secondo.webgui.client.loginview.LoginHeader;
import com.secondo.webgui.client.loginview.LoginView;
import com.secondo.webgui.client.mainview.Commandpanel;
import com.secondo.webgui.client.mainview.Header;
import com.secondo.webgui.client.mainview.MenuBarCommandPanel;
import com.secondo.webgui.client.mainview.StatusBar;
import com.secondo.webgui.client.mainview.TabView;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.client.rpc.SecondoServiceAsync;

/**
 * Entry point classes define onModuleLoad().
 */
public class SecondoWeb implements EntryPoint {

	/** Main Elements of the Website */
	private FlowPanel content = new FlowPanel();
	private FlowPanel header  = new FlowPanel();
	private FlowPanel footer = new FlowPanel();
	private FlowPanel statusbar = new FlowPanel();
	private Header mainheader = new Header();
	private TabView tabs = new TabView();
	private Commandpanel commandPanel = new Commandpanel();
	private LoginHeader loginheader = new LoginHeader();
	private LoginFooter loginfooter = new LoginFooter();
	private LoginView loginView = new LoginView();
	private DatabaseView databaseView = new DatabaseView();
	private StatusBar statusBar = new StatusBar();
	private MenuBarCommandPanel menubarCP = new MenuBarCommandPanel();
	//those 3 should not be needed, because of sessiondata-class
	private int status=0;
	private boolean loggedin = false;
	private String currentDatabase="";
	
	private SecondoServiceAsync secondoService = (SecondoServiceAsync) GWT.create(SecondoService.class);
	private ArrayList<String> logindata = new ArrayList<String>();
	private ArrayList<String> databases = new ArrayList<String>();
	//Commands for Navigation bar
	private Command closeDatabaseCommand;
	private Command logoutCommand;
	//Commands for MenuBar of CommandPanel
	private Command basicCommand1;
	private Command basicCommand2;
	private Command basicCommand3;

	/*The message displayed to the user when the server cannot be reached or returns an error.*/
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";


	/**
	 * This is the entry point method which starts the application.
	 */
	public void onModuleLoad() {
					
		//set default values in login textfields
		this.loginView.getUsername().setText("Tina");
		this.loginView.getPassword().setText("s3c0nd0");
		//this.loginView.getIpadresse().setText("192.168.186.138");
		this.loginView.getIpadresse().setText("agnesi.fernuni-hagen.de");
		//this.loginView.getPort().setText("1234");
		this.loginView.getPort().setText("1302");

		
/**#################### Eventhandler for Buttons, Tabs and Navigation that need to change the Contentview or make an RPC-Call, which can only be done in this class###########################*/

		/* Adding an Eventhandler to the Enter-Key in the Commandpanel */
		this.commandPanel.getTextArea().addKeyPressHandler(new KeyPressHandler() {
			public void onKeyPress(KeyPressEvent event) {
				boolean enterPressed = KeyCodes.KEY_ENTER == event
						.getNativeEvent().getKeyCode();
				if (enterPressed) {
					
					// delete everything before >
					String command = commandPanel.getTextArea().getText();
					int splitIndex = command.indexOf(">");
					command = command.substring(splitIndex+1);

					sendCommand(command);
					
				}

			}
		});
		
		/*Adding an Eventhandler on the second tab for building the formatted view*/
		this.tabs.getTabPanel().addSelectionHandler(new SelectionHandler<Integer>() {
			  @Override
			  public void onSelection(SelectionEvent<Integer> event) {
			    if (event.getSelectedItem() == 1) {
			    				    	
			    	setFormattedData();
			    	tabs.getFormattedView().getFormattedScrollPanel().scrollToTop();
			    }

			  }
			});
		
		/*Adding an Eventhandler on the graphical tab for adding formatted data to the graphical view*/
		this.tabs.getTabPanel().addSelectionHandler(new SelectionHandler<Integer>() {
			  @Override
			  public void onSelection(SelectionEvent<Integer> event) {
			    if (event.getSelectedItem() == 2 ) {
			    	
			    	//setFormattedData();
			    	getFirstTuplesList();
			    	//setPointOnMap();
			    	setGraphicalView();
			    	
			    }
			     //map view
			    if (event.getSelectedItem() == 3) {
			    	
			    	setFormattedData();
			    	tabs.getMapView().initializeMap(51.3760448, 7.4947253); //initialize the google map with center in hagen
			    	//setPointOnMap();
			    	setMapView();
				}
			    
			    //map view
			    if (event.getSelectedItem() == 4) {
			    	
			    	//setFormattedData();
			    	//tabs.getOsmView().initializeOSMMap(51.3760448, 7.4947253); //initialize the osm map with center in hagen

			    	//setMapView();
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
		this.menubarCP.getQuerySelection().addChangeHandler(new ChangeHandler() {
		      public void onChange(ChangeEvent event) {
	        	  
	        	  int selectedCommandIndex = menubarCP.getQuerySelection().getSelectedIndex();
	        	  
	        	  if(menubarCP.getQuerySelection().getItemText(selectedCommandIndex).equals("Select Command...")){			
					}
					else{	  						
						updateResult(menubarCP.getQuerySelection().getItemText(selectedCommandIndex));

					}       
	              }
	          });
	
		
		/*creates a command to execute when close database is selected in the navigation*/
		this.closeDatabaseCommand = new Command() {

		      public void execute() {
		    	  
		    	  closeDatabase(currentDatabase);  //database aus sessiondata
		    	  
		    	  deleteCommandHistory();
		      }
		    };    
		 
		this.mainheader.getNavigation().getCloseDatabase().setScheduledCommand(closeDatabaseCommand);
		
		
		/*creates a command to execute when logout is selected in the navigation*/
				this.logoutCommand = new Command() {

				      public void execute() {
				    	  
				    	logout();
				      }
				    };    
				 
		this.mainheader.getNavigation().getLogout().setScheduledCommand(logoutCommand);
		
		
		/*creates a command to execute when the first basic command is selected in the menubar of the commandpanel*/
		this.basicCommand1 = new Command() {

		      public void execute() {
		    	  
		    	sendCommand("query Stadt feed filter[.SName contains \"Bremen\"] consume");
		      }
		    };    
		 
        this.menubarCP.getBasicCommandItem1().setScheduledCommand(basicCommand1);
       
       /*creates a command to execute when the second basic command is selected in the menubar of the commandpanel*/
		this.basicCommand2 = new Command() {

		      public void execute() {
		    	  
		    	sendCommand("query Kreis feed filter[.KName contains \"Rosenheim\"] consume");
		      }
		    };    
		 
        this.menubarCP.getBasicCommandItem2().setScheduledCommand(basicCommand2);
      
      /*creates a command to execute when the third basic command is selected in the menubar of the commandpanel*/
		this.basicCommand3 = new Command() {

		      public void execute() {
		    	  
		    	sendCommand("query Autobahn feed filter[.AName contains \"6\"] consume");
		      }
		    };    
		 
        this.menubarCP.getBasicCommandItem3().setScheduledCommand(basicCommand3);

		
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
				commandPanel.getTextArea().setText("Your command was not accepted."  + "\n" + "Sec >");		
				System.out.println("Query failed: " + caught.getMessage());
			}

			@Override
			public void onSuccess(String result) { //result here is return from secondoserviceimpl method sendCommand
				//System.out.println("Call to secondo has been successful with the following result:" + result);

				String command = commandPanel.getTextArea().getText();
				int splitIndex = command.indexOf(">");
				command = command.substring(splitIndex+1);
				
				tabs.getSecondoOutput().setText(
						"Your entered command was :"+ command + 
						"\n" +"The result for your query to Secondo is : "+ "\n" + result) ;
				
				//move all scrollpanels to the top
				tabs.getStandardView().getScrollPanel().scrollToTop();
				tabs.getFormattedView().getFormattedScrollPanel().scrollToTop();
				tabs.getMapView().getScrollPanel().scrollToTop();
				tabs.getMapView().getTextPanel().scrollToTop();
				
				commandPanel.getTextArea().setText("Sec >");
				
				//getFormattedResult(); //falls man sich in einem anderen Tab befindet beim ausführen des befehls muss auch diese ansicht befüllt werden, andererseits verlangsamt das die standardausgabe enorm
				//setGraphicalData(); //bringt gar kein ergebnis
				
				updateCommandHistory();
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
				
				//reset all formatted views 
				//tabs.getFormattedView().getFormattedList().clear();
			//tabs.getGraphicalView().getFormattedList().clear();
				
				tabs.getFormattedOutput().setText("");
				//tabs.getGraphicalView().getFormattedOutput().setText("");
				tabs.getMapView().getFormattedOutput().setText("");

				String currentResult = "";
				String currentResult2 ="";
				String currentResult3 ="";
				
			/*	Scheduler.get().scheduleIncremental(new RepeatingCommand() {
					  
					private int counter = 0;
	                protected static final int MAX_LOOPS = 1000;
	                
				    public boolean execute() {
				      if (counter == MAX_LOOPS) {
				        //TODO if ready
				        return false;
				      }
				      //TODO if not ready
				      return true;
				    }
				  });*/
				
				//add data from formatted result list to all 3 formatted views
				for (String data : formattedResultList){	
					
					//tabs.getGraphicalView().getFormattedList().add(data);
					
					currentResult = tabs.getFormattedOutput().getText();
					tabs.getFormattedOutput().setText(currentResult + data);
					
					//currentResult2 = tabs.getGraphicalView().getFormattedOutput().getText();
					//tabs.getGraphicalView().getFormattedOutput().setText(currentResult2 + data);
					
					currentResult3 = tabs.getMapView().getFormattedOutput().getText();
					tabs.getMapView().getFormattedOutput().setText(currentResult3 + data);
				}
				
				/*tabs.getGraphicalView().getTextCellList().setRowCount(tabs.getGraphicalView().getFormattedList().size(), true);
			    tabs.getGraphicalView().getTextCellList().setRowData(0, tabs.getGraphicalView().getFormattedList());
				tabs.getGraphicalView().getTextCellList().redraw();*/
				
				// Add the new data to the dataProvider.
			  /*   tabs.getGraphicalView().getDataProvider().updateRowCount(tabs.getGraphicalView().getFormattedList().size(), true);
			     tabs.getGraphicalView().getDataProvider().updateRowData(0, tabs.getGraphicalView().getFormattedList());
			     tabs.getGraphicalView().getTextCellList().redraw();*/

				
				tabs.getStandardView().getScrollPanel().scrollToTop();
				tabs.getFormattedView().getFormattedScrollPanel().scrollToTop();
				tabs.getMapView().getScrollPanel().scrollToTop();
				tabs.getMapView().getTextPanel().scrollToTop();
				//tabs.getGraphicalView().getScrollPanel().scrollToTop();
				//tabs.getGraphicalView().getTextPanel().scrollToTop();
			}
		  };
		  
		secondoService.getFormattedResult(callback); 

		return "";
	}
	
	/** Starts an RPC call to the server, gets the formatted result of the first tuples and displays it in the view */
	public String getFirstTuplesList() {
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<String> firstTuplesList) { //result here is return from secondoserviceimpl method
				
				//clear the celllist
				tabs.getGraphicalView().getFormattedList().clear();

				//tabs.getGraphicalView().getFormattedOutput().setText("");

				//String currentResult = "";
				
				//add data from formatted result list to all 3 formatted views
				for (String data : firstTuplesList){	

					tabs.getGraphicalView().getFormattedList().add(data);				
					
					//currentResult= tabs.getGraphicalView().getFormattedOutput().getText();
					//tabs.getGraphicalView().getFormattedOutput().setText(currentResult + data);

				}
				
				/*tabs.getGraphicalView().getTextCellList().setRowCount(tabs.getGraphicalView().getFormattedList().size(), true);
			    tabs.getGraphicalView().getTextCellList().setRowData(0, tabs.getGraphicalView().getFormattedList());
				tabs.getGraphicalView().getTextCellList().redraw();*/
				
				// Add the new data to the dataProvider. Only needed for paging
			     tabs.getGraphicalView().getDataProvider().updateRowCount(tabs.getGraphicalView().getFormattedList().size(), true);
			     tabs.getGraphicalView().getDataProvider().updateRowData(0, tabs.getGraphicalView().getFormattedList());
			     tabs.getGraphicalView().getTextCellList().redraw();

				 tabs.getGraphicalView().getScrollPanel().scrollToTop();
				//tabs.getGraphicalView().getTextScrollPanel().scrollToTop();
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
				
          		//String homeText = tabs.getSecondoOutput().getText();
          		//select the first tab by default
          		tabs.getTabPanel().selectTab(0);
          		//clear views
          		tabs.getSecondoOutput().setText("");
          	    //enter some default text to the first tab
          		tabs.getSecondoOutput().setText(
          				"\n" + 
          				"Welcome " + logindata.get(0) + "!" + 
          				"\n \n Database " + openDatabase + " is opened successfully!");
          		
          		statusBar.getSecondoServer().setText(logindata.get(2) + " : " + logindata.get(3));
          		statusBar.getUserName().setText(logindata.get(0));
          		statusBar.getOpenDatabase().setText(openDatabase);
          		
          		//delete data from last actions
      			commandPanel.getTextArea().setText("Sec >");	
      			menubarCP.getQuerySelection().clear();
      			menubarCP.getQuerySelection().addItem("Select Command...");
      			
      			deleteCommandHistory();
				//reset all formatted views 
				//tabs.getFormattedView().getFormattedList().clear();
				tabs.getGraphicalView().getFormattedList().clear();
				
				tabs.getFormattedOutput().setText("");
				//tabs.getGraphicalView().getFormattedOutput().setText("");
				tabs.getMapView().getFormattedOutput().setText("");
          		
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
				Window.alert(SERVER_ERROR);
				
			}

			@Override
			public void onSuccess(String userstatus) { //result here is return from secondoserviceimpl method
          		
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
    				//reset all formatted views 
    				//tabs.getFormattedView().getFormattedList().clear();
    				tabs.getGraphicalView().getFormattedList().clear();
    				
    				tabs.getFormattedOutput().setText("");
    				//tabs.getGraphicalView().getFormattedOutput().setText("");
    				tabs.getMapView().getFormattedOutput().setText("");
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
				
				menubarCP.getQuerySelection().clear();
				menubarCP.getQuerySelection().addItem("Select Command...");
							
				for (String command : commandHistory){		
					menubarCP.getQuerySelection().addItem(command);
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
				
				menubarCP.getQuerySelection().clear();
				menubarCP.getQuerySelection().addItem("Select Command...");
				
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

				tabs.getSecondoOutput().setText("The result for your query to Secondo is : "+ "\n" + result) ;
				
				commandPanel.getTextArea().setText("Sec >");	
				
				setFormattedData();
			
			}
		  };
		  
		secondoService.updateResult(selectedCommand, callback); 
		

		return "";
	}
	
	/** Starts an RPC call to the server to get a list of datatypes from the secondoresult and sets them on the map */
	public String setMapView() {
						
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
					tabs.getMapView().deleteAllMarkers(); 
					tabs.getMapView().deleteAllLocations(); 
					tabs.getMapView().deleteAllLines();
					tabs.getMapView().deleteAllPolylines();
					tabs.getMapView().deleteAllPolygons();
					tabs.getMapView().resetBounds();
					tabs.getMapView().resetMap(); //delete all overlays
					//tabs.getMapView().initializeMap(51.3760448, 7.4947253); //coordinates of the center of the map
									
					for(DataType data : result){

						if(data.getName().equals("Point")){

							   tabs.getMapView().addMarker(((Point) data).getY(), ((Point) data).getX()); //add coordinates of point to the map
							
						}
						if(data.getName().equals("Line")){
							
							tabs.getMapView().addLine(((Line) data).getA().getY(), ((Line) data).getA().getX(), ((Line) data).getB().getY(), ((Line) data).getB().getX());
						}
                       if(data.getName().equals("Polyline")){
                    	   
                    	   tabs.getMapView().deleteAllLocations();
                    	   
                    	   //System.out.println("############## Datatype is Polyline");
                    	   
							for (Point point: ((Polyline) data).getPath()){
								tabs.getMapView().addLocation(point.getY(), point.getX()); //add point 	
								//System.out.println("##############point added to polyline");// klappt!
							}
							
							//tabs.getMapView().addPolyline();
							//tabs.getMapView().addPolygon();
							tabs.getMapView().drawPolygon();
							
						}
					}
					tabs.getMapView().showMarkerOverlays();
					tabs.getMapView().showLineOverlays();
					//tabs.getMapView().showPolylineOverlays();
					//tabs.getMapView().showPolygonOverlays();
					//tabs.getMapView().drawPolyline();
					//tabs.getMapView().drawPolygon();

				}
				
				else{
				tabs.getMapView().deleteAllMarkers(); 
				tabs.getMapView().deleteAllLocations();
				tabs.getMapView().deleteAllLines();
				tabs.getMapView().deleteAllPolylines();
				tabs.getMapView().deleteAllPolygons();
				tabs.getMapView().resetMap();
				//tabs.getMapView().initializeMap(51.3760448, 7.4947253); //build a map with fernuni hagen standard point
				System.out.println("##############else of setmapview");
				
				}
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
					tabs.getGraphicalView().deleteAllPoints();
					tabs.getGraphicalView().deleteAllLines();
					tabs.getGraphicalView().removeSVG();
					
					for(DataType data : result){
						if(data.getName().equals("Point")){

							tabs.getGraphicalView().addPointToDataset(((Point) data).getY(), ((Point) data).getX());
							
						}
						if(data.getName().equals("Line")){
							
							tabs.getGraphicalView().addLineToDataset(((Line) data).getA().getX(), ((Line) data).getA().getY(), ((Line) data).getB().getX(), ((Line) data).getB().getY());
						}
					}
					
					//get the first element of the list and check its type
					if(result.get(0).getName().equals("Point")){
						tabs.getGraphicalView().showPointArray(tabs.getGraphicalView().getDiv());
					}
					if(result.get(0).getName().equals("Line")){
						tabs.getGraphicalView().showLineArray(tabs.getGraphicalView().getDiv());
					}
					
				}
				
				else{
					
					System.out.println("##########else from getgraphicalview");
				
					tabs.getGraphicalView().deleteAllPoints();
					tabs.getGraphicalView().deleteAllLines();
					tabs.getGraphicalView().removeSVG();
				    tabs.getGraphicalView().addPointToDataset(51.3760448, 7.4947253);
				    tabs.getGraphicalView().showPointArray(tabs.getGraphicalView().getDiv());
				
				}
			}
		  };
		  
		secondoService.getResultTypeList(callback); 
		
		return "";
	}
	
	
	
/**################ Method to set 3 different contents to the website, depending on the userstatus and the chosen database ##############*/
	
	
	/**Sets the content on the HTML-Page depending on the userstatus*/
	public void setContent(int status){
		switch(status){
        case 0:
            System.out.println("User is not logged in");
            header.clear();
			header.add(loginheader.gethPanel());
			
			content.clear();
			content.add(loginView.getPanel());
			
			footer.clear();
			footer.add(loginfooter.gethPanel());
			
			/* Associate the panels with the HTML host page.*/
			RootPanel.get("content").add(content);

			RootPanel.get("header").add(header);

			RootPanel.get("footer").add(footer);
            
            break;
        case 1:
            System.out.println("User is logged in but has not chosen a database");

            header.clear();
			header.add(mainheader.gethPanel());
			
			content.clear();
			content.add(databaseView.gethPanel());
			
			footer.clear();
			footer.add(loginfooter.gethPanel());						

		/* Associate the panels with the HTML host page.*/
		RootPanel.get("content").add(content);

		RootPanel.get("header").add(header);

		RootPanel.get("commandpanel").add(footer);
		
         break;
        case 2:
            System.out.println("User is logged in and has chosen a database");
            header.clear();
			header.add(mainheader.gethPanel());
			
			content.clear();
			content.add(tabs.getTabPanel());
			
			footer.clear();
			footer.add(menubarCP.gethPanel());
			footer.add(commandPanel.gethPanel());
			footer.add(statusBar.gethPanel());


		/* Associate the panels with the HTML host page.*/
		RootPanel.get("content").add(content);

		RootPanel.get("header").add(header);

		RootPanel.get("commandpanel").add(footer);

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
