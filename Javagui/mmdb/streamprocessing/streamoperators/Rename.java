package mmdb.streamprocessing.streamoperators;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
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
 * Operator rename resembling the operator in the core.<br>
 * Renames all columns in a stream of MemoryTuples by adding a given postfix.<br>
 * 
 * @author Bjoern Clasen
 *
 */
public class Rename implements StreamOperator {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operators first parameter as a StreamOperator.
	 */
	private StreamOperator streamInput;

	/**
	 * The postfix to add to all column names.
	 */
	private String postfix;

	/**
	 * The operator's output type.
	 */
	private MemoryTuple outputType;

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Rename.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Rename(node, identParam);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifiers
	 *            operator's postfix parameter
	 */
	public Rename(Node input, String postfix) {
		this.input = input;
		this.postfix = postfix;
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator providing Tuples?
		TypecheckTools.checkNodeType(this.input, StreamOperator.class,
				this.getClass(), 1);
		this.streamInput = (StreamOperator) this.input;
		TypecheckTools.checkOutputType(this.streamInput, MemoryTuple.class,
				this.getClass(), 1);

		this.outputType = this.calculateOutputType();
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
	public MemoryTuple getNext() throws MemoryException {
		return (MemoryTuple) this.streamInput.getNext();
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
	public MemoryTuple getOutputType() {
		return this.outputType;
	}

	/**
	 * Calculates the header of the tuples being putout by rename.<br>
	 * Does this by adding the postfix to all names.<br>
	 * This is the only real operation rename does: Tuples themself remain
	 * untouched.
	 * 
	 * @return the TypecheckInstance for all output tuples.
	 * @throws TypeException
	 *             if the postfix was {@code null} or empty.
	 */
	private MemoryTuple calculateOutputType() throws TypeException {
		List<RelationHeaderItem> inputType = ((MemoryTuple) this.streamInput
				.getOutputType()).getTypecheckInfo();

		if (this.postfix == null || this.postfix.length() < 1) {
			throw new TypeException(
					"Postfix needs a fixed String, but got null or \"\"");
		}

		List<RelationHeaderItem> outputType = new ArrayList<RelationHeaderItem>();
		for (RelationHeaderItem inputHeaderItem : inputType) {
			String identifier = inputHeaderItem.getIdentifier() + "_"
					+ this.postfix;
			String typeName = inputHeaderItem.getTypeName();
			RelationHeaderItem outputHeaderItem = new RelationHeaderItem(
					identifier, typeName);
			outputType.add(outputHeaderItem);
		}

		return MemoryTuple.createTypecheckInstance(outputType);
	}

}
