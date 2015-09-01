package mmdb.streamprocessing.objectnodes.logic;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.tools.TypecheckTools;

public abstract class LogicalOperator implements ObjectNode {

	private Node input1, input2;

	private ObjectNode objectInput1, objectInput2;

	private MemoryAttribute outputType;

	public LogicalOperator(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

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

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	@Override
	public MemoryObject getResult() {
		AttributeBool inputBool1 = (AttributeBool) this.objectInput1
				.getResult();
		AttributeBool inputBool2 = (AttributeBool) this.objectInput2
				.getResult();
		return new AttributeBool(calcResult(inputBool1, inputBool2));
	}

	protected abstract boolean calcResult(AttributeBool inputBool1,
			AttributeBool inputBool2);

}
