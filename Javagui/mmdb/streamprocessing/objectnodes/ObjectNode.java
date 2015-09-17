package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.error.memory.MemoryException;
import mmdb.streamprocessing.Node;

/**
 * Base class for all ObjectNodes, mainly Operators that have a single result.
 * 
 * @author Björn Clasen
 *
 */
public interface ObjectNode extends Node {

	/**
	 * Retrieves the result of this ObjectNode/Operator.<br>
	 * {@link Node#typeCheck()} must have been called beforehand.
	 * 
	 * @return the result as a MemoryObject, {@code null} if it's an undefined
	 *         attribute.
	 * @throws MemoryException
	 *             if during tree evaluation memory tended to run out. Execution
	 *             is then canceled and intermediate results are discarded.
	 */
	public MemoryObject getResult() throws MemoryException;

}
