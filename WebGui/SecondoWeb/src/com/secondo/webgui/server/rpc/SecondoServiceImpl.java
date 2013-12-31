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

package com.secondo.webgui.server.rpc;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import com.google.gwt.user.server.rpc.RemoteServiceServlet;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.server.controller.OptimizerConnector;
import com.secondo.webgui.server.controller.SecondoConnector;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.UserData;

/**
*  This class is responsible for the communication between the client and server side of the application. 
*  It represents the server side implementation of the RPC-Service Interface. It implements the Interface 
*  SecondoService and all methods that have been defined in this interface.
*  
*  @author Kristina Steiger
*  
**/
public class SecondoServiceImpl extends RemoteServiceServlet implements SecondoService, Serializable {
	
	private static final long serialVersionUID = 1L;
	
	/**The secondo server connection*/
	private SecondoConnector sc = new SecondoConnector();
	
	/**The optimizer server connection*/
	private OptimizerConnector oc = new OptimizerConnector();
	
	//temporary data
	private UserData sd = new UserData();
	private ArrayList<String> secondoConnectionData = new ArrayList<String>();
	
	public SecondoServiceImpl(){	
	}

	/**This method sends the given command to secondo and returns the result
	 * 
	 * @param command The command to be send to the server
	 * @return The result from the secondo database
	 * */
	@Override
	public String sendCommand(String command) {
		
		System.out.println("SecondoServiceImpl has been called!");

    	try {
			sc.doQuery(command);
		} catch (IOException e) {
			System.out.println("Call to Secondo-Server failed.");
			e.printStackTrace();
		}
  	
    	    //save command and result in the history lists
	    	sd.getCommandHistory().add(command);
	    	sd.getResultHistory().add(sc.getSecondoresult());	    	
	    	sd.getFormattedResultHistory().add(sc.getFormattedList());
		
	    return sc.getSecondoresult();
	  }

	/**This method sends the given connection data to secondo. Afterwards the method to update the databaselist is called.
	 * 
	 * @param secondoConnectionData The list with the connection data of the user
	 * */
	@Override
	public void setSecondoConnectionData(ArrayList<String> secondoConnectionData) {
		this.secondoConnectionData = secondoConnectionData;

		sc.setConnection(secondoConnectionData.get(0), secondoConnectionData.get(1), secondoConnectionData.get(2), new Integer(secondoConnectionData.get(3)));

		//set data in sessiondata object
		sd.setUsername(secondoConnectionData.get(0));
		sd.setPassword(secondoConnectionData.get(1));
		sd.setSecondoIP(secondoConnectionData.get(2));
		sd.setSecondoPort(secondoConnectionData.get(3));
		sd.setLoggedIn(true);
	}
	
	/**Sets the connection data for the optimizer
	 * 
	 * @param Host The host for the connection to the optimizer
	 * @param Port The port for the connection to the optimizer
	 * @return An errormessage if the connection failed
	 * */
	@Override
	public String setOptimizerConnection(String Host, int Port) {
		
		//disconnect optimizer if its connected
		if(oc.isOptimizerConnected()){
			oc.disconnectOptimizer();
		}
		
		//set new optimizer connection data
		oc.setOptimizerConnection(Host, Port);
		
		//test connection
		if(oc.connectOptimizer()){
			oc.disconnectOptimizer();
			return "no error";		
		}
		else{
			return  oc.getLastError();
		}
	}
	

	/**Returns the current connection data of the optimizer
	 * 
	 * @return The current connection data of the optimizer
	 * */
	@Override
	public ArrayList<String> getOptimizerConnectionData() {

		return oc.getOptimizerConnection();
	}
	

