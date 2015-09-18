package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Simple product operator resembling the core operator.<br>
 * Creates the cartesian product of two tuple streams.
 * 
 * @author Bjoern Clasen
 *
 */
public class Product implements StreamOperator {

	/**
	 * The operator's parameter Nodes.
	 */
	private Node input1, input2;

	/**
	 * The operators parameters as StreamOperators.
	 */
	private StreamOperator streamInput1, streamInput2;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * Stores the current input of the left stream.
	 */
	private MemoryTuple currentInputLeft;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Product.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Product(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Product(Node input1, Node input2) {
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

		// Is input1 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input1, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput1 = (StreamOperator) this.input1;
		TypecheckTools.checkOutputType(this.streamInput1, MemoryTuple.class,
				this.getClass(), 1);

		// Is input1 a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input2, StreamOperator.class,
				this.getClass(), 2);
		this.streamInput2 = (StreamOperator) this.input2;
		TypecheckTools.checkOutputType(this.streamInput2, MemoryTuple.class,
				this.getClass(), 2);

		this.checkIdentifierDuplications();

		this.outputType = MemoryTuple
				.createTypecheckInstance(calculateOutputType());
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void open() throws MemoryException {
		this.streamInput1.open();
		this.streamInput2.open();
		this.currentInputLeft = (MemoryTuple) this.streamInput1.getNext();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getNext() throws MemoryException {
		if (this.currentInputLeft == null) {
			return null;
		}

		MemoryTuple currentInputRight = (MemoryTuple) this.streamInput2
				.getNext();
		if (currentInputRight == null) {
			this.currentInputLeft = (MemoryTuple) this.streamInput1.getNext();
			if (this.currentInputLeft == null) {
				return null;
			}
			this.streamInput2.close();
			this.streamInput2.open();
			currentInputRight = (MemoryTuple) this.streamInput2.getNext();
			// If right stream had no tuples at all
			if (currentInputRight == null) {
				return null;
			}
		}

		MemoryTuple resultTuple = new MemoryTuple();
		for (MemoryAttribute attr : currentInputLeft.getAttributes()) {
			resultTuple.addAttribute(attr);
		}
		for (MemoryAttribute attr : currentInputRight.getAttributes()) {
			resultTuple.addAttribute(attr);
		}

		return resultTuple;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void close() {
		this.streamInput1.close();
		this.streamInput2.close();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryTuple getOutputType() {
		return this.outputType;
	}

	/**
	 * Determines the output type by concatenating the headers.
	 * 
	 * @return the concatenated headers.
	 */
	private List<RelationHeaderItem> calculateOutputType() {
		List<RelationHeaderItem> outputType = new ArrayList<RelationHeaderItem>();
		outputType.addAll(((MemoryTuple) this.streamInput1.getOutputType())
				.getTypecheckInfo());
		outputType.addAll(((MemoryTuple) this.streamInput2.getOutputType())
				.getTypecheckInfo());
		return outputType;
	}

	/**
	 * Checks the identifiers of both input stream's tuples for duplications.
	 * 
	 * @throws TypeException
	 *             if any duplication has been found.
	 */
	private void checkIdentifierDuplications() throws TypeException {
		for (RelationHeaderItem input1Header : ((MemoryTuple) this.streamInput1
				.getOutputType()).getTypecheckInfo()) {
			for (RelationHeaderItem input2Header : ((MemoryTuple) this.streamInput2
					.getOutputType()).getTypecheckInfo()) {
				if (input1Header.getIdentifier().equals(
						input2Header.getIdentifier())) {
					throw new TypeException("Identifier duplication: "
							+ input1Header.getIdentifier());
				}
			}
		}
	}

}
