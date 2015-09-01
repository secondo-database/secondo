package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class Greater extends ComparisonOperator {

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Greater.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Greater(node1, node2);
	}

	public Greater(Node input1, Node input2) {
		super(input1, input2);
	}

	@SuppressWarnings("unchecked")
	@Override
	public MemoryObject getResult() {
		Comparable<MemoryObject> object1 = (Comparable<MemoryObject>) this.objectInput1
				.getResult();
		MemoryObject object2 = this.objectInput2.getResult();

		if (object1 == null || object2 == null) {
			if (object1 != null) {
				return new AttributeBool(true);
			} else {
				return new AttributeBool(false);
			}
		}

		int intResult = object1.compareTo(object2);
		return new AttributeBool(intResult > 0);
	}

}
