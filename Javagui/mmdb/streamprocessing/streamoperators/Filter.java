package mmdb.streamprocessing.streamoperators;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * The filter operator resembling the operator in the core.<br>
 * Filters a Stream if elements by evaluating a boolean function over each
 * element. Uses a ParameterFunction for this purpose.
 * 
 * @author Björn Clasen
 *
 */
public class Filter implements StreamOperator {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's second parameter as a ParameterFunction.
	 */
	private ParameterFunction functionInput;

	/**
	 * The operator's output type (equals the OutputType of the input stream).
	 */
	private MemoryObject outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Filter.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Filter(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Filter(Node input1, Node input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();

		// Is input1 a StreamOperator?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input1;

		// Is input2 a ParamterFunction?
		TypecheckTools.checkNodeType(this.input2, ParameterFunction.class,
				this.getClass(), 2);
		this.functionInput = (ParameterFunction) this.input2;

		// Typecheck ParameterFunction
		this.functionInput.setParamTypes(this.streamInput.getOutputType());
		this.functionInput.typeCheck(); // !!! Important !!!

		// Does the ParameterFunction provide an ObjectNode providing an
		// AttributeBool?
		TypecheckTools.checkFunctionOperatorType(this.functionInput,
				ObjectNode.class, this.getClass(), 2);
		TypecheckTools.checkFunctionOutputType(this.functionInput,
				AttributeBool.class, this.getClass(), 2);

		this.outputType = this.streamInput.getOutputType();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();
	}

	/**
	 * {@inheritDoc}<br>
	 * Passes the element to the ParameterFunction and evaluates it's output
	 * operator.
	 */
	@Override
	public MemoryObject getNext() throws MemoryException {
		MemoryObject inputObject = null;
		boolean matchesCondition = false;
		while (!matchesCondition
				&& (inputObject = this.streamInput.getNext()) != null) {
			ObjectNode functionResult = (ObjectNode) this.functionInput
					.evaluate(inputObject);
			AttributeBool resultAttribute = (AttributeBool) functionResult
					.getResult();
			if (resultAttribute == null) {
				matchesCondition = false;
			} else {
				matchesCondition = resultAttribute.isValue();
			}
		}
		return inputObject;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.streamInput.close();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

}
