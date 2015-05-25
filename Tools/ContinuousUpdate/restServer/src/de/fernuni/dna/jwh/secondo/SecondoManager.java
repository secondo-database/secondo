package de.fernuni.dna.jwh.secondo;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.LinkedTransferQueue;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import de.fernuni.dna.jwh.representation.NLRepresentation;

/**
 * This class provides the management of the Secondo-Intercation for each registered handler
 * @author Jerome White
 *
 */
public class SecondoManager {

	private static final Logger log4j = LogManager
			.getLogger(SecondoManager.class.getName());

	// Holds the SecondoManager Instance for each registered handler
	private static Map<String, SecondoManager> instances;

	// Each SecondoManager holds a reference to the Queue containing the
	// Tuples which sould be transfered to the Secondo-Server
	private BlockingDeque<NLRepresentation> queue;
	private SecondoTransferThread transferThread;

	/**
	 * Private Constructor to enable the Singleton-Pattern for each registerd Handler
	 * @param handler Name of the Handler this instance should belong to
	 * @throws IOException
	 * @throws SecondoException
	 */
	private SecondoManager(String handler) throws IOException,
			SecondoException {
		super();
		queue = new LinkedBlockingDeque<NLRepresentation>();
		transferThread = new SecondoTransferThread(this, handler);
		transferThread.start();
	}

	/**
	 * Gets/Creates the instance of the SecondoManager for the specified handler
	 * @param handler Handler-Name for which the SecondoManager should be returned
	 * @return Initialized SecondoManager for the handler
	 * @throws IOException
	 * @throws SecondoException
	 */
	public static synchronized SecondoManager getInstance(String handler)
			throws IOException, SecondoException {
		SecondoManager instance;
		if (instances == null) {
			log4j.debug("Creating Instances HashMap");
			instances = new HashMap<String, SecondoManager>();
		}

		instance = instances.get(handler);

		if (instance == null) {
			log4j.debug("Creating instance for " + handler);
			instances.put(handler, new SecondoManager(handler));
			instance = instances.get(handler);
		}

		log4j.debug("Returning Instance for " + handler);
		return instance;
	}

	/** 
	 * Restarts the transfer-thread in case of an error in SECONDO
	 * Should only be called after verifying that the transferThread is really not working!
	 * @param handler
	 * @throws IOException
	 * @throws SecondoException
	 */
	public void restartTransfer(String handler) throws IOException, SecondoException {
		log4j.error("TransferThread for " + handler + " failed, restarting!");
		transferThread = new SecondoTransferThread(this, handler);
		transferThread.start();
	}

	/**
	 * Provides Information about any setup SecondManagers
	 * @return True if any SecondoManagers were created
	 */
	public static boolean isAvailable() {
		if (instances != null && instances.size() > 0) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Adds a tuple to the FIFO-Queue for this SecondoManager
	 * @param o Tuple which sould be added to the queue
	 */
	public void addTuple(NLRepresentation o) {
		queue.addLast(o);
	}

	/**
	 * Return the next tuple from the queue
	 * @return Next tuple from the queue
	 * @throws InterruptedException
	 */
	public NLRepresentation getTuple() throws InterruptedException {
		try {
			NLRepresentation obj = queue.take();
			queue.addFirst(obj);
			return obj;
		} catch (InterruptedException e) {
			log4j.debug(e);
			log4j.debug("Interrupted during queue.take()");
			throw e;
		}
	}
	
	/**
	 * Marks the provided Tuple as processed and does remove it from the queue
	 */
	public void processedOK(){
		queue.removeFirst();
	}

	/**
	 * Shutdown this SecondoManager
	 * @throws InterruptedException
	 */
	private void shutdown() throws InterruptedException {
		transferThread.interrupt();
		transferThread.join();
	}

	/**
	 * Shutdown all SecondoManagers
	 */
	public static void shutdownManagers() {

		for (Iterator<Entry<String, SecondoManager>> iterator = instances
				.entrySet().iterator(); iterator.hasNext();) {
			Entry<String, SecondoManager> type = iterator.next();
			try {
				type.getValue().shutdown();
			} catch (InterruptedException e) {
				log4j.error(e);
			}
		}
	}

	public String getPrintableQueueSize() {
		return "Number of current queue entries: " + queue.size();
	}

}
