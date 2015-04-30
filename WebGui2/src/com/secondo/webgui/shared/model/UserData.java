package com.secondo.webgui.shared.model;

import java.io.Serializable;
import java.util.ArrayList;

/**
 * This class models a storage for all session based user data of the current
 * session. It has to be serializable to be exchanged between client and server.
 * 
 * @author Irina Russkaya
 * 
 */
public class UserData implements Serializable {

	private static final long serialVersionUID = 8278823105467991938L;
	private String username = "";
	private String password = "";
	private String secondoIP = "";
	private String secondoPort = "";
	private String openDatabase = "no database open";
	private boolean isLoggedIn = false;
	private ArrayList<String> commandHistory = new ArrayList<String>();
	private ArrayList<String> resultHistory = new ArrayList<String>();
	private ArrayList<ArrayList<String>> formattedResultHistory = new ArrayList<ArrayList<String>>();

	public UserData() {
	}

	/**
	 * Returns the currently open database
	 * 
	 * @return The currently open database
	 * */
	public String getOpenDatabase() {
		return openDatabase;
	}

	/**
	 * Sets the open database to the given value
	 * 
	 * @param openDatabase
	 *            The currently open database
	 * */
	public void setOpenDatabase(String openDatabase) {
		this.openDatabase = openDatabase;
	}

	/**
	 * Returns the username of the current user
	 * 
	 * @return The username of the current user
	 * */
	public String getUsername() {
		return username;
	}

	/**
	 * Sets the username to the given value
	 * 
	 * @param username
	 *            The username of the current user
	 * */
	public void setUsername(String username) {
		this.username = username;
	}

	/**
	 * Returns the password of the current user
	 * 
	 * @return The password of the current user
	 * */
	public String getPassword() {
		return password;
	}

	/**
	 * Sets the password to the given value
	 * 
	 * @param password
	 *            The password of the current user
	 * */
	public void setPassword(String password) {
		this.password = password;
	}

	/**
	 * Returns the current IP-address of the secondo server
	 * 
	 * @return The current IP-address of the secondo server
	 * */
	public String getSecondoIP() {
		return secondoIP;
	}

	/**
	 * Sets the IP-address of the secondo-server to the given value
	 * 
	 * @param secondoIP
	 *            The current IP-address of the secondo-server
	 * */
	public void setSecondoIP(String secondoIP) {
		this.secondoIP = secondoIP;
	}

	/**
	 * Returns the current Port of the secondo server
	 * 
	 * @return The current Port of the secondo server
	 * */
	public String getSecondoPort() {
		return secondoPort;
	}

	/**
	 * Sets the Port of the secondo-server to the given value
	 * 
	 * @param secondoPort
	 *            The current Port of the secondo-server
	 * */
	public void setSecondoPort(String secondoPort) {
		this.secondoPort = secondoPort;
	}

	/**
	 * Returns true if the user is logged in
	 * 
	 * @return True if the user is logged in
	 * */
	public boolean isLoggedIn() {
		return isLoggedIn;
	}

	/**
	 * Sets the value to true if the user is logged in
	 * 
	 * @param isLoggedIn
	 *            True if the user logged in, else false
	 * */
	public void setLoggedIn(boolean isLoggedIn) {
		this.isLoggedIn = isLoggedIn;
	}

	/**
	 * Returns a list with all commands entered during the last session
	 * 
	 * @return The list with all commands entered during the last session
	 * */
	public ArrayList<String> getCommandHistory() {
		return commandHistory;
	}

	/**
	 * Returns a list with all results from secondo from the last session
	 * 
	 * @return The list with all results from secondo from the last session
	 * */
	public ArrayList<String> getResultHistory() {
		return resultHistory;
	}

	/**
	 * Returns a list with all formatted result lists of the last session
	 * 
	 * @return The list with all formatted result lists of the last session
	 * */
	public ArrayList<ArrayList<String>> getFormattedResultHistory() {
		return formattedResultHistory;
	}
}
