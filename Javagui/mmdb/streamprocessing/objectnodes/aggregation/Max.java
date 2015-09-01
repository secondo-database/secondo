package mmdb.streamprocessing.objectnodes.aggregation;

import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.features.Orderable;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class Max extends AggregationOperator {

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Max.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Max(node, identParam);
	}

	public Max(Node input, String identifier) {
		super(input, identifier);
	}

	@Override
	public void typeCheck() throws TypeException {
		super.typeCheckForInterface(Orderable.class);
	}

	@Override
	public MemoryAttribute getResult() {
		this.streamInput.open();
		MemoryTuple tuple = (MemoryTuple) this.streamInput.getNext();
		if (tuple == null) {
			return null;
		}
		Orderable result = (Orderable) tuple.getAttribute(this.attributeIndex);
		while ((tuple = (MemoryTuple) this.streamInput.getNext()) != null) {
			Orderable next = (Orderable) tuple
					.getAttribute(this.attributeIndex);
			if (next.compareTo(result) > 0) {
				result = next;
			}
		}
		this.streamInput.close();
		return (MemoryAttribute) result;
	}

}