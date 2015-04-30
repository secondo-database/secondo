package com.secondo.webgui.server.controller;

import java.io.IOException;
import java.util.ArrayList;

import com.secondo.webgui.shared.model.DataType;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import tools.Environment;

/**
 * This class offers methods to connect to the secondo server, send commands to
 * the secondo database and retrieve the result in NestList-Format. For this
 * functionality it uses the external file SecondoInterface.jar.
 * 
 * @author Irina Russkaya
 */
public class SecondoConnector {

	// interface for connection to secondo database
	private ESInterface secondoInterface = new ESInterface();

	// transformer for data from secondo database
	private TextFormatter textFormatter = new TextFormatter();
	private DataTypeConstructor typeConstructor = new DataTypeConstructor();

	// temporary data
	private String secondoresult = "";
	private ArrayList<String> databaselist = new ArrayList<String>();

	private int numberOfTuplesInRelationFromResult = 0;

	public SecondoConnector() {
	}

	/**
	 * Connects to SECONDO
	 * 
	 * @return True if connection was successful
	 * */
	public boolean connect() {

		boolean ok = secondoInterface.connect();
		return ok;
	}

	/** Disconnects from Secondo */
	public void disconnect() {
		secondoInterface.terminate();
	}

	/**
	 * Checks if the application is still connected to Secondo
	 * 
	 * @return True if the application is connected to Secondo
	 * */
	public boolean isConnected() {
		return secondoInterface.isConnected();
	}

	/**
	 * Sets the values for the connection with SECONDO
	 * 
	 * @param User
	 *            The username of the current user
	 * @param PassWd
	 *            The password of the current user
	 * @param Host
	 *            The Host of the Secondo-Server
	 * @param Port
	 *            The Port of the Secondo-Server
	 * */
	public void setConnection(String User, String PassWd, String Host, int Port) {

		Environment.ENCODING = "UTF8";

		secondoInterface.setUserName(User);
		secondoInterface.setPassWd(PassWd);
		secondoInterface.setHostname(Host);
		secondoInterface.setPort(Port);
	}

