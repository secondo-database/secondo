package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

public class ParsingException extends MMDBException {

	private static final long serialVersionUID = 7638420368851305121L;

	public ParsingException(String message) {
		super(message);
	}

	public ParsingException(String format, Object... args) {
		super(String.format(format, args));
	}

}
