package mmdb.streamprocessing.objectnodes.aggregation;

import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Orderable;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import sj.lang.ListExpr;

public class Min extends AggregationOperator {

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Min.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Min(node, identParam);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifier
	 *            operator's second parameter
	 */
	public Min(Node input, String identifier) {
		super(input, identifier);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		super.typeCheckForInterface(Orderable.class);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getResult() throws MemoryException {
		this.streamInput.open();
		MemoryTuple tuple = (MemoryTuple) this.streamInput.getNext();
		if (tuple == null) {
			return null;
		}
		Orderable result = (Orderable) tuple.getAttribute(this.attributeIndex);
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			Orderable next = (Orderable) tuple
					.getAttribute(this.attributeIndex);
			if (next.compareTo(result) < 0) {
				result = next;
			}
		}
		this.streamInput.close();
		return (MemoryAttribute) result;
	}

}
