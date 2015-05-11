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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.shared.model.DataType;

/**
 * This class is responsible for the communication between the client and server
 * side of the application. It represents the server side implementation of the
 * RPC-Service Interface. It implements the Interface SecondoService and all
 * methods that have been defined in this interface.
 * 
 * @author Irina Russkaya
 * 
 **/
public class SecondoServiceImpl extends RemoteServiceServlet implements
		SecondoService, Serializable {

	private static final long serialVersionUID = 1L;
	private Map<String, SecondoServiceCore> coreInstances = new HashMap<String, SecondoServiceCore>();

	public SecondoServiceImpl() {
	}

	/**
	 * Returns SecondoServiceCore object. Each session gets its own object.
	 * 
	 * @return The SecondoServiceCore object
	 */
	private SecondoServiceCore getSecondoServiceCore() {
		String sid = this.getThreadLocalRequest().getSession().getId();
		System.out.println("Session id: " + sid);

		SecondoServiceCore coreInstance = coreInstances.get(sid);
		if (coreInstance == null) {
			coreInstance = new SecondoServiceCore();
			coreInstances.put(sid, coreInstance);
		}
		return coreInstance;
	}

	/**
	 * Closes the session with the specified id; SecondoServiceCore object will
	 * be removed
	 */
	public void closeSession() {
		String sid = this.getThreadLocalRequest().getSession().getId();
		SecondoServiceCore coreInstance = coreInstances.get(sid);
		if (coreInstance != null) {
			coreInstance.close();
			coreInstances.remove(sid);
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.secondo.webgui.client.rpc.SecondoService#sendCommandWithoutResult
	 * (java.lang.String)
	 */
	@Override
	public String sendCommandWithoutResult(String command) {

		return getSecondoServiceCore().sendCommandWithoutResult(command);

	}

	/**
	 * This method sends the given command to secondo and returns the result
	 * 
	 * @param command
	 *            The command to be send to the server
	 * @return The result from the secondo database
	 * */
	public String sendCommand(String command) {

		return getSecondoServiceCore().sendCommand(command);
	}

	/**
	 * This method sends the given connection data to secondo. Afterwards the
	 * method to update the databaselist is called.
	 * 
	 * @param secondoConnectionData
	 *            The list with the connection data of the user
	 * */
	@Override
	public void setSecondoConnectionData(ArrayList<String> secondoConnectionData) {
		getSecondoServiceCore().setSecondoConnectionData(secondoConnectionData);
	}

	/**
	 * This method opens the given database. If successful the name of the
	 * database is returned, otherwise "failed"
	 * 
	 * @param database
	 *            The database to be opened
	 * @return The name of the open database or failed
	 * */
	@Override
	public String openDatabase(String database) {

		return getSecondoServiceCore().openDatabase(database);
	}

	/**
	 * This method closes the given database. If successful the name of the
	 * database is returned, otherwise "failed"
	 * 
	 * @param database
	 *            The database to be closed
	 * @return The name of the closed database or failed
	 * */
	@Override
	public String closeDatabase(String database) {

		return getSecondoServiceCore().closeDatabase(database);
	}

	/**
	 * This method disconnects from the secondo server and resets the
	 * application to the loginpage
	 * 
	 * @return Ok after logout
	 * */
	@Override
	public String logout() {

		return getSecondoServiceCore().logout();
	}

	/**
	 * This method returns the currently open Database
	 * 
	 * @return The currently open database
	 * */
	@Override
	public String getOpenDatabase() {

		return getSecondoServiceCore().getOpenDatabase();
	}

	/**
	 * This method returns the user connection data of the current session
	 * 
	 * @return The user connection data
	 * */
	@Override
	public ArrayList<String> getSecondoConnectionData() {
		return getSecondoServiceCore().getSecondoConnectionData();
	}

	/**
	 * This method returns the secondo command history of the current session
	 * 
	 * @return The secondo command history of the current session
	 * */
	@Override
	public ArrayList<String> getCommandHistory() {
		return getSecondoServiceCore().getCommandHistory();
	}

	/**
	 * Add the given command in the history list
	 * 
	 * @param command
	 *            The command to be added to the history list
	 */
	@Override
	public void addCommandToHistory(String command) {
		getSecondoServiceCore().addCommandToHistory(command);
	}

	/** This method deletes the command history of the current session */
	@Override
	public void deleteCommandHistory() {

		getSecondoServiceCore().deleteCommandHistory();
	}

	/**
	 * This method returns the secondo result history of the current session
	 * 
	 * @return The list with the current result history
	 * */
	@Override
	public ArrayList<String> getResultHistory() {
		return getSecondoServiceCore().getResultHistory();
	}

	/**
	 * This method returns a formatted text result of the secondo data
	 * 
	 * @result The list with the formatted text result
	 * */
	@Override
	public ArrayList<String> getFormattedResult() {

		return getSecondoServiceCore().getFormattedResult();
	}

	/**
	 * This method returns a list with the datatype objects that are included in
	 * the secondoresult
	 * 
	 * @return The list with datatype objects included in the result
	 * */
	@Override
	public ArrayList<DataType> getResultTypeList() {
		return getSecondoServiceCore().getResultTypeList();
	}

	/** Resets the object counter of the DataTypeConstructor to 1 */
	@Override
	public void resetObjectCounter() {
		getSecondoServiceCore().resetObjectCounter();
	}

	/**
	 * Writes the given string into a textfile with the given filename
	 * 
	 * @param text
	 *            The text to be written into a file
	 * @param filename
	 *            The name of the file
	 * */
	public void saveTextFile(String text, String filename) {

		getSecondoServiceCore().saveTextFile(text, filename);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.secondo.webgui.client.rpc.SecondoService#saveGPXfileToServer(java
	 * .lang.String)
	 */
	@Override
	public void saveGPXfileToServer(String filename) {
		getSecondoServiceCore().saveGPXfileToServer(filename);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.secondo.webgui.client.rpc.SecondoService#sendMail(java.lang.String)
	 */
	public Boolean sendMail(String html) {
		return getSecondoServiceCore().sendMail(html);
	}

	/* (non-Javadoc)
	 * @see com.secondo.webgui.client.rpc.SecondoService#getFormattedResultForSymTraj()
	 */
	@Override
	public ArrayList<String> getFormattedResultForSymTraj() {		
		return getSecondoServiceCore().getFormattedResultForSymTraj();
	}

}
