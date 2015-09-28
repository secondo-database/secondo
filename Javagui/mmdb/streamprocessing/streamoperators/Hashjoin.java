package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.service.MemoryWatcher;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.HeaderTools;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * The HashJoin operator resembling the operator in the core.<br>
 * Like the core operator it takes the second input stream for the stored
 * Hashmap.
 * 
 * @author Bjoern Clasen
 *
 */
public class Hashjoin implements StreamOperator {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2;

	/**
	 * The operator's identifier parameters.
	 */
	private String identifier1, identifier2;

	/**
	 * The operator's parameters as StreamOperators.
	 */
	private StreamOperator streamInput1, streamInput2;

	/**
	 * The indices of the identifiers in the incoming tuple streams.
	 */
	private int columnIndex1, columnIndex2;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * The stored hashmap containing the second input stream.
	 */
	private Map<MemoryAttribute, List<MemoryTuple>> map;

	/**
	 * A temporary queue to store tuples that have already been joined but not
	 * put out.
	 */
	private ConcurrentLinkedQueue<MemoryTuple> tupleStore;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 4, Hashjoin.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		String string1 = NestedListProcessor.nlToIdentifier(params[2]);
		String string2 = NestedListProcessor.nlToIdentifier(params[3]);
		return new Hashjoin(node1, node2, string1, string2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 * @param identifier1
	 *            operator's third parameter
	 * @param identifier2
	 *            operator's fourth parameter
	 */
	public Hashjoin(Node input1, Node input2, String identifier1,
			String identifier2) {
		this.input1 = input1;
		this.input2 = input2;
		this.identifier1 = identifier1;
		this.identifier2 = identifier2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.input2.typeCheck();

		// Is input1 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput1 = (StreamOperator) this.input1;
		TypecheckTools.checkOutputType(this.streamInput1, MemoryTuple.class,
				this.getClass(), 1);

		// Is input2 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input2, StreamOperator.class,
				this.getClass(), 2);
		this.streamInput2 = (StreamOperator) this.input2;
		TypecheckTools.checkOutputType(this.streamInput2, MemoryTuple.class,
				this.getClass(), 2);

		// Do join identifiers exist in relations?
		HeaderTools.checkIdentifiersPresent(((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo(), this.getClass(),
				this.identifier1);
		HeaderTools.checkIdentifiersPresent(((MemoryTuple) this.streamInput2
				.getOutputType()).getTypecheckInfo(), this.getClass(),
				this.identifier2);

		// Do identifier types match?
		checkAttributeTypesMatch();

		// Any identifier collisions?
		checkIdentifierCollisions();

		// Store column indices
		this.columnIndex1 = HeaderTools.getHeaderIndexForIdentifier(
				((MemoryTuple) this.streamInput1.getOutputType())
						.getTypecheckInfo(), this.identifier1, this.getClass());
		this.columnIndex2 = HeaderTools.getHeaderIndexForIdentifier(
				((MemoryTuple) this.streamInput2.getOutputType())
						.getTypecheckInfo(), this.identifier2, this.getClass());

		this.outputType = calcOutputType();
	}

	/**
	 * {@inheritDoc}<br>
	 * Loads the complete second input stream into a map mapping the join
	 * attribute's value to all matching tuples.
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput1.open();
		this.streamInput2.open();

		// Create Map
		this.map = new HashMap<MemoryAttribute, List<MemoryTuple>>();
		MemoryTuple curTuple;
		int memorywatch_counter = 0;
		while ((curTuple = (MemoryTuple) this.streamInput2.getNext()) != null) {
			memorywatch_counter++;
			if (memorywatch_counter % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
			addTupleToMap(curTuple);
		}
		this.streamInput2.close();

		// Initialize Store
		this.tupleStore = new ConcurrentLinkedQueue<MemoryTuple>();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() throws MemoryException {
		if (this.map == null || this.tupleStore == null) {
			throw new StreamStateException(
					"Stream was accessed while being closed!");
		}

		// If right stream was empty from the start...
		if (this.map.isEmpty()) {
			return null;
		}

		// If Store is not empty just put out first element in store
		if (!this.tupleStore.isEmpty()) {
			return this.tupleStore.poll();
		} else {
			// Fill Store with next match(es)
			MemoryTuple curTuple;
			boolean foundMatch = false;
			while (!foundMatch
					&& (curTuple = (MemoryTuple) this.streamInput1.getNext()) != null) {
				MemoryAttribute joinAttribute = curTuple
						.getAttribute(this.columnIndex1);
				List<MemoryTuple> joinCandidates = this.map.get(joinAttribute);

				if (joinCandidates != null && joinCandidates.size() != 0) {
					foundMatch = true;
					this.tupleStore = joinTuples(curTuple, joinCandidates);
				}
			}
			return this.tupleStore.poll();
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.streamInput1.close();
		this.map = null;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * Checks if the types of the columns identified by the two parameters
	 * match.<br>
	 * They have to to make joining possible.
	 * 
	 * @throws TypeException
	 *             if types do not match.
	 */
	private void checkAttributeTypesMatch() throws TypeException {
		Class<? extends MemoryAttribute> class1 = HeaderTools
				.getClassForHeaderItem(((MemoryTuple) this.streamInput1
						.getOutputType()).getTypecheckInfo(), this.identifier1,
						this.getClass());
		Class<? extends MemoryAttribute> class2 = HeaderTools
				.getClassForHeaderItem(((MemoryTuple) this.streamInput2
						.getOutputType()).getTypecheckInfo(), this.identifier2,
						this.getClass());
		if (class1 != class2) {
			throw new TypeException(
					"%s: cannot equi-join attribute types: %s != %s", this
							.getClass().getSimpleName(),
					MemoryAttribute.getTypeName(class1),
					MemoryAttribute.getTypeName(class2));
		}
	}

	/**
	 * Check if there would be any identifier collisions in the output tuples.
	 * 
	 * @throws TypeException
	 *             if any collisions would occur.
	 */
	private void checkIdentifierCollisions() throws TypeException {
		List<RelationHeaderItem> header1 = ((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo();
		List<RelationHeaderItem> header2 = ((MemoryTuple) this.streamInput2
				.getOutputType()).getTypecheckInfo();

		for (RelationHeaderItem item1 : header1) {
			for (RelationHeaderItem item2 : header2) {
				if (item1.getIdentifier().equals(item2.getIdentifier())) {
					throw new TypeException(
							"%s: identifier collision on identifier: %s", this
									.getClass().getSimpleName(),
							item1.getIdentifier());
				}
			}
		}
	}

	/**
	 * Determines the OutputType by adding all columns of both input streams.
	 * 
	 * @return the determined OutputType.
	 */
	private MemoryTuple calcOutputType() {
		List<RelationHeaderItem> header1 = ((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo();
		List<RelationHeaderItem> header2 = ((MemoryTuple) this.streamInput2
				.getOutputType()).getTypecheckInfo();

		List<RelationHeaderItem> outputHeader = new ArrayList<RelationHeaderItem>();
		outputHeader.addAll(header1);
		outputHeader.addAll(header2);
		return MemoryTuple.createTypecheckInstance(outputHeader);
	}

	/**
	 * Adds a tuple to the joining map.<br>
	 * If there are already tuples with the same value in the join column the
	 * new tuple is added to their list. If this is the first tuple with this
	 * value a new list is created.
	 * 
	 * @param curTuple
	 */
	private void addTupleToMap(MemoryTuple curTuple) {
		MemoryAttribute joinAttribute = curTuple.getAttribute(columnIndex2);
		List<MemoryTuple> curList = this.map.get(joinAttribute);
		if (curList != null) {
			curList.add(curTuple);
		} else {
			curList = new ArrayList<MemoryTuple>();
			curList.add(curTuple);
			this.map.put(joinAttribute, curList);
		}
	}

	/**
	 * Joins the current tuple with all join candidates and stores the resulst
	 * in a queue.
	 * 
	 * @param curTuple
	 *            the tuple to join.
	 * @param joinCandidates
	 *            the join candidates.
	 * @return the queue containing all joined tuples.
	 * @throws MemoryException
	 *             if during joining memory tended to run out.
	 */
	private ConcurrentLinkedQueue<MemoryTuple> joinTuples(MemoryTuple curTuple,
			List<MemoryTuple> joinCandidates) throws MemoryException {
		ConcurrentLinkedQueue<MemoryTuple> newTupleStore = new ConcurrentLinkedQueue<MemoryTuple>();
		int memorywatch_counter = 0;
		for (MemoryTuple joinCandidate : joinCandidates) {
			MemoryTuple newTuple = new MemoryTuple();
			for (MemoryAttribute attribute : curTuple.getAttributes()) {
				newTuple.addAttribute(attribute);
			}
			for (MemoryAttribute attribute : joinCandidate.getAttributes()) {
				newTuple.addAttribute(attribute);
			}
			newTupleStore.add(newTuple);
			memorywatch_counter++;
			if (memorywatch_counter % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
		}
		return newTupleStore;
	}

}
