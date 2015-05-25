package de.fernuni.dna.jwh.secondo;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.ServerSocket;
import java.net.Socket;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import de.fernuni.dna.jwh.Configuration;
import de.fernuni.dna.jwh.representation.NLRepresentation;

/**
 * Simple server which provides the tuples received by a specific handler to the
 * Secondo-Receive-Stament
 * 
 * @author Jerome White
 * 
 */
public class SecondoTransferThread extends Thread {

	private static final Logger log4j = LogManager
			.getLogger(SecondoTransferThread.class.getName());

	private SecondoManager manager;
	private SecondoConnection connection;

	ServerSocket serverSocket;
	Socket socket;

	private String handler;
	private DataOutputStream outToClient;
	private BufferedReader inFromClient;

	private boolean working;
	private boolean failed;

	/**
	 * Constructor of the transfer thread
	 * 
	 * @param secondoManager
	 *            Manager which holds the Tuple-Queue
	 * @param newHandler
	 *            Handler for wich this TransferThread is created
	 * @throws IOException
	 * @throws SecondoException
	 */
	public SecondoTransferThread(SecondoManager secondoManager,
			String newHandler) throws IOException, SecondoException {
		manager = secondoManager;
		handler = newHandler;
		failed = false;
		working = false;
		log4j.debug("Creating SecondoConnection - " + handler);
		connection = new SecondoConnection(handler);
	}

	/**
	 * Opens the Connection to the Secondo-Server, starts the Receive-Thread and
	 * serves Tuples to the created Receive-Thread
	 */
	@Override
	public void run() {
		try {
			connection.openDatabase();
			log4j.debug("Start Receive-Query in Secondo");
			connection.startReceive();

			openSendConnection();

			working = true;

			while (working) {
				// Get the next tuple and send it
				try {
					NLRepresentation o = manager.getTuple();
					
					//Check if the connection is still available
					if (!connection.isReceiving()) {
						throw new SecondoException("Receive-Statement failed");
					}
					
					//Transfer the tuple
					transfer(o);
					//Mark the tuple as processed
					manager.processedOK();
					
					log4j.debug("Sent: " + o);
				} catch (InterruptedException e) {
					log4j.info("Transfer-Thread was Interrupted, quitting");
					throw e;
				}
			}
		} catch (IOException | SecondoException | ClassNotFoundException
				| InterruptedException e) {
			log4j.error(e);
			try {
				working = false;
				failed = true;
				inFromClient.close();
				outToClient.close();
				socket.close();
				serverSocket.close();
				if (!(e instanceof InterruptedException)) {
					manager.restartTransfer(handler);
				}
			} catch (IOException | SecondoException e1) {
				log4j.error(e1);
			}
			connection.closeConnection();
		}
	}

	/*
	 * Sends the Tuple to the Secondo-Server
	 */
	private void transfer(NLRepresentation o) throws IOException {
		outToClient.write((o.toString() + "\n").getBytes());
	}

	/*
	 * Create the service which provides the Tuples to the Secondo-Server
	 */
	private void openSendConnection() throws IOException,
			ClassNotFoundException {
		serverSocket = new ServerSocket(
				Configuration.values.handlers.get(handler).nestedListPort);
		log4j.info("Waiting for Connection");
		socket = serverSocket.accept();
		outToClient = new DataOutputStream(socket.getOutputStream());
		inFromClient = new BufferedReader(new InputStreamReader(
				socket.getInputStream()));
		initializeSendConnection();

	}

	/*
	 * Initialize the Secondo-Connection by implementing the expected
	 * init-protocol
	 */
	private void initializeSendConnection() throws IOException,
			ClassNotFoundException {
		log4j.info("Createing temporary Object...");
		NLRepresentation represenation = getRepresenationObject();

		/*
		 * Potocol: Secondo: <GET_TYPE> This: <TYPE> This: Tuple-String This:
		 * <TYPE/> Secondo: <GET_TYPE>
		 */

		String in = inFromClient.readLine();
		if (in.equals("<GET_TYPE>")) {
			outToClient.write("<TYPE>\n".getBytes());
			outToClient
					.write((represenation.getStreamType() + "\n").getBytes());
			outToClient.write("</TYPE>\n".getBytes());
		} else {
			System.err.println("Error initializing");
		}

		in = inFromClient.readLine();
		if (!in.equals("</GET_TYPE>")) {
			System.err.println("Error initializing Server");
		}
	}

	/*
	 * Returns an Instance of the Representation-Object for the configured
	 * handler were acting fro
	 */
	private NLRepresentation getRepresenationObject()
			throws ClassNotFoundException {
		try {
			Class<?> c = Class.forName(Configuration.values.handlers
					.get(handler).representationClass);
			Constructor<?> cons = c.getConstructor();
			return (NLRepresentation) cons.newInstance();
		} catch (ClassNotFoundException | NoSuchMethodException
				| SecurityException | InstantiationException
				| IllegalAccessException | IllegalArgumentException
				| InvocationTargetException e) {
			log4j.fatal(e);
			throw new ClassNotFoundException();
		}

	}

	public boolean isFailed() {
		return failed;
	}
}
