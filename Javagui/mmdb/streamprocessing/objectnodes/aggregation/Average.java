package mmdb.streamprocessing.objectnodes.aggregation;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.features.Summable;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import sj.lang.ListExpr;

public class Average extends AggregationOperator {

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Average.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Average(node, identParam);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input
	 *            operator's first parameter
	 * @param identifier
	 *            operator's second parameter
	 */
	public Average(Node input, String identifier) {
		super(input, identifier);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void typeCheck() throws TypeException {
		super.typeCheckForInterface(Summable.class);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public MemoryAttribute getResult() throws MemoryException {
		float sum = 0;
		int count = 0;
		this.streamInput.open();
		MemoryTuple tuple;
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			sum += ((Summable) tuple.getAttribute(this.attributeIndex))
					.getValueAsReal();
			count++;
		}
		this.streamInput.close();
		if (count == 0) {
			return null;
		}
		float average = sum / count;
		return new AttributeReal(average);
	}

	/**
	 * Averages OutputType is always of type real.
	 */
	@Override
	protected MemoryAttribute determineOutputType(
			List<RelationHeaderItem> inputOutputType) throws TypeException {
		return new AttributeReal();
	}

}
