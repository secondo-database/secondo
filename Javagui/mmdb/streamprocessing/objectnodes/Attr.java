package mmdb.streamprocessing.objectnodes;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Attr implements ObjectNode {

	private Node input;

	private ObjectNode objectInput;

	private String identifier;

	private int identifierIndex;

	private MemoryAttribute outputType;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Attr.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Attr(node, identParam);
	}

	public Attr(Node input, String identifier) {
		this.input = input;
		this.identifier = identifier;
	}

	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input an ObjectNode providing a Tuple?
		TypecheckTools.checkNodeType(this.input, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput = (ObjectNode) input;
		TypecheckTools.checkOutputType(this.objectInput, MemoryTuple.class,
				this.getClass(), 1);

		this.identifierIndex = HeaderTools.getHeaderIndexForIdentifier(
				((MemoryTuple) this.objectInput.getOutputType())
						.getTypecheckInfo(), this.identifier, this.getClass());

		this.outputType = determineOutputType();
	}

	@Override
	public MemoryAttribute getResult() {
		MemoryTuple tuple = (MemoryTuple) this.objectInput.getResult();
		if (tuple == null) {
			return null;
		}

		return tuple.getAttribute(this.identifierIndex);
	}

	@Override
	public MemoryAttribute getOutputType() {
		return outputType;
	}

	private MemoryAttribute determineOutputType() throws TypeException {
		List<RelationHeaderItem> typecheckInfo = ((MemoryTuple) this.objectInput
				.getOutputType()).getTypecheckInfo();
		try {
			// Create a new (Typecheck-)Instance of the MemoryAttribute via
			// Reflection
			return typecheckInfo.get(this.identifierIndex).getType()
					.newInstance();
		} catch (Exception e) {
			throw new TypeException(String.format(
					"Error in instantiating Attribute: %s",
					typecheckInfo.get(this.identifierIndex).getType()));
		}
	}

}
