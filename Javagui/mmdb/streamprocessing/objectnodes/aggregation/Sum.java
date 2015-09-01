package mmdb.streamprocessing.objectnodes.aggregation;

import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Summable;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class Sum extends AggregationOperator {

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Sum.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Sum(node, identParam);
	}

	public Sum(Node input, String identifier) {
		super(input, identifier);
	}

	@Override
	public void typeCheck() throws TypeException {
		super.typeCheckForInterface(Summable.class);
	}

	@Override
	public MemoryAttribute getResult() {
		this.streamInput.open();
		MemoryTuple tuple = (MemoryTuple) this.streamInput.getNext();
		if (tuple == null) {
			// ANSI 92: NULL
			// SECONDO: 0
			return null;
		}
		Summable result = (Summable) tuple.getAttribute(this.attributeIndex);
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			result = result.sum((Summable) tuple
					.getAttribute(this.attributeIndex));
		}
		this.streamInput.close();
		return (MemoryAttribute) result;
	}

}
