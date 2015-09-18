package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

/**
 * This exception indicates an error while letting the core transform a query to
 * nested list format.
 * 
 * @author Bjoern Clasen
 */
public class TransformQueryException extends MMDBException{

	private static final long serialVersionUID = -237668786538776993L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public TransformQueryException(String message) {
		super(message);
	}

}
