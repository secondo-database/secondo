package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

/**
 * This exception indicates any exception during typecheck of an operator tree.
 * 
 * @author Björn Clasen
 */
public class TypeException extends MMDBException {

	private static final long serialVersionUID = -6957146059375843655L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public TypeException(String string) {
		super(string);
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
	public TypeException(String format, Object... args) {
		super(String.format(format, args));
	}

}
