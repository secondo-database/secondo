package mmdb.streamprocessing.streamoperators;

import java.util.concurrent.ArrayBlockingQueue;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.error.streamprocessing.QueueClosedException;

/**
 * A blocking queue for storing operator results as MemoryObjects.<br>
 * Not yet in use.
 * 
 * @author Björn Clasen
 */
public class Queue {

	/**
	 * The actual blocking queue to store results in.
	 */
	private ArrayBlockingQueue<MemoryObject> queue = new ArrayBlockingQueue<MemoryObject>(
			100);

	/**
	 * A sentinel representing the end of the queue.
	 */
	private MemoryObject SENTINEL = new MemoryTuple();

	/**
	 * Retrieves the next result element from this queue.
	 * 
	 * @return next result as MemoryObject.
	 * @throws QueueClosedException
	 *             if the queue is empty.
	 */
	public synchronized MemoryObject get() throws QueueClosedException {
		try {
			MemoryObject tuple = this.queue.take();
			if (tuple == SENTINEL) {
				throw new QueueClosedException("Queue is empty!");
			} else {
				return tuple;
			}
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	/**
	 * Closes the queue.
	 */
	public synchronized void close() {
		this.put(this.SENTINEL);
	}

	/**
	 * Puts the result object into the queue.
	 * 
	 * @param object
	 *            the result to put into the queue.
	 */
	public synchronized void put(MemoryObject object) {
		try {
			queue.put(object);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

}
