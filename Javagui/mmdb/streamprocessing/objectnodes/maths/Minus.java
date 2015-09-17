package mmdb.streamprocessing.objectnodes.maths;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Operator minus(-) resembling the core operator.<br>
 * Subtracts two numbers.
 * 
 * @author Björn Clasen
 *
 */
public class Minus implements ObjectNode {

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
	private MemoryObject outputType;

	/**
	 * An enum stating the possible input variants of this operator.
	 */
	private enum InputVariant {
		INT_INT, INT_REAL, REAL_INT, REAL_REAL
	}

	/**
	 * Stores the currently active input variant, detected during typecheck.
	 */
	private InputVariant inputVariant;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Minus.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Minus(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Minus(Node input1, Node input2) {
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

		this.inputVariant = determineInputVariant(
				this.objectInput1.getOutputType(),
				this.objectInput2.getOutputType());

		// OutputType set in "determineInputVariant"
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getResult() throws MemoryException {
		if (this.objectInput1.getResult() == null
				|| this.objectInput2.getResult() == null) {
			return null;
		}

		switch (this.inputVariant) {
		case INT_INT:
			return getIntIntResult();
		case INT_REAL:
			return getIntRealResult();
		case REAL_INT:
			return getRealIntResult();
		case REAL_REAL:
			return getRealRealResult();
		default:
			return null;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * Calculates Minus' result based on integers as input.
	 * 
	 * @return the result of the integer subtraction.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private MemoryAttribute getIntIntResult() throws MemoryException {
		int intValue1 = ((AttributeInt) this.objectInput1.getResult())
				.getValue();
		int intValue2 = ((AttributeInt) this.objectInput2.getResult())
				.getValue();
		return new AttributeInt(intValue1 - intValue2);
	}

	/**
	 * Calculates Minus' result based on integer and real as input.
	 * 
	 * @return the result of the integer-real subtraction.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private MemoryAttribute getIntRealResult() throws MemoryException {
		int intValue = ((AttributeInt) this.objectInput1.getResult())
				.getValue();
		float realValue = ((AttributeReal) this.objectInput2.getResult())
				.getValue();

		return new AttributeReal(intValue - realValue);
	}

	/**
	 * Calculates Minus' result based on real and integers as input.
	 * 
	 * @return the result of the real-integer subtraction.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private MemoryAttribute getRealIntResult() throws MemoryException {
		float realValue = ((AttributeReal) this.objectInput1.getResult())
				.getValue();
		int intValue = ((AttributeInt) this.objectInput2.getResult())
				.getValue();
		return new AttributeReal(realValue - intValue);
	}

	/**
	 * Calculates Minus' result based on reals as input.
	 * 
	 * @return the result of the real subtraction.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private MemoryAttribute getRealRealResult() throws MemoryException {
		float realValue1 = ((AttributeReal) this.objectInput1.getResult())
				.getValue();
		float realValue2 = ((AttributeReal) this.objectInput2.getResult())
				.getValue();
		return new AttributeReal(realValue1 - realValue2);
	}

	/**
	 * Determines the present InputVariant.
	 * 
	 * @param obj1
	 *            the OutputType of the first parameter.
	 * @param obj2
	 *            the OutputType of the second parameter.
	 * @return detected InputVariant.
	 * @throws TypeException
	 *             if no valid InputVariant could be detected.
	 */
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

		throw new TypeException("%s's %ss provide invalid datatypes: %s + %s.",
				this.getClass().getSimpleName(), Nodes.NodeType.ObjectNode,
				obj1.getClass().getSimpleName(), obj2.getClass()
						.getSimpleName());
	}

}