	/**Sends the query to the optimizer and returns the optimized query, an error message, status of connection, the last errorcode 
	 * and the time spent for optimization
	 * 
	 * @param command The command to be send to the optimizer
	 * @param database The currently open database
	 * @param executeFlag The execute Flag
	 * @return A list with the optimized query, an error message, status of connection, the last errorcode 
	 * and the time spent for optimization */
	@Override
	public ArrayList<String> getOptimizedQuery(String command, String database, boolean executeFlag) {
		
		ArrayList<String> resultList = new ArrayList<String>();
		String result = "";
		
		try {
			result = oc.getOptimizedQuery(command, database, executeFlag);
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		//add result, errormessage and connectioninfo to resultlist
		String errormessage = oc.getErrorMessage();
		boolean isConnected = oc.isOptimizerConnected();
		String lastError = oc.getLastError();
		String timeSpentForOptimization = oc.getTimeSpentForOptimization();
		
		resultList.add(result);
		resultList.add(errormessage);
		if(isConnected){
			resultList.add("true");
		}
		else{
			resultList.add("false");
		}
		resultList.add(lastError);
		resultList.add(timeSpentForOptimization);
		
		return resultList;
	}	
	
	/**Returns a list with all available databases 
	 * 
	 * @return A list with all available databases
	 * */
	@Override
	public ArrayList<String> updateDatabaseList(){
		
		return sc.updateDatabases();
	}

	/**This method opens the given database. If successful the name of the database is returned, otherwise "failed"
	 * 
	 * @param database The database to be opened
	 * @return The name of the open database or failed
	 * */
	@Override
	public String openDatabase(String database){		
		
		if(sd.isLoggedIn() == false){
			return "failed";
		}
		
		boolean ok = sc.openDatabase(database);

		if (ok == true){
			
			sd.setOpenDatabase(database);
			sc.getFormattedList().clear();
			sc.getResultTypeList().clear();
			return database;
		}		
		else 
			return "failed";
	}
	
	/**This method closes the given database. If successful the name of the database is returned, otherwise "failed"
	 * 
	 * @param database The database to be closed
	 * @return The name of the closed database or failed
	 * */
	@Override
    public String closeDatabase(String database){
		
		boolean ok = sc.closeDatabase(database);
		
		if (ok == true){
			
			sd.setOpenDatabase("no database open");

			return database;
		}		
		else 
			return "failed";
	}
    
	/**This method disconnects from the secondo server and resets the application to the loginpage
	 * 
	 * @return Ok after logout
	 * */
	@Override
    public String logout(){
    	
    	sc.disconnect();
		
    	sd.setLoggedIn(false);

    	return "ok";
	}
    
    /**This method returns the currently open Database
     * 
     * @return The currently open database
     * */
    @Override
    public String getOpenDatabase(){
    	
    	return sd.getOpenDatabase();    	
    }   
	
    /**This method returns the user connection data of the current session
     * 
     * @return The user connection data
     * */
    @Override
	public ArrayList<String> getSecondoConnectionData() {
		return secondoConnectionData;
	}
	
    /**This method returns the secondo command history of the current session
     * 
     * @return The secondo command history of the current session
     * */
    @Override
	public ArrayList<String> getCommandHistory() {
		return sd.getCommandHistory();
	}
	
	/**This method deletes the command history of the current session */
    @Override
	public void deleteCommandHistory(){
		
		sd.getCommandHistory().clear();
	}
	
    /**This method returns the secondo result history of the current session
     * 
     * @return The list with the current result history
     * */
    @Override
	public ArrayList<String> getResultHistory() {
		return sd.getResultHistory();
	}
	
	/**This method returns a formatted text result of the secondo data 
	 * 
	 * @result The list with the formatted text result
	 * */
    @Override
	public ArrayList<String> getFormattedResult(){
		
    	return sc.getFormattedList();
	}

	/**This method returns a list with the datatype objects that are included in the secondoresult 
	 * 
	 * @return The list with datatype objects included in the result
	 * */
    @Override
	public ArrayList<DataType> getResultTypeList() {
		
		return sc.getResultTypeList();
	}

	/**Resets the object counter of the DataTypeConstructor to 1*/
	@Override
	public void resetObjectCounter() {
		sc.resetCounter();		
	}	
	
	/**Writes the given string into a textfile with the given filename
	 * 
	 * @param text The text to be written into a file
	 * @param filename The name of the file
	 * */
	public void saveTextFile(String text, String filename){

		BufferedWriter writer = null;
		try
		{
		    writer = new BufferedWriter( new FileWriter(filename));
		    writer.write(text);
		}
		catch ( IOException e)
		{
		}
		finally
		{
		    try
		    {
		        if ( writer != null)
		        writer.close( );
		    }
		    catch ( IOException e)
		    {
		    }
		}
	}
}
