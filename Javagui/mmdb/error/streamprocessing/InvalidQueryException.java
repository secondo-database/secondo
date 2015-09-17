package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

/**
 * This exception indicates that a query sent to the core for transformation to
 * nested list was invalid.
 * 
 * @author Björn Clasen
 */
public class InvalidQueryException extends MMDBException {

	private static final long serialVersionUID = -4604130986535531625L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public InvalidQueryException(String message) {
		super(message);
	}

}
