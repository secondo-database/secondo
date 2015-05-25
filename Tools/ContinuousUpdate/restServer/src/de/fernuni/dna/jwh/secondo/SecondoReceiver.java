package de.fernuni.dna.jwh.secondo;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * Thread implementation which is used to run the Secondo-Command which is used
 * to receive the Tuples from this service
 * 
 * @author Jerome White
 *
 */
class SecondoReceiver extends Thread {
	private static final Logger log4j = LogManager
			.getLogger(SecondoReceiver.class.getName());

	private int sendPort;
	private String sendHost;
	private String sendRelation;
	private SecondoConnection secondoConnection;
	private boolean isActive;
	private boolean isFailed;

	/**
	 * 
	 * @param host
	 * @param port
	 * @param relation
	 * @param secondoConn
	 */
	public SecondoReceiver(String host, int port, String relation,
			SecondoConnection secondoConn) {
		super();
		sendHost = host;
		sendPort = port;
		sendRelation = relation;
		secondoConnection = secondoConn;
		isActive = false;
		isFailed = false;
	}

	/**
	 * Starts the receive statement in the Secondo-Server
	 */
	public void run() {
		// Wait until the connection to Secondo is established
		while (!secondoConnection.isConnected()) {
			log4j.debug("Wating for Secondo Connection...");
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				log4j.error(e);
			}

		}

		isActive = true;

		try {
			String stmt = "query receivenlstream(\"" + sendHost + "\","
					+ sendPort + ") owntransactioninsert[\"" + sendRelation
					+ "\"] providemessages[9000] count";
			log4j.info("Statement:" + stmt);

			secondoConnection.call(stmt, true);

		} catch (SecondoException e) {
			log4j.error(e);
			log4j.error("Error in SecondoReceiver!");
			isActive = false;
			isFailed = true;
		}
		isActive = false;
	}

	/**
	 * @return True if the call of the receive-statement is active
	 */
	public boolean isActive() {
		return isActive;
	}

	/**
	 * @return True if the call of the receive-statement failed for some reason
	 */
	public boolean isFailed() {
		return isFailed;
	}
}