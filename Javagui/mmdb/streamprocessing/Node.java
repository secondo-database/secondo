package mmdb.streamprocessing;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

/**
 * Base interface for all Nodes.
 * 
 * @author Bjoern Clasen
 */
public interface Node {

	/**
	 * Performs a recursive typecheck on this Node.<br>
	 * Determines OutputType so {@link Node#getOutputType()} does not work
	 * before the Node has been typechecked.
	 * 
	 * @throws TypeException
	 *             if any type error is found. Recursive process is completely
	 *             cancelled.
	 */
	public void typeCheck() throws TypeException;

	/**
	 * Returns the OutputType of this Node represented by a TypecheckInstance.<br>
	 * For most MemoryAttributes you will only need the TypecheckInstance's
	 * class.<br>
	 * For MemoryRelations, MemoryTuples and complex MemoryAttributes you will
	 * need the TypecheckInfo which is obtained by calling
	 * <code>getTypecheckInfo()</code> on the return value of this method.
	 * 
	 * @return
	 */
	public MemoryObject getOutputType();

}
