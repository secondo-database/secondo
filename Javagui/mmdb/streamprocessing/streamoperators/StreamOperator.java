package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryObject;
import mmdb.error.memory.MemoryException;
import mmdb.streamprocessing.Node;

/**
 * Interface for all StreamNodes/StreamOperators.<br>
 * Implements the iterator interface open, getNext, close.
 * 
 * @author Bjoern Clasen
 *
 */
public interface StreamOperator extends Node {

	/**
	 * Recursively opens the stream to enable pulling elements.<br>
	 * May initialize some fields.
	 * 
	 * @throws MemoryException
	 *             if during initialization of some fields memory tended to run
	 *             out.
	 */
	public void open() throws MemoryException;

	/**
	 * Retrieves the next element of the stream.<br>
	 * Can only be called if the stream has been opened beforehand.
	 * 
	 * @return the next element in the stream or {@code null} if there are no
	 *         more elements left.
	 * @throws MemoryException
	 *             if during calculation of the next element in the stream
	 *             memory tended to run out.
	 */
	public MemoryObject getNext() throws MemoryException;

	/**
	 * Recursively closes the stream.<br>
	 * Resets all temporarily stored data to enable the stream to be reopened
	 * again.
	 */
	public void close();

}
