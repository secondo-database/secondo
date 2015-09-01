package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

public class QueueClosedException extends MMDBException {

	private static final long serialVersionUID = -1304348094655376129L;
	
	public QueueClosedException(String message) {
		super(message);
	}

}
