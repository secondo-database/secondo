package mmdb.streamprocessing.tools;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;

public class ParserTools {

	public static void checkListElemCount(NestedListNode[] list, int target,
			Class<? extends Node> caller) throws ParsingException {
		if (list.length != target) {
			throw new ParsingException(
					"%s has wrong number of params! Expects %d but got %d.",
					caller.getSimpleName(), target, list.length);
		}
	}

}
