package mmdb.error.inout;

import mmdb.error.MMDBException;

/**
 * This exception indicates an error during import of a any MemoryObject.
 * 
 * @author Bjoern Clasen
 */
public class ImportException extends MMDBException {

	private static final long serialVersionUID = 2935528668452012102L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public ImportException(String message) {
		super(message);
	}

}