	/**
	 * Method to make a connection to the secondo server and send a query to the
	 * secondo database
	 * 
	 * @param command
	 *            The command to be send to the server.
	 * @return The result from secondo in listexpression format.
	 * @exception IOException
	 *                On input error.
	 */
	public ListExpr doQuery(String command) throws IOException {

		this.secondoresult = "";
		ListExpr resultList = new ListExpr();

		secondoInterface.useBinaryLists(true);

		if (!secondoInterface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();

		// get the command from the userinterface and send it to secondo, return
		// the listexpr from secondo
		secondoInterface.secondo(command, resultList, errorCode, errorPos,
				errorMessage);

		if (errorCode.value != 0) {
			System.err.println("Error in executing " + command + " \n"
					+ errorMessage);
			this.secondoresult = "Error in executing " + command + " \n"
					+ errorMessage;
			return resultList;
		} else {
			System.err.println("success!");
			this.secondoresult = resultList.toString();

			// format the data for the formatted view
			textFormatter.formatData(resultList);

			// analyze the geodatatype and put it into the resulttypelist for
			// the graphical view
			typeConstructor.getDataType(resultList);
			numberOfTuplesInRelationFromResult = typeConstructor
					.getNumberOfTuplesInRelation();
			return resultList;
		}
		/*
		 * if (Secondointerface.isConnected()) { this.disconnect(); }
		 */
	}

	/**
	 * Method to make a connection to the secondo server and send a query to the
	 * secondo database. Successful query has no result in resultList (like
	 * create or delete queries)
	 * 
	 * @param command
	 *            The command to be send to the server.
	 * @exception IOException
	 *                On input error.
	 */
	public void doQueryWithoutResult(String command) throws IOException {

		this.secondoresult = "";
		ListExpr resultList = new ListExpr();

		secondoInterface.useBinaryLists(true);

		if (!secondoInterface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();

		// get the command from the userinterface and send it to secondo, return
		// the listexpr from secondo
		secondoInterface.secondo(command, resultList, errorCode, errorPos,
				errorMessage);

		if (errorCode.value != 0) {
			System.err.println("Error in executing " + command + " \n"
					+ errorMessage);
			this.secondoresult = "Error in executing " + command + " \n"
					+ errorMessage;
		} else {
			System.err.println("success!");
			this.secondoresult = resultList.toString();

		}

	}

	/**
	 * Returns all available databases from secondo and adds them to a list
	 * 
	 * @return The list of all available databases
	 * */
	public ArrayList<String> updateDatabases() {

		ArrayList<String> databaselist = new ArrayList<String>();

		ListExpr Databases = new ListExpr();

		secondoInterface.useBinaryLists(true);

		// close connection to old secondo server
		if (secondoInterface.isConnected()) {
			this.disconnect();
		}

		// Connect to new secondo server
		this.connect();

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();
		String query = "list databases";

		// get a list of all databases and write it to string-attribute
		secondoInterface.secondo("list databases", Databases, errorCode,
				errorPos, errorMessage);

		Databases = Databases.second().second();
		System.out.println("Ausgabe von Databases.second().second()"
				+ Databases);

		// get databasenames and put them in an arraylist
		int index = 0;

		while (!Databases.isEmpty()) {
			String Name = Databases.first().symbolValue();
			databaselist.add(index, Name);
			System.out.println("Ein Element der Datenbankliste: " + Name);

			Databases = Databases.rest();
			index++;
		}

		if (errorCode.value != 0) {
			System.err.println("Error in executing query" + query + " \n\n"
					+ errorMessage);
		}

		// very important
		if (secondoInterface.isConnected()) {
			this.disconnect();
		}
		return databaselist;
	}

	/**
	 * Sends a command to secondo to open the given database
	 * 
	 * @param database
	 *            The database to be opened
	 * @return True if the database has been opened
	 * */
	public boolean openDatabase(String database) {

		ListExpr resultList = new ListExpr();

		// database name has to be lower case for secondo system, thats why its
		// been changed here
		String lcdatabase = database.toLowerCase();

		secondoInterface.useBinaryLists(true);

		if (!secondoInterface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();

		// open the given database
		secondoInterface.secondo("open database " + lcdatabase, resultList,
				errorCode, errorPos, errorMessage);

		System.out.println("Sent command to secondo to open database "
				+ database + ", changed to lower case " + lcdatabase);

		/*
		 * if (Secondointerface.isConnected()) { this.disconnect(); }
		 */

		if (errorCode.value != 0 && errorCode.value != 7) {

			System.err.println("Error in executing query" + errorMessage);
			return false;
		} else {
			System.err.println("success!");
			return true;
		}
	}

	/**
	 * Sends a command to secondo to close the given database
	 * 
	 * @param database
	 *            The database to be closed
	 * @return True if the database has been closed
	 * */
	public boolean closeDatabase(String database) {

		typeConstructor.getResultTypeList().clear();
		this.secondoresult = "";

		ListExpr resultList = new ListExpr();

		secondoInterface.useBinaryLists(true);

		if (!secondoInterface.isConnected()) {
			this.connect();
		}

		IntByReference errorCode = new IntByReference(0);
		IntByReference errorPos = new IntByReference(0);
		StringBuffer errorMessage = new StringBuffer();

		secondoInterface.secondo("close database", resultList, errorCode,
				errorPos, errorMessage);

		// very important
		if (secondoInterface.isConnected()) {
			this.disconnect();
		}

		if (errorCode.value != 0) {
			System.err.println("Error in executing query" + errorMessage);
			return false;
		} else {
			System.err.println("success!");

			return true;
		}
	}

	/** Resets counter for objects to 1 */
	public void resetCounter() {
		typeConstructor.resetCounter();
	}

	/**
	 * Returns the list of formatted text that is returned from the
	 * textFormatter
	 * 
	 * @return The list of formatted text
	 * */
	public ArrayList<String> getFormattedList() {
		return textFormatter.getFormattedList();
	}

	/**
	 * Returns the list of datatype objects that is returned from the
	 * datatypeConstructor
	 * 
	 * @return The list of datatype objects
	 * */
	public ArrayList<DataType> getResultTypeList() {

		return typeConstructor.getResultTypeList();
	}

	/**
	 * Returns the hostname of the secondo server
	 * 
	 * @return The hostname of the secondo server
	 * */
	public String getHostName() {
		return secondoInterface.getHostname();
	}

	/**
	 * Returns the port of the secondo server
	 * 
	 * @return The port of the secondo server
	 * */
	public int getPort() {
		return secondoInterface.getPort();
	}

	/**
	 * Returns the username of the current user
	 * 
	 * @return The username of the current user
	 * */
	public String getUserName() {
		return secondoInterface.getUserName();
	}

	/**
	 * Returns the password of the current user
	 * 
	 * @return The password of the current user
	 * */
	public String getPassWd() {
		return secondoInterface.getPassWd();
	}

	/**
	 * Returns the resultlist of the secondo database
	 * 
	 * @return The resultlist of the secondo database
	 * */
	public String getSecondoresult() {
		return secondoresult;
	}

	/**
	 * Returns the list of currently available databases
	 * 
	 * @return The list of currently available databases
	 * */
	public ArrayList<String> getDatabaselist() {
		return databaselist;
	}

	/**
	 * @return the numberOfTuplesInRelationFromResult
	 */
	public int getNumberOfTuplesInRelationFromResult() {
		return numberOfTuplesInRelationFromResult;
	}
}
