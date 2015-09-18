package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * The extend operator resembling the operator in the core.<br>
 * Adds columns to tuples in a stream by performing calculations on the existing
 * columns.
 * 
 * @author Bjoern Clasen
 */
public class Extend implements StreamOperator {

	/**
	 * The operator's first parameter as a Node.
	 */
	private Node input1;

	/**
	 * The operator's second parameter.<br>
	 * A map mapping the new identifiers to their calculation functions.
	 */
	private Map<String, Node> input2;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Extend.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Map<String, Node> paramMap = NestedListProcessor
				.nlToIdentifierNodePairs(params[1], environment);
		return new Extend(node1, paramMap);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Extend(Node input1, Map<String, Node> input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();

		// Is input1 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		TypecheckTools.checkOutputType(this.input1, MemoryTuple.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input1;

		// Check <identifier, function> Pairs
		int inputNumber = 3;
		for (Node node : this.input2.values()) {
			// Is the Node a Function?
			TypecheckTools.checkNodeType(node, ParameterFunction.class,
					this.getClass(), inputNumber);
			ParameterFunction functionNode = (ParameterFunction) node;

			// Typecheck
			functionNode.setParamTypes(this.streamInput.getOutputType());
			node.typeCheck();

			// Does Function provide an ObjectNode providing Attributes?
			TypecheckTools.checkFunctionOperatorType(functionNode,
					ObjectNode.class, this.getClass(), inputNumber);
			TypecheckTools.checkFunctionOutputType(functionNode,
					MemoryAttribute.class, this.getClass(), inputNumber);

			inputNumber += 2;
		}

		this.outputType = calculateOutputType();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() throws MemoryException {
		MemoryTuple inputTuple = (MemoryTuple) this.streamInput.getNext();

		// Has input stream ended?
		if (inputTuple == null) {
			return null;
		}

		// Add all old attributes
		MemoryTuple outputTuple = new MemoryTuple();
		for (MemoryAttribute attr : inputTuple.getAttributes()) {
			outputTuple.addAttribute(attr);
		}

		// Add all new attributes
		for (Node node : this.input2.values()) {
			ParameterFunction fun = (ParameterFunction) node;
			MemoryAttribute newAttr = (MemoryAttribute) ((ObjectNode) fun
					.evaluate(inputTuple)).getResult();
			outputTuple.addAttribute(newAttr);
		}

		return outputTuple;
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

	/**
	 * Calculates the OutputType by adding new columns to the header of the
	 * input stream.
	 * 
	 * @return the TypecheckInstance of the MemoryTuples that are being put out.
	 * @throws TypeException
	 *             if new identifiers already exist in the input tuples.
	 */
	private MemoryTuple calculateOutputType() throws TypeException {
		// Retrieve OutputType of streamInput
		MemoryObject inputTypecheckInstance = this.streamInput.getOutputType();
		List<RelationHeaderItem> inputTypecheckInfo = ((MemoryTuple) inputTypecheckInstance)
				.getTypecheckInfo();

		List<RelationHeaderItem> newTypecheckInfo = new ArrayList<RelationHeaderItem>();
		newTypecheckInfo.addAll(inputTypecheckInfo);

		// Add all new Attributes
		for (String identifier : this.input2.keySet()) {
			// No identifier duplications?
			HeaderTools.checkIdentifiersNotPresent(inputTypecheckInfo,
					this.getClass(), identifier);

			// New Attribute Type
			MemoryAttribute attributeType = (MemoryAttribute) ((ParameterFunction) this.input2
					.get(identifier)).getOutputType();
			RelationHeaderItem newItem = new RelationHeaderItem(identifier,
					MemoryAttribute.getTypeName(attributeType.getClass()));
			newTypecheckInfo.add(newItem);
		}

		return MemoryTuple.createTypecheckInstance(newTypecheckInfo);
	}

}
