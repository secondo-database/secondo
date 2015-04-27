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

package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * This class models a storage for all session based user data of the current session.
 * It has to be serializable to be exchanged between client and server.
 * 
 * @author Irina Russkaya
 * 
 */
public class UserData implements Serializable {
	
	private static final long serialVersionUID = 8278823105467991938L;
	private String username ="";
	private String password = "";
	private String secondoIP = "";
	private String secondoPort = "";
	private String openDatabase = "no database open";
	private boolean isLoggedIn = false;
	private ArrayList<String> commandHistory = new ArrayList<String>();
	private ArrayList<String> resultHistory = new ArrayList<String>();
	private ArrayList<ArrayList<String>> formattedResultHistory = new ArrayList<ArrayList<String>>();
	private int numberOfSuccessfulReusltsInPatternMatching=0;
	
	public UserData(){		
	}

	/**Returns the currently open database
	 * 
	 * @return The currently open database
	 * */
	public String getOpenDatabase() {
		return openDatabase;
	}

	/**Sets the open database to the given value
	 * 
	 * @param openDatabase The currently open database
	 * */
	public void setOpenDatabase(String openDatabase) {
		this.openDatabase = openDatabase;
	}

	/**Returns the username of the current user
	 * 
	 * @return The username of the current user
	 * */
	public String getUsername() {
		return username;
	}

	/**Sets the username to the given value
	 * 
	 * @param username The username of the current user
	 * */
	public void setUsername(String username) {
		this.username = username;
	}

	/**Returns the password of the current user
	 * 
	 * @return The password of the current user
	 * */
	public String getPassword() {
		return password;
	}

	/**Sets the password to the given value
	 * 
	 * @param password The password of the current user
	 * */
	public void setPassword(String password) {
		this.password = password;
	}

	/**Returns the current IP-address of the secondo server
	 * 
	 * @return The current IP-address of the secondo server
	 * */
	public String getSecondoIP() {
		return secondoIP;
	}

	/**Sets the IP-address of the secondo-server to the given value
	 * 
	 * @param secondoIP The current IP-address of the secondo-server
	 * */
	public void setSecondoIP(String secondoIP) {
		this.secondoIP = secondoIP;
	}

	/**Returns the current Port of the secondo server
	 * 
	 * @return The current Port of the secondo server
	 * */
	public String getSecondoPort() {
		return secondoPort;
	}

	/**Sets the Port of the secondo-server to the given value
	 * 
	 * @param secondoPort The current Port of the secondo-server
	 * */
	public void setSecondoPort(String secondoPort) {
		this.secondoPort = secondoPort;
	}

	/**Returns true if the user is logged in
	 * 
	 * @return True if the user is logged in
	 * */
	public boolean isLoggedIn() {
		return isLoggedIn;
	}

	/**Sets the value to true if the user is logged in
	 * 
	 * @param isLoggedIn True if the user logged in, else false
	 * */
	public void setLoggedIn(boolean isLoggedIn) {
		this.isLoggedIn = isLoggedIn;
	}

	/**Returns a list with all commands entered during the last session
	 * 
	 * @return The list with all commands entered during the last session
	 * */
	public ArrayList<String> getCommandHistory() {
		return commandHistory;
	}

	/**Returns a list with all results from secondo from the last session
	 * 
	 * @return The list with all results from secondo from the last session
	 * */
	public ArrayList<String> getResultHistory() {
		return resultHistory;
	}
	
	/**Returns a list with all formatted result lists of the last session
	 * 
	 * @return The list with all formatted result lists of the last session
	 * */
	public ArrayList<ArrayList<String>> getFormattedResultHistory() {
		return formattedResultHistory;
	}

	public int getNumberOfSuccessfulReusltsInPatternMatching() {
		return numberOfSuccessfulReusltsInPatternMatching;
	}

	public void setNumberOfSuccessfulReusltsInPatternMatching(
			int numberOfSuccessfulReusltsInPatternMatching) {
		this.numberOfSuccessfulReusltsInPatternMatching = numberOfSuccessfulReusltsInPatternMatching;
	}
}
