package com.secondo.webgui.server;

import java.util.ArrayList;

public class SessionData {
	
	private String username ="";
	private String password = "";
	private String secondoIP = "";
	private String secondoPort = "";
	private String openDatabase = "no database open";
	private String loggedIn = "false";
	private ArrayList<String> commandHistory = new ArrayList<String>();
	private ArrayList<String> resultHistory = new ArrayList<String>();
	private ArrayList<ArrayList<String>> formattedResultHistory = new ArrayList<ArrayList<String>>();
	
	public SessionData(){
		
		
	}

	public String getOpenDatabase() {
		return openDatabase;
	}

	public void setOpenDatabase(String openDatabase) {
		this.openDatabase = openDatabase;
	}

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public String getSecondoIP() {
		return secondoIP;
	}

	public void setSecondoIP(String secondoIP) {
		this.secondoIP = secondoIP;
	}

	public String getSecondoPort() {
		return secondoPort;
	}

	public void setSecondoPort(String secondoPort) {
		this.secondoPort = secondoPort;
	}

	public String getLoggedIn() {
		return loggedIn;
	}

	public void setLoggedIn(String loggedIn) {
		this.loggedIn = loggedIn;
	}

	public ArrayList<String> getCommandHistory() {
		return commandHistory;
	}

	public void setCommandHistory(ArrayList<String> commandHistory) {
		this.commandHistory = commandHistory;
	}

	public ArrayList<String> getResultHistory() {
		return resultHistory;
	}

	public void setResultHistory(ArrayList<String> resultHistory) {
		this.resultHistory = resultHistory;
	}

	public ArrayList<ArrayList<String>> getFormattedResultHistory() {
		return formattedResultHistory;
	}

	public void setFormattedResultHistory(
			ArrayList<ArrayList<String>> formattedResultHistory) {
		this.formattedResultHistory = formattedResultHistory;
	}

	
}
