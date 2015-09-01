package mmdb.streamprocessing.objectnodes.logic;

import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.tools.ParserTools;

public class Or extends LogicalOperator {

	public static Node fromNL(NestedListNode[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Or.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Or(node1, node2);
	}

	public Or(Node input1, Node input2) {
		super(input1, input2);
	}

	protected boolean calcResult(AttributeBool inputBool1,
			AttributeBool inputBool2) {
		if (inputBool1 == null || inputBool2 == null) {
			if (inputBool1 == null & inputBool2 == null) {
				return false;
			} else {
				return true;
			}
		}

		return inputBool1.isValue() || inputBool2.isValue();
	}

}
