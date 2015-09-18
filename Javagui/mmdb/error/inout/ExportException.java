package mmdb.error.inout;

import mmdb.error.MMDBException;

/**
 * This exception indicates an error during export of a any MemoryObject.
 * 
 * @author Bjoern Clasen
 */
public class ExportException extends MMDBException {

	private static final long serialVersionUID = -2495107194021992145L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public ExportException(String message) {
		super(message);
	}

}
