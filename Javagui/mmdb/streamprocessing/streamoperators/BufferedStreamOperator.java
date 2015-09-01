package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryTuple;
import mmdb.error.streamprocessing.QueueClosedException;
import mmdb.error.streamprocessing.TypeException;

public abstract class BufferedStreamOperator implements StreamOperator {

	private Queue buffer;

	public BufferedStreamOperator() throws TypeException {
		this.buffer = new Queue();
	}

	protected final void addResult(MemoryTuple tuple) {
		this.buffer.put(tuple);
	}

	@Override
	public final MemoryTuple getNext() {
		try {
			return buffer.get();
		} catch (QueueClosedException e) {
			return null;
		}
	}

}
