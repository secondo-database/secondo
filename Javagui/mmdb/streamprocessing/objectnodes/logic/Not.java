package mmdb.streamprocessing.objectnodes.logic;

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

public class Not implements ObjectNode {

	private Node input;

	private ObjectNode objectInput;

	private MemoryAttribute outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Not.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Not(node1);
	}

	public Not(Node input) {
		this.input = input;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input an ObjectNode providing an AttributeBool?
		TypecheckTools.checkNodeType(this.input, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput = (ObjectNode) this.input;
		TypecheckTools.checkOutputType(this.objectInput, AttributeBool.class,
				this.getClass(), 1);

		this.outputType = new AttributeBool();
	}

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	@Override
	public MemoryObject getResult() {
		AttributeBool inputBool = (AttributeBool) this.objectInput.getResult();

		if (inputBool == null) {
			return null;
		}

		return new AttributeBool(!inputBool.isValue());
	}

}
