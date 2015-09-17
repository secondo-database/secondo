package mmdb.streamprocessing.objectnodes;

import mmdb.data.MemoryRelation;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.ParserTools;
import mmdb.streamprocessing.tools.TypecheckTools;
import sj.lang.ListExpr;

/**
 * Operator Count resembling the core operator.<br>
 * Counts streams and relations.
 * 
 * @author Björn Clasen
 */
public class Count implements ObjectNode {

	/**
	 * The operator's parameter Node.
	 */
	private Node input;

	/**
	 * The operator's output type.
	 */
	private AttributeInt outputType;

	/**
	 * An enum stating the possible input variants of this operator.
	 */
	private enum InputVariant {
		STREAM, RELATION
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
		ParserTools.checkListElemCount(params, 1, Count.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		return new Count(node1);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's parameter
	 */
	public Count(Node input) {
		this.input = input;
	}

	/**
	 * {@inheritDoc}
	 */
	@SuppressWarnings("unchecked")
	@Override
	public void typeCheck() throws TypeException {
		this.input.typeCheck();

		// Is input a StreamOperator or an ObjectNode?
		TypecheckTools.checkMultipleNodeTypes(this.input, this.getClass(), 1,
				StreamOperator.class, ObjectNode.class);

		this.inputVariant = determineInputVariant();

		this.outputType = new AttributeInt();
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getResult() throws MemoryException {
		switch (this.inputVariant) {
		case STREAM:
			return new AttributeInt(streamCalculation());
		case RELATION:
			int relResult = relationCalculation();
			if (relResult < 0) {
				return null;
			} else {
				return new AttributeInt(relResult);
			}
		default:
			return null;
		}
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public AttributeInt getOutputType() {
		return this.outputType;
	}

	/**
	 * Calculates Count's result based on a MemoryRelation as input.
	 * 
	 * @return the number of lines the MemoryRelation parameter has.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private int relationCalculation() throws MemoryException {
		ObjectNode objectInput = (ObjectNode) this.input;
		MemoryRelation relation = (MemoryRelation) objectInput.getResult();
		if (relation == null) {
			return -1;
		}

		int count = relation.getTuples().size();
		return count;
	}

	/**
	 * Calculates Count's result based on a StreamOperator as input.
	 * 
	 * @return the number elements the StreamOperator parameter provides.
	 * @throws MemoryException
	 *             if during calculation memory tended to run out. Execution is
	 *             then canceled and intermediate results are discarded.
	 */
	private int streamCalculation() throws MemoryException {
		StreamOperator streamInput = (StreamOperator) this.input;
		int count = 0;

		streamInput.open();
		while (streamInput.getNext() != null) {
			count++;
		}
		streamInput.close();
		return count;
	}

	/**
	 * Determines the present InputVariant.
	 * 
	 * @return detected InputVariant.
	 * @throws TypeException
	 *             if no valid InputVariant could be detected.
	 */
	private InputVariant determineInputVariant() throws TypeException {
		if (this.input instanceof StreamOperator) {
			return InputVariant.STREAM;
		} else {
			// Does ObjectNode provide a MemoryRelation?
			TypecheckTools.checkOutputType(this.input, MemoryRelation.class,
					this.getClass(), 1);
			return InputVariant.RELATION;
		}
	}

}
