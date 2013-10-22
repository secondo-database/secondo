package com.secondo.webgui.server;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;


import com.secondo.webgui.client.datatypes.DataType;
import com.secondo.webgui.client.datatypes.Point;
import com.secondo.webgui.utils.LEUtils;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;

public class SecondoConnection implements Serializable {

	private static final long serialVersionUID = 1790952205953926206L;
	private String secondocommand = "";
	private String secondoresult = "No result";
	private ArrayList<String> databaselist = new ArrayList<String>();
	private ESInterface Secondointerface = new ESInterface();
	private TextFormatter textFormatter = new TextFormatter();
	private GeoTypeConstructor typeConstructor = new GeoTypeConstructor();

	/** connect the commandpanel to SECONDO */
	public boolean connect() {

		boolean ok = Secondointerface.connect();
		return ok;
	}

	/** disconnect from Secondo */
	public void disconnect() {
		Secondointerface.terminate();
	}

	/** Set the values for the connection with SECONDO */
	public void setConnection(String User, String PassWd, String Host, int Port) {
		Secondointerface.setUserName(User);
		Secondointerface.setPassWd(PassWd);
		Secondointerface.setHostname(Host);
		Secondointerface.setPort(Port);
	}

	/**
	 * Method to make a connection to the secondo server and send a query to the
	 * secondo database
	 */
	public boolean doQuery(String command) throws IOException {

		this.secondoresult = "";
		this.secondocommand = command;
		ListExpr resultList = new ListExpr();

		Secondointerface.useBinaryLists(true);

		//not necessary
		if (!Secondointerface.isConnected()) { 
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();

		// get the command from the userinterface and send it to secondo, return the listexpr from secondo
		 Secondointerface.secondo(command, resultList, errorCode,
		 errorPos, errorMessage); 

		 this.secondoresult = resultList.toString();
		 
		 //format the data for the formatted view
		 textFormatter.formatData(resultList);
		 
		 //analyze the geodatatype and put it into the resulttypelist for the graphical view
		 typeConstructor.getDataType(resultList);

		// very important
		/*if (Secondointerface.isConnected()) {
			this.disconnect();
		}*/

		if (errorCode.value != 0) {
			System.err.println("Error in executing query " + command + " \n\n"
					+ errorMessage);
			this.secondoresult = "Error in executing query " + command + " \n\n"
					+ errorMessage;
			return false;
		} else {
			System.err.println("success!");

			return true;
		}

	}
	
	public ArrayList<String> getFormattedList() {
		return textFormatter.getFormattedList();
	}

	public void setFormattedList(ArrayList<String> formattedList) {
		this.textFormatter.setFormattedList(formattedList);
	}
	
	/**Returns the list of types that are returned in the resultList from Secondo*/
	public ArrayList<DataType> getResultTypeList(){
		
		return typeConstructor.getResultTypeList();
	}
	
	
	/**Returns the list of the first tuples of the values that are returned in the resultList from Secondo*/
	public ArrayList<String> getFirstTuplesOfValues(){
		
		return textFormatter.getFirstTuplesOfValues();
	}

	/** Gets all available databases from secondo and adds them to a list */
	public void updateDatabases() {

		ListExpr Databases = new ListExpr();

		Secondointerface.useBinaryLists(true);

		if (!Secondointerface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		String query = "list databases"; 

		// get a list of all databases and write it to string-attribute
		Secondointerface.secondo("list databases", Databases, errorCode,
				errorPos, errorMessage);

		Databases = Databases.second().second();
		System.out.println("Ausgabe von Databases.second().second()"
				+ Databases);// korrekt: (BERLINTEST OPT SYMTRAJ)

		// get databasenames and put them in an arraylist
		int index = 0;
		databaselist.clear();
		while (!Databases.isEmpty()) {
			String Name = Databases.first().symbolValue();
			databaselist.add(index, Name);
			System.out.println("Ein Element der Datenbankliste: " + Name);// korrekt und ohne Klammer

			Databases = Databases.rest();
			index++;

		}

		if (errorCode.value != 0) {
			System.err.println("Error in executing query" + query + " \n\n"
					+ errorMessage);
		}

		// very important
		if (Secondointerface.isConnected()) {
			this.disconnect();
		}

	}

	public boolean openDatabase(String database) {
		
		ListExpr resultList = new ListExpr();
		
		//database name has to be lower case for secondo system, thats why its been changed here
		String lcdatabase = database.toLowerCase();

		Secondointerface.useBinaryLists(true);

		if (!Secondointerface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		
		// open the given database
		Secondointerface.secondo("open database " +lcdatabase, resultList,
						errorCode, errorPos, errorMessage);
		
		System.out.println("Sent command to secondo to open database " +database + ", changed to lower case "+lcdatabase); 
		
	/*	// very important
		if (Secondointerface.isConnected()) {
			this.disconnect();
			}*/

		if (errorCode.value != 0) {
			System.err.println("Error in executing query"
							+ errorMessage);
			return false;
		} else {
			System.err.println("success!");
			return true;
			}

	}
	
	public boolean closeDatabase(String database) {
		
		typeConstructor.getResultTypeList().clear();
		this.secondoresult = "";
		
		
		ListExpr resultList = new ListExpr();
		
		Secondointerface.useBinaryLists(true);

		if (!Secondointerface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		
		Secondointerface.secondo("close database", resultList, errorCode,
				errorPos, errorMessage);
		
		// very important
		if (Secondointerface.isConnected()) {
			this.disconnect();
			}

		if (errorCode.value != 0) {
			System.err.println("Error in executing query"
							+ errorMessage);
			return false;
		} else {
			System.err.println("success!");

			return true;
			}

	}
	
	/*public String getDataType(ListExpr le){
		
		String dataType = "unknown type";
		
		//all general inquiries
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("inquiry")){
			dataType = "inquiry";
			}

		//type is point
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("point")){
			dataType= "point";
		     }
				
        //type is mpoint
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("mpoint")){
			dataType = "mpoint";
		      }
		
		//type is region
		if(le.first().atomType()==ListExpr.SYMBOL_ATOM && le.first().symbolValue().equals("region")){
			dataType = "region";
		      }
				
		//type is relation
		if(le.first().atomType()!=ListExpr.SYMBOL_ATOM && le.first().first().writeListExprToString().contains("rel")){
			dataType = "rel";
			  }
		
		return dataType;
	}*/

	public String getHostName() {
		return Secondointerface.getHostname();
	}

	public int getPort() {
		return Secondointerface.getPort();
	}

	public String getUserName() {
		return Secondointerface.getUserName();
	}

	public String getPassWd() {
		return Secondointerface.getPassWd();
	}

	public String getSecondocommand() {
		return secondocommand;
	}

	public void setSecondocommand(String secondocommand) {
		this.secondocommand = secondocommand;
	}

	public String getSecondoresult() {
		return secondoresult;
	}

	public void setSecondoresult(String secondoresult) {
		this.secondoresult = secondoresult;
	}

	public ArrayList<String> getDatabaselist() {
		return databaselist;
	}

	public void setDatabaselist(ArrayList<String> databaselist) {
		this.databaselist = databaselist;
	}
	
}
