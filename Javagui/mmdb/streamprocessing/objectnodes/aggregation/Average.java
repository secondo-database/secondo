package mmdb.streamprocessing.objectnodes.aggregation;

import java.util.List;

import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.features.Summable;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class Average extends AggregationOperator {

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Average.class);
		Node node = NestedListProcessor.nlToNode(params[0], environment);
		String identParam = NestedListProcessor.nlToIdentifier(params[1]);
		return new Average(node, identParam);
	}

	public Average(Node input, String identifier) {
		super(input, identifier);
	}

	@Override
	public void typeCheck() throws TypeException {
		super.typeCheckForInterface(Summable.class);
	}

	@Override
	public MemoryAttribute getResult() {
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

	@Override
	protected MemoryAttribute determineOutputType(
			List<RelationHeaderItem> inputOutputType) throws TypeException {
		return new AttributeReal();
	}

}
