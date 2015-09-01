package mmdb.error.streamprocessing;

import mmdb.error.MMDBException;

public class TypeException extends MMDBException {

	private static final long serialVersionUID = -6957146059375843655L;

	public TypeException(String string) {
		super(string);
	}

	public TypeException(String format, Object... args) {
		super(String.format(format, args));
	}

}
