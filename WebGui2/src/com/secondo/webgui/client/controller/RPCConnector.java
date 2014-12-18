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

package com.secondo.webgui.client.controller;

import java.util.ArrayList;

import org.gwtopenmaps.openlayers.client.popup.Popup;

import com.google.gwt.core.client.GWT;
import com.google.gwt.i18n.client.NumberFormat;
import com.google.gwt.user.client.Timer;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.FormPanel;
import com.google.gwt.user.client.ui.PopupPanel;
import com.secondo.webgui.client.mainview.MainView;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.client.rpc.SecondoServiceAsync;
import com.secondo.webgui.shared.model.DataType;

/**
 * This class is responsible for RPC-Calls to the server side of the application.
 * 
 * @author Kristina Steiger
 */
public class RPCConnector {
	
	/**Service object for RPC-Calls to the server*/
	private SecondoServiceAsync secondoService = (SecondoServiceAsync) GWT.create(SecondoService.class);
	private MainView mainView;
	private PopupPanel loadingPopup;
	private String currentCommand = "";
	
	/**The standard message displayed to the user when the server cannot be reached or returns an error.*/
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";
	
	public RPCConnector(){
		
	}
	
/*###################### Methods with RPC-Calls to the Application-Server for getting server-side Data #########################################*/

	/** Starts an RPC call to the server to send the command from the commandpanel to secondo and get the result back 
	 * 
	 * @param command The command to be send to the secondo server
	 * @param mv The main view object
	 * @param lp The loading popup object
	 * */
	public void sendCommand(String command, MainView mv, PopupPanel lp) {
		
		this.mainView = mv;
		this.loadingPopup = lp;
		this.currentCommand = command;
								
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {				
				
				loadingPopup.hide();
				System.err.println("Query failed: " + caught.getMessage());
			}

			@Override
			public void onSuccess(String result) {
				
				//result is return from secondoserviceimpl method				
				if(result.startsWith("Error")){
//					mainView.getCommandPanel().getTextArea().setText(result  + "\n" + "Sec >");
					loadingPopup.hide();
				}
				else{
					
					if (mainView.getOptionsTabPanel()
							.isPatternMatchingIsInitiated()) {
						
						setNumberOfTuplesInRelationFromResultList(mainView);
					}	
				
				    updateCommandHistory(mainView);			    
				    
				
				   //put secondo data into the corresponding views
	        	    setTextView(mainView, loadingPopup);
					

	        	   //get datatype resultlist for map
	        	    getDatatypeResultList(mainView, loadingPopup);			        
		           
		            
		              //start timer to wait for data being loaded
			   	       Timer timer = new Timer() {  
			   	    	
			   	    	private int counter = 0; 
			   	    	private int maxCount = 100;
			   	    	
			            @Override
			            public void run() {
			 
			            //If data is loaded or maxtime is reached close loading animation and stop timer
			           	 if (counter >= maxCount) {						        	   				
			   				loadingPopup.hide();  
		                    cancel();
			                return;
			              }
			           	 //check if data in map and graphical view is loaded
			           	 if(mainView.isMapTurnedOn()){
			           		 
			           	  if(mainView.getMapView().isDataLoaded()){
			           		 //if textpanel is turned on, wait for text to be loaded
			           		  if(mainView.isTextTurnedOn()){
			           			  if(mainView.getMainheader().getTextViewOfTrajInDialog().isDataLoaded() && mainView.getMapView().isDataLoaded()){
			           				 counter = maxCount;				           		     
			           			  }			           			  
			           		  }
			           		  else{
			           		     counter = maxCount;
			           		     
			           		     
			           		  }
			             	}
			           	   counter++;
			           	 }
			           	 //Graphical view is shown
			           	 else{
			           		if(mainView.getGraphicalView().isDataLoaded()){
				           		 //if textpanel is turned on, wait for text to be loaded
				           		 if(mainView.isTextTurnedOn()){
				           			  if(mainView.getMainheader().getTextViewOfTrajInDialog().isDataLoaded() && mainView.getGraphicalView().isDataLoaded()){
				           				 counter = maxCount;
					           		    
				           			  }			           			  
				           		  }
				           		  else{
				           		     counter = maxCount;
				           		    
				           		     System.out.println("###GraphicalView data ist loaded!");
				           		  }
				             	}
				           	counter++;			           		 
			           	 }
			             }
			            };
			           timer.scheduleRepeating(500); 
				}   
			}
		  };		  		 
		secondoService.sendCommand(command, callback); 
	}
	
	
	
	/** Starts an RPC call to the server to set the connection data for the optimizer 
	 * 
	 * @param Host Server URL of the optimizer server
	 * @param Port Port of the optimizer server
	 * @param mv The main view object
	 * @param lp The loading popup object
	 * */
	public void setOptimizerConnection(String Host, int Port, MainView mv, PopupPanel lp) {
		
		this.mainView = mv;
		this.loadingPopup = lp;
						  
		  AsyncCallback<String> callback = new AsyncCallback<String>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert("An error occurred while "
							+ "attempting to contact the optimizer.");		
					loadingPopup.hide(); 
				}

				@Override
				public void onSuccess(String result) {
					
              	      loadingPopup.hide();
              	      
              	      if(!result.isEmpty()){

                      System.out.println("####### Connection Errormessage: "+ result);	
                      if(result.equals("no error")){
                    	  //everything is okay
                    	  Window.alert("Test of Optimizer Connection... successful!"); 
                    	  //set the new optimizerurl to the statusbar
                    		getOptimizerConnection(mainView);
                      }
                      else{
                    	  
                    	  System.out.println("#################error in setoptimizerconnection");
                    	  Window.alert("Test of Optimizer Connection... failed! \nCheck your connection data and try again. " +
                    	  		"\nMake sure the chosen optimizer is running.");
                        }
              	      }
				}
			  };	  		 
			// Make the call. Control flow will continue immediately and later 'callback' will be invoked when the RPC completes.
			  secondoService.setOptimizerConnection(Host, Port, callback); 
	}
	
	/** Starts an RPC call to the server to get the connection data of the optimizer 
	 * 
	 * @param mv The main view object
	 * */
	public void getOptimizerConnection(MainView mv) {
		
		this.mainView = mv;
						  
		  AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert("An error occurred while "
							+ "attempting to contact the optimizer. Please check your connection data and try again.");					
				}

				@Override
				public void onSuccess(ArrayList<String> result) {
					if(!result.isEmpty()){			
//				    	mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getHost().setText(result.get(0));
//				    	mainView.getCommandPanel().getMenubarCP().getOptimizerDialog().getPort().setText(result.get(1));
				    	mainView.getStatusBar().getOptimizer().setText(result.get(0) + " : " + result.get(1));
					}
				}
			  };	  		 
			// Make the call. Control flow will continue immediately and later 'callback' will be invoked when the RPC completes.
			  secondoService.getOptimizerConnectionData(callback); 
	}	
	
	/** Starts an RPC call to the server to get the command history and update the dropdownlist for command history
	 * 
	 * @param mv The main view object
	 * */
	public void updateCommandHistory(MainView mv) {
		
		this.mainView = mv;
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(ArrayList<String> commandHistory) { 
				
//				mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().clear();
//				mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().addItem("Command History...");
							
				for (String command : commandHistory){		
//					mainView.getCommandPanel().getMenubarCP().getCommandHistoryBox().addItem(command);
				}				
			}
		  };		  
		secondoService.getCommandHistory(callback); 
	}
	
	/** Starts an RPC call to the server to add a command to the command history in the sessiondata-object
	 * 
	 * @param command The command to be added to the commandhistory.
	 * */
	public void addCommandToHistory(String command){

		AsyncCallback<Void> callback = new AsyncCallback<Void>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);				
			}

			@Override
			public void onSuccess(Void result) {
				
				System.out.println("####### Command is added to CommandHistory.");
			}
		  };		  
		secondoService.addCommandToHistory(command, callback); 
	}
	
	
	
	/** Starts an RPC call to the server, gets the formatted text result and displays it in the text view 
	 * 
	 * @param mv The main view object
	 * */
	public void setTextView(MainView mv, PopupPanel lp) {
		
		this.mainView = mv;
		this.loadingPopup = lp;
						
		AsyncCallback<ArrayList<String>> callback = new AsyncCallback<ArrayList<String>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);		
				loadingPopup.hide();
				mainView.getMainheader().getTextViewOfTrajInDialog().setDataLoaded(true);
			}

			@Override
			public void onSuccess(ArrayList<String> textResultList) {
				
				if(!textResultList.isEmpty()){					
					
					mainView.getMainheader().getTextViewOfTrajInDialog().getResultList().add(textResultList);
					mainView.getMainheader().getTextViewOfTrajInDialog().updateTextView();
				}
				//resultlist is empty
				else{
					mainView.getMainheader().getTextViewOfTrajInDialog().setDataLoaded(true);
				}
			}
		  };		  
		secondoService.getFormattedResult(callback); 
	}
	
	/** Starts an RPC call to the server to get a list of datatypes from the secondoresult and put it into resultLists
	 * 
	 * @param mv The main view object
	 * @param lp The loading popup object
	 * */
	public void getDatatypeResultList(MainView mv, PopupPanel lp) {
		
		this.mainView = mv;
		this.loadingPopup = lp;
						
		AsyncCallback<ArrayList<DataType>> callback = new AsyncCallback<ArrayList<DataType>>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);	
				loadingPopup.hide(); 
				mainView.getGraphicalView().setDataLoaded(true);
				mainView.getMapView().setDataLoaded(true);
			}

			@Override
			public void onSuccess(ArrayList<DataType> result) {
				
				mainView.getMapView().getCurrentResultTypeList().clear();
				mainView.getMapView().clearControllers();
				mainView.getGraphicalView().getCurrentResultList().clear();
				
				if(!result.isEmpty()){

				     mainView.getToolbox().getResultList().add(result);
					
					for(DataType datatype: result){
						mainView.getMapView().getCurrentResultTypeList().add(datatype);
						mainView.getGraphicalView().getCurrentResultList().add(datatype);				
					}			
					 //initialize data in views
				     mainView.getMapView().initializeOverlays();
				     mainView.getGraphicalView().initDataTypes();				     	
				
				   //start timer to wait for data being initialized
		   	       Timer timer = new Timer() {  
		   	    	
		   	    	private int counter = 0; 
		   	    	private int maxCount = 100;
		   	    	
		            @Override
		            public void run() {
		 
		            //If data is initialized start drawing of data
		           	 if (counter >= maxCount) {					        
		                 //resizing + update drawing
		   			     if(mainView.isMapTurnedOn()){
		   			        	mainView.showMapView();
		   			        }		   			    
	                        cancel();
		                    return;
		              }
		           	  if(mainView.getMapView().isDataInitialized() && mainView.getGraphicalView().isDataInitialized()){
		           		counter = maxCount;
		             	}
		           	  counter++;
		             }
		            };
		           timer.scheduleRepeating(500);
		        
				   mainView.getToolbox().updateObjectList();
				
	        	   //Add changehandler to query checkboxes
	        	   mainView.addQueryCheckBoxChangeHandler();
	        	   mainView.addObjectCheckboxChangeHandler();
				 }
				//resultlist is empty
				else{
					mainView.getGraphicalView().setDataLoaded(true);
					mainView.getMapView().setDataLoaded(true);
				}
			}
		  };		  
		secondoService.getResultTypeList(callback); 
	}
	
	/** Starts an RPC call to the server to write a string to a textfile 
	 * 
	 * @param text Text to be saved in the textfile
	 * @param filename Name of the textfile
	 * */
	public void saveTextFile(String text, String filename) {
						
		AsyncCallback<Void> callback = new AsyncCallback<Void>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);		
			}

			@Override
			public void onSuccess(Void result) {
				
				String url = GWT.getModuleBaseURL() + "downloadService?fileName=" + "secondo-text.txt";
				Window.open( url, "_blank", "status=0,toolbar=0,menubar=0,location=0");
				
				System.out.println("########### result is saved in file");		
			}
		  };		  
		secondoService.saveTextFile(text, filename, callback); 		
	}
	
	/** Starts an RPC call to the server to reset the object counter to 1 */
	public void resetObjectCounter() {
						
		AsyncCallback<Void> callback = new AsyncCallback<Void>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);		
			}

			@Override
			public void onSuccess(Void result) {				
				//do nothing		
			}
		  };		  
		secondoService.resetObjectCounter(callback); 		
	}
	
	public void doGPXimport(final String nameOfUploadedFile, final int option, MainView mv, PopupPanel lp){	
		this.mainView = mv;
		this.loadingPopup = lp;

		final String sufix=nameOfUploadedFile.substring(nameOfUploadedFile.lastIndexOf("\\")+1, nameOfUploadedFile.lastIndexOf("."));
		final String relName="Raw"+sufix;


		String command="let "+relName+" = gpximport('"+relName.replace("Raw", "")+".gpx"+"') consume";	
		
		System.out.println("Command "+ command);
		
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);		
			}

			@Override
			public void onSuccess(String result) {		
				if (result.contains("already used")) {
					deleteOldRelation(relName, "", nameOfUploadedFile, option);					
				}else{
				makeMPfromGPX(relName, nameOfUploadedFile, option);
				}	
					
				
						
			}
		  };		  
		  secondoService.sendCommandWithoutResult(command, callback);	
		
	}
	
	public void makeMPfromGPX(final String startRelationName, final String nameOfUploadedFile, final int option) {		
		final String sufix=nameOfUploadedFile.substring(nameOfUploadedFile.lastIndexOf("\\")+1, nameOfUploadedFile.lastIndexOf("."));
		final String resultRelationName="MPfromGPX"+sufix;
		String command = "let "+resultRelationName+" = "+startRelationName+" feed extend[Trip: makepoint(.Lon, .Lat)]sortby[Time asc]approximate[Time, Trip, [const duration value (0 300000)]]";
		System.out.println("Command " + command);
		
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);
			}

			@Override
			public void onSuccess(String result) {
				if (result.contains("already used")) {
					deleteOldRelation(resultRelationName, startRelationName, nameOfUploadedFile, option);					
				}
				else{
					makeRelationFromMP(resultRelationName, nameOfUploadedFile, option);
				}
			}
		};
		secondoService.sendCommandWithoutResult(command, callback);
	}
	
	public void makeRelationFromMP(final String startRelationName, final String nameOfUploadedFile, final int option) {
		final String sufix=nameOfUploadedFile.substring(nameOfUploadedFile.lastIndexOf("\\")+1, nameOfUploadedFile.lastIndexOf("."));
		final String resultRelationName="MPfromGPXrelation"+sufix;
		String command = "let "+resultRelationName+" = "+startRelationName+" feed namedtransformstream[Trip] consume";
		System.out.println("Command " + command);
		
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);
			}

			@Override
			public void onSuccess(String result) {
				if (result.contains("already used")) {
					deleteOldRelation(resultRelationName, startRelationName, nameOfUploadedFile, option);
					
				}
				else{
					createSymTraj(resultRelationName, nameOfUploadedFile, option, mainView, loadingPopup);
				}
			}
		};
		secondoService.sendCommandWithoutResult(command, callback);

	}
	
	public void createSymTraj(final String startRelationName, final String nameOfUploadedFile, final int option, MainView mv,
			PopupPanel lp) {
		final String sufix=nameOfUploadedFile.substring(nameOfUploadedFile.lastIndexOf("\\")+1, nameOfUploadedFile.lastIndexOf("."));
		String resultRelationName = "";
		String command = "";
		//speed mode
		if (option == 0) {
			resultRelationName = "SymTrajWithSpeedMode"+sufix;
			command = "let "
					+ resultRelationName
					+ "= "
					+ startRelationName
					+ " feed addcounter[TrackId, 1] projectextend[TrackId, Trip; Traj: trajectory(.Trip), SymTraj: units(speed(gk(.Trip))) transformstream extend[Speed: the_unit(tolabel(getSpeedString(val(initial(.Elem)))), inst(initial(.Elem)), inst(final(.Elem)), TRUE, FALSE)] makemvalue[Speed]] consume";
			System.out.println("Command " + command);
			}
		
		//direction
		if (option == 1) {
			resultRelationName = "SymTrajWithDirection"+sufix;
			command = "let "
					+ resultRelationName
					+ "= "
					+ startRelationName
					+ " feed addcounter[TrackId, 1] projectextend[TrackId, Trip; Traj: trajectory(.Trip), SymTraj: units(direction("+startRelationName+" feed extract[Trip])) transformstream extend[Direction: the_unit(tolabel(getDirectionString(val(initial(.Elem)))), inst(initial(.Elem)), inst(final(.Elem)), TRUE, FALSE)] makemvalue[Direction]] consume";
			System.out.println("Command " + command);
			}
		
		//distance
		if(option==2){			
			double lat= mv.getMapView().getMyLocation().getX();
			double lon= mv.getMapView().getMyLocation().getY();
			resultRelationName = "SymTrajWithDistance"+sufix;
			command = "let "
					+ resultRelationName
					+ "= "
					+ startRelationName
					+ " feed addcounter[TrackId, 1] projectextend[TrackId, Trip; Traj: trajectory(.Trip), SymTraj: units(distance(gk(.Trip), gk(point ( "+lon+" "+lat+" )))) transformstream extend[Distance: the_unit(tolabel(getDistanceString(val(initial(.Elem)))), inst(initial(.Elem)), inst(final(.Elem)), TRUE, FALSE)] makemvalue[Distance]] consume";
			System.out.println("Command " + command);
		}
		
		//administrative districts
		if (option == 3) {
			resultRelationName = "SymTrajWithAdminDistricts"+sufix;
			command = "let "
					+ resultRelationName
					+ "= "					
					+ " AdminDistrictsGermany feed "+startRelationName+" feed addcounter[TrackId, 1] itSpatialJoin[Gebiet, Trip] projectextend[TrackId, Trip, KName; Traj: trajectory(.Trip), Pieces: .Trip at .Gebiet] filter[not(isempty(deftime(.Pieces)))] projectextendstream[TrackId, Trip, Traj, KName; Time: components(deftime(.Pieces))] extend[Mintime: minimum(.Time), U: the_unit(tolabel(.KName), minimum(.Time), maximum(.Time), TRUE, FALSE)] sortby[TrackId, Mintime] groupby[TrackId;Trip : group feed extract[Trip], Traj : group feed extract[Traj], SymTrip : group feed makemvalue[U]] consume";
			System.out.println("Command " + command);
		}
		
		final 		String resultRelation=resultRelationName;
			AsyncCallback<String> callback = new AsyncCallback<String>() {

				@Override
				public void onFailure(Throwable caught) {
					Window.alert(SERVER_ERROR);
				}

				@Override
				public void onSuccess(String result) {
					if (result.contains("already used")) {
						deleteOldRelation(resultRelation, startRelationName, nameOfUploadedFile, option);

					} else {
						mainView.getOptionsTabPanel()
								.getSelectOptionsForExistingTrajectories()
								.addItem(resultRelation);
						Window.alert("Symbolic trajetory was scuccesfully created. On \"Try trajectory\" animate a newly created relation! ");
					}
				}

			};
			secondoService.sendCommandWithoutResult(command, callback);
		
	}
	
	public void deleteOldRelation(final String resultRelationName, final String startRelationName, final String nameOfUploadedFile, final int option){
		
		final String command="delete " + resultRelationName;
		System.out.println("Command "+ command);
		
		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);		
			}

			@Override
			public void onSuccess(String result) {				
				if(resultRelationName.contains("Raw")){
					doGPXimport(nameOfUploadedFile, option, mainView, loadingPopup);
				}				
				if(resultRelationName.contains("MPfromGPXrelation")){				
					
					makeRelationFromMP(startRelationName, nameOfUploadedFile, option);
				}
				if(resultRelationName.contains("MPfromGPX")&& !resultRelationName.contains("relation")){
					
					makeMPfromGPX(startRelationName, nameOfUploadedFile, option);
				}
				if(resultRelationName.contains("SymTrajWith")){
					createSymTraj(startRelationName, nameOfUploadedFile, option, mainView, loadingPopup);			
				}
				
					
			}
		  };		  
		  secondoService.sendCommandWithoutResult(command, callback);	
	}
	
	public void getCoordinateFromAddress(String command, MainView mv,
			PopupPanel lp) {
		this.mainView = mv;
		this.loadingPopup = lp;
		this.currentCommand = command;

		AsyncCallback<String> callback = new AsyncCallback<String>() {

			@Override
			public void onSuccess(String result) {
				loadingPopup.hide();
				mainView.getMainheader().getLocationDialog()
						.setLabelForResult(result);
				
				if(!result.isEmpty()){	
				result=result.replace("(", "");
				result=result.replace(")", "");
				result=result.replace("point", "");
				result=result.replace("\n", "");
				result=result.trim();
				int end=result.indexOf(" ");
				String lon_str=result.substring(0, end);
				String lat_str=result.substring(result.indexOf(" ")+1, result.length());
				double lon = NumberFormat.getDecimalFormat().parse(lon_str);
				double lat = NumberFormat.getDecimalFormat().parse(lat_str);
				System.out.println("Lon "+ lon+ " Lat "+ lat);
				mainView.getMapView().centerOnMyLocation(lon, lat);
				
				//start timer to wait for data being initialized
		   	       Timer timer = new Timer() {  
		   	    	
		   	    	private int counter = 0; 
		   	    	private int maxCount = 100;
		   	    	
		            @Override
		            public void run() {
		 
		            //If data is initialized start drawing of data
		           	 if (counter >= maxCount) {					        
		                 //resizing + update drawing
		   			     if(mainView.isMapTurnedOn()){
		   			        	mainView.showMapView();
		   			        }		   			    
	                        cancel();
		                    return;
		              }
		           	  if(mainView.getMapView().isDataInitialized() && mainView.getGraphicalView().isDataInitialized()){
		           		counter = maxCount;
		             	}
		           	  counter++;
		             }
		            };
		           timer.scheduleRepeating(500);
				
				}

			}

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);
				loadingPopup.hide();
			}
		};
		secondoService.sendCommand(currentCommand, callback);
	}
	
	public void saveGPXfileToServer(final String filename, final FormPanel form){
		
		if (filename.length() == 0) {
            Window.alert("Please select a file");}
		if(filename.endsWith(".gpx"))
		{
		AsyncCallback<Void> callback = new AsyncCallback<Void>() {
			

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);	
				
			}

			@Override
			public void onSuccess(Void result) {
				form.submit();
				String url = GWT.getModuleBaseURL() + "uploadService?fileName=" + filename;
				Window.open( url, "_blank", "status=0,toolbar=0,menubar=0,location=0");	
				Window.alert("File upload is successfull");
				
				
			}
			
			
		};
		
		secondoService.saveGPXfileToServer(filename, callback); 
		
		
		}
		else {
			Window.alert("You can upload only gpx file");
		}
		
	}
	
	
	/**
	 * The method sends command to the Secondo server, adds it to command
	 * history and updates history.
	 * 
	 * @param command
	 *            to be sent to the server
	 * @param mv
	 * @param lp
	 */
	public void sendCommandAndUpdateHistory(String command, MainView mv,
			PopupPanel lp) {
		sendCommand(command, mv, lp);
		addCommandToHistory(command);
		updateCommandHistory(mv);
		
	}	
	

		
	/**
	 * sets number of tuples and show in pattern result part of options tabs
	 * @param mv
	 */
	public void setNumberOfTuplesInRelationFromResultList(final MainView mv){
		
		AsyncCallback<Integer> callback = new AsyncCallback<Integer>() {

			@Override
			public void onFailure(Throwable caught) {
				Window.alert(SERVER_ERROR);
				
			}

			@Override
			public void onSuccess(Integer result) {

				mv.getOptionsTabPanel().setTextInResultOfPatternMatchingLabel("Result of pattern matching: "+result.toString());
				mv.getOptionsTabPanel().setPatternMatchingIsInitiated(false);
				
			}			
		};
		secondoService.getNumberOfTuplesInRelationFromResultList(callback);
		
	}
}
