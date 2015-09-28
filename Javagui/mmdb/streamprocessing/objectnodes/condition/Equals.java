package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryObjects;
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
 * Implementation of operator equals resembling the core operator.<br>
 * Checks if the first parameter equals the second.
 * 
 * @author Bjoern Clasen
 */
public class Equals implements ObjectNode {

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
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Equals.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Equals(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Equals(Node input1, Node input2) {
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

		this.outputType = new AttributeBool();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getResult() throws MemoryException {
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

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getOutputType() {
		return this.outputType;
	}

	/**
	 * Checks if the OutputTypes of the parameters match so they can be
	 * compared.
	 * 
	 * @throws TypeException
	 *             if the OutputTypes do not match.
	 */
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
