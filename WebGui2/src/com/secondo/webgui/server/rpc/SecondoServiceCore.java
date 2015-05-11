package com.secondo.webgui.server.rpc;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Properties;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.PasswordAuthentication;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;
import com.secondo.webgui.client.rpc.SecondoService;
import com.secondo.webgui.server.controller.SecondoConnector;
import com.secondo.webgui.shared.model.DataType;
import com.secondo.webgui.shared.model.UserData;

/**
 * This class is responsible for the communication between the client and server
 * and support multi-threading.
 * 
 * @author Irina Russkaya
 *
 */
public class SecondoServiceCore extends RemoteServiceServlet implements
		SecondoService, Serializable {

	private static final long serialVersionUID = 1L;

	/** The secondo server connection */
	private SecondoConnector sc = new SecondoConnector();

	// temporary data
	private UserData sd = new UserData();
	private ArrayList<String> secondoConnectionData = new ArrayList<String>();

	public SecondoServiceCore() {
	}

	@Override
	public String sendCommandWithoutResult(String command) {

		System.out.println("SecondoServiceImpl has been called!");

		try {
			sc.doQueryWithoutResult(command);
		} catch (IOException e) {
			System.out.println("Call to Secondo-Server failed.");
			e.printStackTrace();
		}

		return sc.getSecondoresult();
	}

	/**
	 * This method sends the given command to secondo and returns the result
	 * 
	 * @param command
	 *            The command to be send to the server
	 * @return The result from the secondo database
	 * */
	@Override
	public String sendCommand(String command) {

		System.out.println("SecondoServiceImpl has been called!");
		System.out.println("Command " + command);

		try {
			sc.doQuery(command);
		} catch (IOException e) {
			System.out.println("Call to Secondo-Server failed.");
			e.printStackTrace();
		}

		// save the result in the history lists
		sd.getResultHistory().add(sc.getSecondoresult());
		sd.getFormattedResultHistory().add(sc.getFormattedList());

		return sc.getSecondoresult();
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
		this.secondoConnectionData = secondoConnectionData;

		sc.setConnection(secondoConnectionData.get(0),
				secondoConnectionData.get(1), secondoConnectionData.get(2),
				new Integer(secondoConnectionData.get(3)));

		// set data in sessiondata object
		sd.setUsername(secondoConnectionData.get(0));
		sd.setPassword(secondoConnectionData.get(1));
		sd.setSecondoIP(secondoConnectionData.get(2));
		sd.setSecondoPort(secondoConnectionData.get(3));
		sd.setLoggedIn(true);
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

		if (sd.isLoggedIn() == false) {
			return "failed";
		}

		boolean ok = sc.openDatabase(database);

		if (ok == true) {

			sd.setOpenDatabase(database);
			sc.getFormattedList().clear();
			sc.getFormattedListWithMlabel().clear();
			sc.getResultTypeList().clear();
			return database;
		} else
			return "failed";
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

		boolean ok = sc.closeDatabase(database);

		if (ok == true) {

			sd.setOpenDatabase("no database open");

			return database;
		} else
			return "failed";
	}

	/**
	 * This method disconnects from the secondo server and resets the
	 * application to the loginpage
	 * 
	 * @return Ok after logout
	 * */
	@Override
	public String logout() {

		sc.disconnect();

		sd.setLoggedIn(false);

		return "ok";
	}

	/**
	 * This method returns the currently open Database
	 * 
	 * @return The currently open database
	 * */
	@Override
	public String getOpenDatabase() {

		return sd.getOpenDatabase();
	}

	/**
	 * This method returns the user connection data of the current session
	 * 
	 * @return The user connection data
	 * */
	@Override
	public ArrayList<String> getSecondoConnectionData() {
		return secondoConnectionData;
	}

	/**
	 * This method returns the secondo command history of the current session
	 * 
	 * @return The secondo command history of the current session
	 * */
	@Override
	public ArrayList<String> getCommandHistory() {
		return sd.getCommandHistory();
	}

	/**
	 * Add the given command in the history list
	 * 
	 * @param command
	 *            The command to be added to the history list
	 */
	@Override
	public void addCommandToHistory(String command) {
		sd.getCommandHistory().add(command);
	}

	/** This method deletes the command history of the current session */
	@Override
	public void deleteCommandHistory() {

		sd.getCommandHistory().clear();
	}

	/**
	 * This method returns the secondo result history of the current session
	 * 
	 * @return The list with the current result history
	 * */
	@Override
	public ArrayList<String> getResultHistory() {
		return sd.getResultHistory();
	}

	/**
	 * This method returns a formatted text result of the secondo data
	 * 
	 * @result The list with the formatted text result
	 * */
	@Override
	public ArrayList<String> getFormattedResult() {

		return sc.getFormattedList();
	}

	/**
	 * This method returns a list with the datatype objects that are included in
	 * the secondoresult
	 * 
	 * @return The list with datatype objects included in the result
	 * */
	@Override
	public ArrayList<DataType> getResultTypeList() {
		return sc.getResultTypeList();
	}

	/** Resets the object counter of the DataTypeConstructor to 1 */
	@Override
	public void resetObjectCounter() {
		sc.resetCounter();
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

		BufferedWriter writer = null;
		try {
			writer = new BufferedWriter(new FileWriter(filename));
			writer.write(text);
		} catch (IOException e) {
		} finally {
			try {
				if (writer != null)
					writer.close();
			} catch (IOException e) {
			}
		}
	}

	@Override
	public void saveGPXfileToServer(String filename) {
		BufferedReader reader = null;

		try {
			reader = new BufferedReader(new FileReader(filename));
			reader.read();
		} catch (IOException e) {
		} finally {
			try {
				if (reader != null)
					reader.close();
			} catch (IOException e) {
			}
		}

	}

	public Boolean sendMail(String html) {
		final String username = "webappsymtraj@gmail.com";
		final String password = "D5h8ReqDPNx4msckyATu";

		Boolean result = false;

		Properties props = new Properties();
		props.put("mail.smtp.starttls.enable", "true");
		props.put("mail.smtp.auth", "true");
		props.put("mail.transport.protocol", "smtp");
		props.put("mail.smtp.host", "smtp.gmail.com");
		props.put("mail.smtp.port", "587");

		Session session = Session.getInstance(props,
				new javax.mail.Authenticator() {
					protected PasswordAuthentication getPasswordAuthentication() {
						return new PasswordAuthentication(username, password);
					}
				});

		try {

			Message message = new MimeMessage(session);
			message.setFrom(new InternetAddress(username));
			message.setRecipients(Message.RecipientType.TO,
					InternetAddress.parse("Irina.Russkaya@Fernuni-Hagen.de"));
			message.setSubject("Support Request");
			message.setContent(html, "text/html");

			Transport.send(message);

			result = true;

		} catch (MessagingException e) {
			throw new RuntimeException(e);
		}

		return result;

	}

	public void close() {
		sc.disconnect();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * com.secondo.webgui.client.rpc.SecondoService#getFormattedResultForSymTraj
	 * ()
	 */
	@Override
	public ArrayList<String> getFormattedResultForSymTraj() {
		return sc.getFormattedListWithMlabel();
	}
}
