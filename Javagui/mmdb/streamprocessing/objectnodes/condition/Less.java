package mmdb.streamprocessing.objectnodes.condition;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.tools.ParserTools;
import sj.lang.ListExpr;

/**
 * Implementation of comparison operator Less(<) resembling the core operator.<br>
 * Checks if first comparable is less than the second.
 * 
 * @author Bjoern Clasen
 */
public class Less extends ComparisonOperator {

	/**
	 * @see mmdb.streamprocessing.Nodes#fromNL(ListExpr[], Environment)
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		ParserTools.checkListElemCount(params, 2, Less.class);
		Node node1 = NestedListProcessor.nlToNode(params[0], environment);
		Node node2 = NestedListProcessor.nlToNode(params[1], environment);
		return new Less(node1, node2);
	}

	/**
	 * Constructor, called by fromNL(...)
	 * 
	 * @param input1
	 *            operator's first parameter
	 * @param input2
	 *            operator's second parameter
	 */
	public Less(Node input1, Node input2) {
		super(input1, input2);
	}

	/**
	 * {@inheritDoc}
	 */
	@SuppressWarnings("unchecked")
	@Override
	public MemoryObject getResult() throws MemoryException {
		Comparable<MemoryObject> object1 = (Comparable<MemoryObject>) this.objectInput1
				.getResult();
		MemoryObject object2 = this.objectInput2.getResult();

		if (object1 == null || object2 == null) {
			if (object2 != null) {
				return new AttributeBool(true);
			} else {
				return new AttributeBool(false);
			}
		}

		int intResult = object1.compareTo(object2);
		return new AttributeBool(intResult < 0);
	}

}
