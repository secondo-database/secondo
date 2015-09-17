package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.service.MemoryWatcher;
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
 * The groupby operator resembling the operator in the core.<br>
 * Groups a (presorted) stream of tuples by the given identifiers. Calculates
 * aggregates of other columns and adds them as new columns.
 * 
 * @author Björn Clasen
 */
public class Groupby implements StreamOperator {

	/**
	 * The operator's first parameter as a Node.
	 */
	private Node input1;

	/**
	 * The identifiers to group by.
	 */
	private String[] identifiers;

	/**
	 * The operator's third parameter.<br>
	 * A map mapping the new identifiers to their aggregation functions.
	 */
	private Map<String, Node> input3;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * Positions of the grouping columns in the incoming tuples.
	 */
	private Map<Integer, Integer> columnPositions;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * Stores tuples matching in the grouping columns
	 */
	private List<MemoryTuple> tupleStore;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 3, Groupby.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identList = NestedListProcessor.nlToIdentifierArray(params[1]);
		Map<String, Node> paramMap = NestedListProcessor
				.nlToIdentifierNodePairs(params[2], environment);
		return new Groupby(node1, identList, paramMap);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param identifiers
	 *            operator's second parameter
	 * @param input3
	 *            operator's third parameter
	 */
	public Groupby(Node input1, String[] identifiers, Map<String, Node> input3) {
		this.input1 = input1;
		this.identifiers = identifiers;
		this.input3 = input3;
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
		this.streamInput = (StreamOperator) this.input1;
		TypecheckTools.checkOutputType(this.streamInput, MemoryTuple.class,
				this.getClass(), 1);

		// Are all grouping identifiers present?
		List<RelationHeaderItem> inputHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		HeaderTools.checkIdentifiersPresent(inputHeader, this.getClass(),
				this.identifiers);

		// Check <identifier, function> Pairs
		int inputNumber = 1 + this.identifiers.length + 2;
		for (Node node : this.input3.values()) {
			// Is the Node a Function?
			TypecheckTools.checkNodeType(node, ParameterFunction.class,
					this.getClass(), inputNumber);
			ParameterFunction functionNode = (ParameterFunction) node;

			// Typecheck ParameterFunction
			functionNode.setParamTypes(MemoryRelation
					.createTypecheckInstance(((MemoryTuple) this.streamInput
							.getOutputType()).getTypecheckInfo()));
			node.typeCheck();

			// Does Function provide an ObjectNode providing an Attribute?
			TypecheckTools.checkFunctionOperatorType(functionNode,
					ObjectNode.class, this.getClass(), inputNumber);
			TypecheckTools.checkFunctionOutputType(functionNode,
					MemoryAttribute.class, this.getClass(), inputNumber);

			inputNumber += 2;
		}

		this.outputType = calculateOutputType();
	}

