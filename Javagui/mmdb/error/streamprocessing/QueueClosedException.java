package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

/**
 * This exception indicates that a queue has been accessed while no element was
 * present. It is meant for parallel procession of queries, which has not yet
 * been implemented!
 * 
 * @author Björn Clasen
 */
public class QueueClosedException extends MMDBException {

	private static final long serialVersionUID = -1304348094655376129L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public QueueClosedException(String message) {
		super(message);
	}

}
