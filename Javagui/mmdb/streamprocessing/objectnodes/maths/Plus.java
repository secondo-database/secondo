package mmdb.streamprocessing.objectnodes.maths;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Plus implements ObjectNode {

	private Node input1, input2;

	private ObjectNode objectInput1, objectInput2;

	private MemoryObject outputType;

	private InputVariant inputVariant;

	private enum InputVariant {
		INT_INT, INT_REAL, REAL_INT, REAL_REAL, STRING_STRING
	}

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Plus.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Plus(node1, node2);
	}

	public Plus(Node input1, Node input2) {
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

		this.inputVariant = determineInputVariant(
				this.objectInput1.getOutputType(),
				this.objectInput2.getOutputType());

		// OutputType set in "determineInputVariant"
	}

	@Override
	public MemoryObject getResult() {
		if (this.objectInput1.getResult() == null
				|| this.objectInput2.getResult() == null) {
			return null;
		}

		switch (this.inputVariant) {
		case INT_INT:
			return getIntIntResult();
		case INT_REAL:
			return getIntRealResult(false);
		case REAL_INT:
			return getIntRealResult(true);
		case REAL_REAL:
			return getRealRealResult();
		case STRING_STRING:
			return getStringStringResult();
		default:
			return null;
		}
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	private MemoryAttribute getIntIntResult() {
		int intValue1 = ((AttributeInt) this.objectInput1.getResult())
				.getValue();
		int intValue2 = ((AttributeInt) this.objectInput2.getResult())
				.getValue();
		return new AttributeInt(intValue1 + intValue2);
	}

	private MemoryAttribute getIntRealResult(boolean reverse) {
		int intValue;
		float realValue;
		if (reverse) {
			intValue = ((AttributeInt) this.objectInput2.getResult())
					.getValue();
			realValue = ((AttributeReal) this.objectInput1.getResult())
					.getValue();
		} else {
			intValue = ((AttributeInt) this.objectInput1.getResult())
					.getValue();
			realValue = ((AttributeReal) this.objectInput2.getResult())
					.getValue();
		}
		return new AttributeReal(intValue + realValue);
	}

	private MemoryAttribute getRealRealResult() {
		float realValue1 = ((AttributeReal) this.objectInput1.getResult())
				.getValue();
		float realValue2 = ((AttributeReal) this.objectInput2.getResult())
				.getValue();
		return new AttributeReal(realValue1 + realValue2);
	}

	private MemoryAttribute getStringStringResult() {
		String stringValue1 = ((AttributeString) this.objectInput1.getResult())
				.getValue();
		String stringValue2 = ((AttributeString) this.objectInput2.getResult())
				.getValue();
		return new AttributeString(stringValue1 + stringValue2);
	}

	private InputVariant determineInputVariant(MemoryObject obj1,
			MemoryObject obj2) throws TypeException {
		if (obj1.getClass() == AttributeInt.class) {
			if (obj2.getClass() == AttributeInt.class) {
				this.outputType = new AttributeInt();
				return InputVariant.INT_INT;
			}
			if (obj2.getClass() == AttributeReal.class) {
				this.outputType = new AttributeReal();
				return InputVariant.INT_REAL;
			}
		}

		if (obj1.getClass() == AttributeReal.class) {
			if (obj2.getClass() == AttributeReal.class) {
				this.outputType = new AttributeReal();
				return InputVariant.REAL_REAL;
			}
			if (obj2.getClass() == AttributeInt.class) {
				this.outputType = new AttributeReal();
				return InputVariant.REAL_INT;
			}
		}

		if (obj1 instanceof AttributeString
				&& obj1.getClass() == obj2.getClass()) {
			this.outputType = new AttributeString();
			return InputVariant.STRING_STRING;
		}

		throw new TypeException("%s's %ss provide invalid datatypes: %s + %s.",
				this.getClass().getSimpleName(), Nodes.NodeType.ObjectNode,
				obj1.getClass().getSimpleName(), obj2.getClass()
						.getSimpleName());
	}

}
