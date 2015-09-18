package mmdb.error.streamprocessing;

/**
 * This exception indicates that a stream was accessed while being closed. This
 * should never happen.
 * 
 * @author Bjoern Clasen
 */
public class StreamStateException extends RuntimeException {

	private static final long serialVersionUID = -2592423401108620181L;

	/**
	 * Creates a new exception an propagates the message to the super class.
	 * 
	 * @param message
	 *            description of the error
	 */
	public StreamStateException(String message) {
		super(message);
	}

}
