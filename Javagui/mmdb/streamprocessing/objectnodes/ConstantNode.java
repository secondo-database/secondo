package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryObject;
import mmdb.error.streamprocessing.TypeException;

/**
 * Class wrapping a MemoryObject in an ObjectNode.<br>
 * This is done to hide and obliviate the fact if an Operator's parameter is a
 * constant or a calculated object.
 * 
 * @author Björn Clasen
 */
public class ConstantNode implements ObjectNode {

	/**
	 * The MemoryObject and its TypecheckInstance.
	 */
	private MemoryObject object, typecheckInstance;

	/**
	 * Private constructor for the factory method.
	 * 
	 * @param object
	 *            The MemoryObject to wrap.
	 * @param typecheckInstance
	 *            The corresponding TypecheckInstance to the MemoryObject.
	 */
	private ConstantNode(MemoryObject object, MemoryObject typecheckInstance) {
		this.object = object;
		this.typecheckInstance = typecheckInstance;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		if (this.typecheckInstance == null) {
			throw new TypeException("Typecheck Instance for Wrapper was null!");
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getResult() {
		return this.object;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.typecheckInstance;
	}

	/**
	 * Factory method for ConstantNodes.
	 * 
	 * @param object
	 *            The MemoryObject to wrap.
	 * @param typecheckInstance
	 *            The corresponding TypecheckInstance to the MemoryObject.
	 * @return the newly instantiated ConstantNode.
	 */
	public static ObjectNode createConstantNode(MemoryObject object,
			MemoryObject typecheckInstance) {
		return new ConstantNode(object, typecheckInstance);
	}

}
