package mmdb.streamprocessing.objectnodes.logic;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.tools.TypecheckTools;

/**
 * Abstract base class for logical operators with two parameters.
 * 
 * @author Bjoern Clasen
 *
 */
public abstract class LogicalOperator implements ObjectNode {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2;

	/**
	 * The operators parameters as ObjectNodes.
	 */
	private ObjectNode objectInput1, objectInput2;

	/**
	 * The operator's output type.
	 */
	private MemoryAttribute outputType;

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public LogicalOperator(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();

		// Is input1 an ObjectNode providing an AttributeBool?
		TypecheckTools.checkNodeType(this.input1, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput1 = (ObjectNode) this.input1;
		TypecheckTools.checkOutputType(this.objectInput1, AttributeBool.class,
				this.getClass(), 1);

		// Is input2 an ObjectNode providing an AttributeBool?
		TypecheckTools.checkNodeType(this.input2, ObjectNode.class,
				this.getClass(), 2);
		this.objectInput2 = (ObjectNode) this.input2;
		TypecheckTools.checkOutputType(this.objectInput2, AttributeBool.class,
				this.getClass(), 2);

		this.outputType = new AttributeBool();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getResult() throws MemoryException {
		AttributeBool inputBool1 = (AttributeBool) this.objectInput1
				.getResult();
		AttributeBool inputBool2 = (AttributeBool) this.objectInput2
				.getResult();
		return new AttributeBool(calcResult(inputBool1, inputBool2));
	}

	/**
	 * Calculates the result of this operators based on the two given
	 * AttributeBools.
	 * 
	 * @param inputBool1
	 *            first AttributeBool.
	 * @param inputBool2
	 *            second AttributeBool.
	 * @return the result of this operator as boolean value.
	 */
	protected abstract boolean calcResult(AttributeBool inputBool1,
			AttributeBool inputBool2);

}
