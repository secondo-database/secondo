package com.secondo.webgui.server;

import java.io.IOException;
import java.util.ArrayList;
import com.google.gwt.user.server.rpc.RemoteServiceServlet;
import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.client.rpc.SecondoService;

public class SecondoServiceImpl extends RemoteServiceServlet implements SecondoService {
	
	private static final long serialVersionUID = 1L;
	
	private SecondoConnection sc;
	private SessionData sd;
	private ArrayList<String> secondoConnectionData = new ArrayList<String>();
	private ArrayList<String> databaselist = new ArrayList<String>();
	
	public SecondoServiceImpl(){
		
		this.sc = new SecondoConnection();
		this.sd = new SessionData();
		
	}

	/**This method sends the given command to secondo and returns the result*/
	public String sendCommand(String command) {
		
		System.out.println("SecondoServiceImpl has been called!");

    	try {
			sc.doQuery(command);
		} catch (IOException e) {
			System.out.println("Call to Secondo-Server failed.");
			e.printStackTrace();
		}
  	
    	//save command and result in the history lists if it doesn´t exist already
	    	sd.getCommandHistory().add(command);
	    	sd.getResultHistory().add(sc.getSecondoresult());
	    	
	    	sd.getFormattedResultHistory().add(sc.getFormattedList());
	    	    	
	    	System.out.println("## command and result is saved in history list");
		
	    return sc.getSecondoresult();
	  }



	/**This method sends the given connection data to secondo. Then the available databases are updated in the list and the databaselist is returned to the user interface*/
	public ArrayList<String> setSecondoConnectionData(ArrayList<String> secondoConnectionData) {
		this.secondoConnectionData = secondoConnectionData;

		sc.setConnection(secondoConnectionData.get(0), secondoConnectionData.get(1), secondoConnectionData.get(2), new Integer(secondoConnectionData.get(3)));

		//set data in sessiondata object
		sd.setUsername(secondoConnectionData.get(0));
		sd.setPassword(secondoConnectionData.get(1));
		sd.setSecondoIP(secondoConnectionData.get(2));
		sd.setSecondoPort(secondoConnectionData.get(3));
		sd.setLoggedIn("true");
		
		//create list of available databases
    	sc.updateDatabases();

    	databaselist = sc.getDatabaselist();
		
		return databaselist;
	}

	/**This method opens the given database. If successful the name of the database is returned, otherwise "failed"*/
	public String openDatabase(String database){
		
		
		if(sd.getLoggedIn().equals("false")){
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
	
	/**This method closes the given database. If successful the name of the database is returned, otherwise "failed"*/
    public String closeDatabase(String database){
		
		boolean ok = sc.closeDatabase(database);//sd.getOpenDatabase()
		
		if (ok == true){
			
			sd.setOpenDatabase("no database open");

			return database;
		}
		
		else 
			return "failed";
	}
    
	/**This method logs out of the application*/
    public String logout(){
		
    	sd.setLoggedIn("false");

    	return "ok";
	}
    
    /**This method returns the currently open Database*/
    public String getOpenDatabase(){
    	
    	return sd.getOpenDatabase();
    	
    }
    
    
    /**This method checks if the user is still logged in*/
	public String isLoggedIn(){
		
		return sd.getLoggedIn();
		
	}
	
    /**This method returns the user connection data of the current session*/
	public ArrayList<String> getSecondoConnectionData() {
		return secondoConnectionData;
	}
	
    /**This method returns the secondo command history of the current session*/
	public ArrayList<String> getCommandHistory() {
		return sd.getCommandHistory();
	}
	
	/**This method deletes the command history of the current session*/
	public String deleteCommandHistory(){
		
		sd.getCommandHistory().clear();
		
		return "";
	}
	
    /**This method returns the secondo result history of the current session*/
	public ArrayList<String> getResultHistory() {
		return sd.getResultHistory();
	}
	
	/**This method returns the secondo result history of the current session*/
	public String updateResult(String command) {

		try {
			sc.doQuery(command);
		} catch (IOException e) {
			System.out.println("Call to Secondo-Server failed.");
			e.printStackTrace();

		}
		
	    return sc.getSecondoresult();
	}
	
	/**This method returns a formatted result of the secondo data */
	public ArrayList<String> getFormattedResult(){
		
    	return sc.getFormattedList();

	}

	/**This method returns an arraylist with the datatypes that are included in the secondoresult */
	public ArrayList<DataType> getResultTypeList() {
		
		return sc.getResultTypeList();
	}

	@Override
	public ArrayList<String> getFirstTuplesOfValues() {

		return sc.getFirstTuplesOfValues();
	}
	
}
