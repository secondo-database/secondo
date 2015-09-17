package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

/**
 * This exception indicates an error during parse phase of query processing
 * (while building up the operator tree).
 * 
 * @author Björn Clasen
 */
public class ParsingException extends MMDBException {

	private static final long serialVersionUID = 7638420368851305121L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public ParsingException(String message) {
		super(message);
	}

	/**
	 * Creates a new exception an propagates the formated message to the super
	 * class. Saves the step of calling String.format(...).
	 * 
	 * @param format
	 *            raw description of the error
	 * @param args
	 *            the args to put into the description
	 */
	public ParsingException(String format, Object... args) {
		super(String.format(format, args));
	}

}
