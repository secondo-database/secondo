package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

// TODO MemoryObjects? Or only MemoryAttributes?
public class Equals implements ObjectNode {

	private Node input1, input2;

	private ObjectNode objectInput1, objectInput2;

	private MemoryAttribute outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Equals.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Equals(node1, node2);
	}

	public Equals(Node input1, Node input2) {
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

		this.outputType = new AttributeBool();
	}

	@Override
	public MemoryAttribute getResult() {
		MemoryObject object1 = objectInput1.getResult();
		MemoryObject object2 = objectInput2.getResult();

		if (object1 == null || object2 == null) {
			if (object1 == null && object2 == null) {
				return new AttributeBool(true);
			} else {
				return new AttributeBool(false);
			}
		}

		boolean result = object1.equals(object2);
		return new AttributeBool(result);
	}

	@Override
	public MemoryAttribute getOutputType() {
		return this.outputType;
	}

}
