package mmdb.streamprocessing.streamoperators;

import java.util.concurrent.ArrayBlockingQueue;

import mmdb.data.MemoryTuple;
import mmdb.error.streamprocessing.QueueClosedException;

public class Queue {

	private ArrayBlockingQueue<MemoryTuple> queue;

	private MemoryTuple SENTINEL = new MemoryTuple();

	public Queue() {
		this.queue = new ArrayBlockingQueue<MemoryTuple>(100);
	}

	public synchronized MemoryTuple get() throws QueueClosedException {
		try {
			MemoryTuple tuple = this.queue.take();
			if (tuple == SENTINEL) {
				throw new QueueClosedException("");
			} else {
				return tuple;
			}
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	public synchronized void close() {
		this.put(this.SENTINEL);
	}

	public synchronized void put(MemoryTuple tuple) {
		try {
			queue.put(tuple);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

}
