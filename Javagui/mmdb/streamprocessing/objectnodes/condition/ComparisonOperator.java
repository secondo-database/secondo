package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryObjects;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.tools.TypecheckTools;

public abstract class ComparisonOperator implements ObjectNode {

	protected Node input1, input2;

	protected ObjectNode objectInput1, objectInput2;

	protected MemoryAttribute outputType;

	public ComparisonOperator(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();
		
		// Is input1 an ObjectNode?
		TypecheckTools.checkNodeType(this.input1, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput1 = (ObjectNode) this.input1;

		// Is input2 an ObjectNode?
		TypecheckTools.checkNodeType(this.input2, ObjectNode.class,
				this.getClass(), 2);
		this.objectInput2 = (ObjectNode) this.input2;

		// Are both outputTypes the same?
		checkOutputTypesMatch();

		// Do both outputTypes implement comparable? (only check 1: they match)
		TypecheckTools.checkOutputTypeHasIFace(this.input1, Comparable.class,
				this.getClass(), 1);

		this.outputType = new AttributeBool();
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	private void checkOutputTypesMatch() throws TypeException {
		Class<?> class1 = this.objectInput1.getOutputType().getClass();
		Class<?> class2 = this.objectInput2.getOutputType().getClass();
		if (class1 != class2) {
			throw new TypeException(
					"%s's inputs are of different types: %s != %s", this
							.getClass().getSimpleName(),
					MemoryObjects.getTypeName(class1),
					MemoryObjects.getTypeName(class2));
		}
	}

}
