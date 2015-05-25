package de.fernuni.dna.jwh.secondo;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import sj.lang.ESInterface;
import sj.lang.IntByReference;
import sj.lang.ListExpr;
import de.fernuni.dna.jwh.Configuration;

/**
 * This class handels the basics of opening a Secondo-Session Each handler will
 * have to use its own SecondoConnection
 * 
 * @author Jerome White
 *
 */
public class SecondoConnection {

	private static final Logger log4j = LogManager
			.getLogger(SecondoConnection.class.getName());

	private ESInterface secondoInterface;
	private SecondoReceiver receiver;
	private String handler;

	/**
	 * This Constructor sets up a SecondoConnection for provided Handler-Name
	 * 
	 * @param newHandler
	 *            Name of the handler to which this SecondoConnection sould
	 *            belong
	 * @throws SecondoException
	 */
	public SecondoConnection(String newHandler) throws SecondoException {
		handler = newHandler;
		secondoInterface = new ESInterface();

		// Initalize the connection according to the configuration of the
		// handler
		if (!secondoInterface.initialize(null, null,
				Configuration.values.handlers.get(handler).secondoHost,
				Configuration.values.handlers.get(handler).secondoPort)) {
			throw new SecondoException(
					"Could not initialize Connection to Secondo Server!");
		}

		secondoInterface.useBinaryLists(true);

		// Connect, if not already connected
		log4j.debug("Check if Connection is already available...");
		if (!secondoInterface.isConnected()) {
			if (!secondoInterface.connect()) {
				SecondoException e = new SecondoException(
						"Could not connect to Secondo Server!");
				log4j.error(e);
				throw e;
			}
		}
	}

	/**
	 * Open the Database as provided by the configuration for the handler
	 * 
	 * @throws SecondoException
	 */
	public void openDatabase() throws SecondoException {
		call("open database "
				+ Configuration.values.handlers.get(handler).secondoDatabase,
				false);
	}

	/**
	 * Close the Database
	 * 
	 * @throws SecondoException
	 */
	public void closeDatabase() throws SecondoException {
		call("close database", false);
	}

	/**
	 * Start the Thread which will be used to run the Secondo-Command to receive
	 * the Nested-Lists from this server.
	 * 
	 * @throws SecondoException
	 */
	public synchronized void startReceive() throws SecondoException {
		// Create the Receiver-Thread according to the configuratio
		// of the handler and start it
		if (receiver == null) {
			receiver = new SecondoReceiver(Configuration.values.hostname,
					Configuration.values.handlers.get(handler).nestedListPort,
					Configuration.values.handlers.get(handler).secondoRelation,
					this);
			receiver.start();
		}

		// Block until the receiver-Thread is available or has failed
		while (!receiver.isActive() && !receiver.isFailed()) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
			}
		}
		if (receiver.isFailed()) {
			SecondoException e = new SecondoException("Could not start Receiver");
			log4j.error(e);
			throw e;
		}
	}

	/**
	 * This server has closed the connection providing the receiver-Thread with
	 * tuples The receiver-Thread should complete shortly, after that join the
	 * receiver-Thread.
	 */
	public void closeConnection() {
		try {
			log4j.info("Waiting for Receiver to complete");
			receiver.join();
			log4j.info("Receiver finished");
		} catch (InterruptedException e) {
			log4j.error(e);
		}
	}

	/**
	 * Returns the connection status
	 * 
	 * @return
	 */
	boolean isConnected() {
		return secondoInterface.isConnected();
	}

	/**
	 * Abstraction to the Secondo-Interface to issue commands to the
	 * Secondo-Server with a little error-handling and logging
	 * 
	 * @param cmd
	 *            Secondo-Command to be executed
	 * @param output
	 *            Should the be Output of the Result?
	 * @throws SecondoException
	 */
	void call(String cmd, boolean output) throws SecondoException {
		ListExpr resultList = new ListExpr();
		IntByReference errorCode = new IntByReference();
		IntByReference errorPos = new IntByReference();
		StringBuffer errorMessage = new StringBuffer();

		log4j.debug("Calling Secondo, Output: " + output + ", Command:\n" + cmd);

		secondoInterface.secondo(cmd, resultList, errorCode, errorPos,
				errorMessage);

		if (errorCode.value != 0) {
			outputResult(Level.ERROR, resultList, errorCode, errorPos,
					errorMessage);
			throw new SecondoException("Secondo returned an Error");
		} else if (output) {
			outputResult(Level.INFO, resultList, errorCode, errorPos,
					errorMessage);
		}
	}

	private static void outputResult(Level level, ListExpr resultList,
			IntByReference errorCode, IntByReference errorPos,
			StringBuffer errorMessage) {
		log4j.log(level, "Result" + resultList.toString());
		if (errorCode.value != 0) {
			log4j.log(level, "Error-Code" + errorCode.value);
			log4j.log(level, "Error-Position" + errorPos.value);
			log4j.log(level, "Error-Message" + errorMessage);
		}
	}

	public boolean isReceiving() {
		return !receiver.isFailed() && receiver.isActive();
	}

}