	/**
	 * {@inheritDoc}<br>
	 * Initializes the tuple store.
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();
		this.tupleStore = new ArrayList<MemoryTuple>();
		MemoryTuple firstTuple = (MemoryTuple) this.streamInput.getNext();
		if (firstTuple != null) {
			this.tupleStore.add(firstTuple);
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() throws MemoryException {
		// Is stream opened?
		if (this.tupleStore == null) {
			throw new StreamStateException(
					"Groupby: Stream was accessed while being closed");
		}

		// Has stream ended?
		if (this.tupleStore.size() != 1) {
			return null;
		}

		// Collect matching tuples in tupleStore
		MemoryTuple inputTuple;
		int memorywatch_counter = 0;
		while ((inputTuple = (MemoryTuple) this.streamInput.getNext()) != null
				&& tuplesMatchOnGroupingColumns(inputTuple, tupleStore.get(0))) {
			memorywatch_counter++;
			if (memorywatch_counter % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
			this.tupleStore.add(inputTuple);
		}

		MemoryTuple outputTuple = calculateOutputTuple();

		// Prepare tupleStore for next call
		this.tupleStore = new ArrayList<MemoryTuple>();
		if (inputTuple != null) {
			this.tupleStore.add(inputTuple);
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
	 * Determines the output type by creating a new header containing the
	 * grouping columns and the aggregate columns.
	 * 
	 * @return the determinded OutputType.
	 * @throws TypeException
	 *             if identifier duplications were found.
	 */
	private MemoryTuple calculateOutputType() throws TypeException {
		this.initPositions();

		MemoryObject inputTypecheckInstance = this.streamInput.getOutputType();
		List<RelationHeaderItem> inputTypecheckInfo = ((MemoryTuple) inputTypecheckInstance)
				.getTypecheckInfo();

		List<RelationHeaderItem> newTypecheckInfo = new ArrayList<RelationHeaderItem>();

		// Add all columns that are used for grouping
		for (String identifier : this.identifiers) {
			for (RelationHeaderItem item : inputTypecheckInfo) {
				if (identifier.equals(item.getIdentifier())) {
					newTypecheckInfo.add(item);
					break;
				}
			}
		}

		// Add all newly calculated columns
		for (String identifier : this.input3.keySet()) {
			HeaderTools.checkIdentifiersNotPresent(newTypecheckInfo,
					this.getClass(), identifier);

			MemoryAttribute attributeType = (MemoryAttribute) ((ParameterFunction) this.input3
					.get(identifier)).getOutputType();
			RelationHeaderItem newItem = new RelationHeaderItem(identifier,
					MemoryAttribute.getTypeName(attributeType.getClass()));
			newTypecheckInfo.add(newItem);
		}

		return MemoryTuple.createTypecheckInstance(newTypecheckInfo);
	}

	/**
	 * Calculates the postions of the grouping columns in the incoming tuples.
	 * 
	 * @throws TypeException
	 *             if any grouping columns were not present.
	 */
	private void initPositions() throws TypeException {
		this.columnPositions = new HashMap<Integer, Integer>();
		List<RelationHeaderItem> oldHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		for (int i = 0; i < this.identifiers.length; i++) {
			this.columnPositions.put(i, HeaderTools
					.getHeaderIndexForIdentifier(oldHeader,
							this.identifiers[i], this.getClass()));
		}
	}

	/**
	 * Checks if two tuples match on all grouping columns.
	 * 
	 * @param tuple1
	 * @param tuple2
	 * @return true if the tuples match on all grouping columns, false
	 *         otherwise.
	 */
	private boolean tuplesMatchOnGroupingColumns(MemoryTuple tuple1,
			MemoryTuple tuple2) {
		for (int position : this.columnPositions.values()) {
			if (!tuple1.getAttribute(position).equals(
					tuple2.getAttribute(position))) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Calculates a single output tuple (group) by using the tuple store.
	 * 
	 * @return the tuple to put out.
	 * @throws MemoryException
	 *             if memory tended to run out during calculation.
	 */
	private MemoryTuple calculateOutputTuple() throws MemoryException {
		MemoryTuple outputTuple = new MemoryTuple();

		// Add all grouping columns (in correct order)
		for (int i = 0; i < this.identifiers.length; i++) {
			int position = columnPositions.get(i);
			outputTuple.addAttribute(tupleStore.get(0).getAttribute(position));
		}

		// Create parameter relation
		MemoryRelation parameterRelation = new MemoryRelation(
				((MemoryTuple) this.streamInput.getOutputType())
						.getTypecheckInfo());
		parameterRelation.setTuples(this.tupleStore);

		// Calculate new columns
		for (Node node : this.input3.values()) {
			ParameterFunction fun = (ParameterFunction) node;
			MemoryAttribute newAttr = (MemoryAttribute) ((ObjectNode) fun
					.evaluate(parameterRelation)).getResult();
			outputTuple.addAttribute(newAttr);
		}

		return outputTuple;
	}

}
