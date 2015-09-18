package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.QueueClosedException;

/**
 * Abstract base class for buffered StreamOperators that support multithreaded
 * evaluation.<br>
 * There are none implemented yet.
 * 
 * @author Bjoern Clasen
 *
 */
public abstract class BufferedStreamOperator implements StreamOperator {

	/**
	 * The buffer this operator stores results in.
	 */
	private Queue buffer = new Queue();

	/**
	 * Adds a result the the resultbuffer.
	 * 
	 * @param object
	 *            the result to add.
	 */
	protected final void addResult(MemoryObject object) {
		this.buffer.put(object);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public final MemoryObject getNext() {
		try {
			return buffer.get();
		} catch (QueueClosedException e) {
			return null;
		}
	}

}
