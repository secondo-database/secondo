package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Contains implements ObjectNode {

	private Node input1, input2;

	private ObjectNode objectInput1, objectInput2;

	private MemoryAttribute outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Contains.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Contains(node1, node2);
	}

	public Contains(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();

		// Is input1 an ObjectNode providing an AttributeString?
		TypecheckTools.checkNodeType(this.input1, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput1 = (ObjectNode) this.input1;
		TypecheckTools.checkOutputType(this.objectInput1,
				AttributeString.class, this.getClass(), 1);

		// Is input2 an ObjectNode providing an AttributeString?
		TypecheckTools.checkNodeType(this.input2, ObjectNode.class,
				this.getClass(), 2);
		this.objectInput2 = (ObjectNode) this.input2;
		TypecheckTools.checkOutputType(this.objectInput2,
				AttributeString.class, this.getClass(), 2);

		this.outputType = new AttributeBool();
	}

	@Override
	public MemoryAttribute getOutputType() {
		return this.outputType;
	}

	@Override
	public MemoryAttribute getResult() {
		AttributeString string1 = (AttributeString) this.objectInput1
				.getResult();
		AttributeString string2 = (AttributeString) this.objectInput2
				.getResult();

		if (string1 == null || string2 == null) {
			return null;
		}

		return new AttributeBool(string1.getValue()
				.contains(string2.getValue()));
	}

}
