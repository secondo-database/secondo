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
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;

public class Groupby implements StreamOperator {

	private Node input1;

	private String[] identifiers;

	private Map<String, Node> input3;

	private Map<Integer, Integer> columnPositions;

	private StreamOperator streamInput;

	private MemoryTuple outputType;

	// Needed for evaluation

	private List<MemoryTuple> tupleStore;

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 3, Groupby.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		String[] identList = NestedListProcessor.nlToIdentifierArray(params[1]);
		Map<String, Node> paramMap = NestedListProcessor
				.nlToIdentifierNodePairs(params[2], environment);
		return new Groupby(node1, identList, paramMap);
	}

	public Groupby(Node input1, String[] identifiers, Map<String, Node> input3) {
		this.input1 = input1;
		this.identifiers = identifiers;
		this.input3 = input3;
	}

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

	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	@Override
	public void open() {
		this.streamInput.open();
		this.tupleStore = new ArrayList<MemoryTuple>();
		MemoryTuple firstTuple = (MemoryTuple) this.streamInput.getNext();
		if (firstTuple != null) {
			this.tupleStore.add(firstTuple);
		}
	}

	@Override
	public MemoryObject getNext() {
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
		while ((inputTuple = (MemoryTuple) this.streamInput.getNext()) != null
				&& tuplesMatchOnGroupingColumns(inputTuple, tupleStore.get(0))) {
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

	@Override
	public void close() {
		this.streamInput.close();
	}

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

	private MemoryTuple calculateOutputTuple() {
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
