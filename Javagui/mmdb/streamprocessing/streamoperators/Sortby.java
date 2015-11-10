package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.features.Orderable;
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
 * Operator sortby resembling the operator in the core.<br>
 * Sorts a stream of tuples by the given columns in the given directions.<br>
 * If no direction is given, asc is taken.<br>
 * Implements Comparator-Interface to be able to provide "compare"-Method for<br>
 * <code>Collections.sort(...)</code>.
 * 
 * @author Bjoern Clasen
 */
public class Sortby implements StreamOperator, Comparator<MemoryTuple> {

	/**
	 * The operator's parameter Node.
	 */
	private Node input1;

	/**
	 * The parameter map containing columns and directions.
	 */
	private Map<String, String> input2;

	/**
	 * The operator's first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * The identifiers to sort by.
	 */
	private String[] sortIdentifiers;

	/**
	 * The positions of the sort identifiers in the incoming tuples.
	 */
	private int[] sortIdentifierPositions;

	/**
	 * An iterator over the sorted tuple list.
	 */
	private Iterator<MemoryTuple> iterator;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Sortby.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		// Parameter modification, since sorting direction is optional
		params[1] = addOrders(params[1]);
		Map<String, String> paramMap = NestedListProcessor
				.nlToIdentifierPairs(params[1]);
		return new Sortby(node1, paramMap);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 * 
	 */
	public Sortby(Node input1, Map<String, String> input2) {
		this.input1 = input1;
		this.input2 = input2;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input1.typeCheck();
		this.sortIdentifiers = this.input2.keySet().toArray(new String[] {});

		// Is input1 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input1;
		TypecheckTools.checkOutputType(this.streamInput, MemoryTuple.class,
				this.getClass(), 1);

		// Are all given Identifiers present?
		List<RelationHeaderItem> inputHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		HeaderTools.checkIdentifiersPresent(inputHeader, this.getClass(),
				this.sortIdentifiers);

		// Are all direction strings correct?
		for (String direction : this.input2.values()) {
			if (!("asc".equals(direction) || "desc".equals(direction))) {
				throw new TypeException(
						"Sortby got an invalid sort direction: " + direction);
			}
		}

		// Are all sort attributes orderable?
		for (String identifier : this.sortIdentifiers) {
			HeaderTools.checkAttributeHasIFace(inputHeader, identifier,
					Orderable.class, this.getClass());
		}

		initIdentifierPositions();

		this.outputType = (MemoryTuple) this.streamInput.getOutputType();
	}

	/**
	 * {@inheritDoc}<br>
	 * Completely loads all tuples into a sorted list.
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput.open();

		this.iterator = createIterator();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getNext() {
		if (this.iterator == null) {
			throw new StreamStateException(
					"Stream was accessod while being closed!");
		}

		if (this.iterator.hasNext()) {
			return this.iterator.next();
		} else {
			return null;
		}
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
	 * Calculates the positions of the sorting attributes in the incoming
	 * MemoryTuples.
	 */
	private void initIdentifierPositions() {
		this.sortIdentifierPositions = new int[sortIdentifiers.length];
		List<RelationHeaderItem> inputHeader = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();
		for (int i = 0; i < this.sortIdentifiers.length; i++) {
			for (int f = 0; f < inputHeader.size(); f++) {
				if (this.sortIdentifiers[i].equals(inputHeader.get(f)
						.getIdentifier())) {
					this.sortIdentifierPositions[i] = f;
				}
			}
		}
	}

	/**
	 * Adds all tuples of the input stream to a sorted list.<br>
	 * Sorts on add.
	 * 
	 * @return an iterator over the sorted list.
	 * @throws MemoryException
	 *             if while storing all incoming tuples memory tended to run
	 *             out.
	 */
	private Iterator<MemoryTuple> createIterator() throws MemoryException {
		ArrayList<MemoryTuple> allTuples = new ArrayList<MemoryTuple>();

		MemoryTuple currentTuple;
		int memorywatch_counter = 0;
		while ((currentTuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			memorywatch_counter++;
			if (memorywatch_counter % MemoryWatcher.MEMORY_CHECK_FREQUENCY == 0) {
				MemoryWatcher.getInstance().checkMemoryStatus();
			}
			allTuples.add(currentTuple);
		}

		Collections.sort(allTuples, this);
		return allTuples.iterator();
	}

	/**
	 * Compares two tuples.<br>
	 * For each sorting attribute values are compared.
	 * 
	 * @param firstTuple
	 * @param secondTuple
	 * @return true if the first tuple is further right, false otherwise
	 */
	public int compare(MemoryTuple firstTuple, MemoryTuple secondTuple) {
		for (int i = 0; i < this.sortIdentifiers.length; i++) {
			Orderable firstAttr = (Orderable) firstTuple
					.getAttribute(this.sortIdentifierPositions[i]);
			Orderable secondAttr = (Orderable) secondTuple
					.getAttribute(this.sortIdentifierPositions[i]);

			String order = this.input2.get(this.sortIdentifiers[i]);

			if (firstAttr == null && secondAttr == null) {
				if (i + 1 < this.sortIdentifiers.length) {
					continue;
				} else {
					return 0;
				}
			}

			if ("asc".equals(order)) {
				if (firstAttr == null) {
					return -1;
				}
				if (secondAttr == null) {
					return 1;
				}
				if (firstAttr.compareTo(secondAttr) < 0) {
					return -1;
				} else if (firstAttr.compareTo(secondAttr) > 0) {
					return 1;
				}
			} else {
				if (firstAttr == null) {
					return 1;
				}
				if (secondAttr == null) {
					return -1;
				}
				if (firstAttr.compareTo(secondAttr) > 0) {
					return -1;
				} else if (firstAttr.compareTo(secondAttr) < 0) {
					return 1;
				}
			}
		}
		return 0;
	}

	/**
	 * Special method for adding "asc" to sorting columns without direction.
	 * 
	 * @param listExpr
	 *            The original parameter list
	 * @return a modified parameter list with appended sorting directions
	 * @throws ParsingException
	 *             if original parameters were not in list form
	 */
	private static ListExpr addOrders(ListExpr listExpr)
			throws ParsingException {
		ListExpr resultList = ListExpr.theEmptyList();
		if (listExpr.isAtom()) {
			throw new ParsingException(
					"Expected Identifier-Pairs but got an atom: "
							+ listExpr.toString());
		}
		while (!listExpr.isEmpty()) {
			ListExpr first = listExpr.first();
			listExpr = listExpr.rest();
			if (first.isAtom()) {
				resultList = ListExpr
						.concat(resultList,
								ListExpr.twoElemList(first,
										ListExpr.symbolAtom("asc")));
			} else {
				resultList = ListExpr.concat(resultList, first);
			}
		}
		return resultList;
	}

}
