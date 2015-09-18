package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Implementation of operator feed resembling the core operator.<br>
 * Transforms a MemoryRelation into a stream of MemoryTuples.
 * 
 * @author Bjoern Clasen
 */
public class Feed implements StreamOperator {

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
	private MemoryTuple outputType;

	/**
	 * An iterator over the MemoryTuples in the MemoryRelation.
	 */
	private Iterator<MemoryTuple> iterator;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 1, Feed.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Feed(node1);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's parameter
	 */
	public Feed(Node input) {
		this.input = input;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input, ObjectNode.class,
				this.getClass(), 1);
		this.objectInput = (ObjectNode) this.input;

		TypecheckTools.checkOutputType(this.objectInput, MemoryRelation.class,
				this.getClass(), 1);

		// Any identifier duplications?
		checkIdentifierDuplications();

		this.outputType = MemoryTuple
				.createTypecheckInstance(((MemoryRelation) this.objectInput
						.getOutputType()).getTypecheckInfo());
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		MemoryRelation inputRelation = (MemoryRelation) this.objectInput
				.getResult();
		if (inputRelation == null) {
			this.iterator = new ArrayList<MemoryTuple>().iterator();
		} else {
			this.iterator = inputRelation.getTuples().iterator();
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getNext() {
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
		this.iterator = null;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryObject getOutputType() {
		return this.outputType;
	}

	/**
	 * Checks the input MemoryRelation for identifier duplications since they
	 * prevent any identifier based operations on this stream to work.
	 * 
	 * @throws TypeException
	 *             if there are any identifier duplications.
	 */
	private void checkIdentifierDuplications() throws TypeException {
		List<RelationHeaderItem> relationHeader = ((MemoryRelation) this.objectInput
				.getOutputType()).getTypecheckInfo();
		Set<String> identifiers = new HashSet<String>();

		for (RelationHeaderItem item : relationHeader) {
			if (!identifiers.add(item.getIdentifier())) {
				throw new TypeException(String.format(
						"%s: Identifier duplication in input relation: %s",
						this.getClass().getSimpleName(), item.getIdentifier()));
			}
		}
	}

}
