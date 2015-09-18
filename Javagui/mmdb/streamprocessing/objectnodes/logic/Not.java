package mmdb.streamprocessing.objectnodes.logic;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Implementation of logical NOT operator resembling the core operator.<br>
 * Returns the opposite of the boolean parameter.
 * 
 * @author Bjoern Clasen
 */
public class Not implements ObjectNode {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operators parameter as an ObjectNode.
	 */
	private ObjectNode objectInput;

	/**
	 * The operator's output type.
	 */
	private MemoryAttribute outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Not.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Not(node1);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's parameter
	 */
	public Not(Node input) {
		this.input = input;
	}

	/**
	 * {@inheritDoc}
	 */
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
		AttributeBool inputBool = (AttributeBool) this.objectInput.getResult();

		if (inputBool == null) {
			return null;
		}

		return new AttributeBool(!inputBool.isValue());
	}

}
