package mmdb.streamprocessing.tools;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import sj.lang.ListExpr;

/**
 * Tool class for checks during parsing (buildup of the operator tree).
 * 
 * @author Bjoern Clasen
 */
public class ParserTools {

	/**
	 * Checks if the given ListExpr contains the right amount of parameters for
	 * the caller.
	 * 
	 * @param listExpressions
	 *            the ListExpr representation of the parameters to count.
	 * @param target
	 *            the target number of parameters.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @throws ParsingException
	 *             if the ListExpr contains a wrong number of parameters.
	 */
	public static void checkListElemCount(ListExpr[] listExpressions,
			int target, Class<? extends Node> caller) throws ParsingException {
		if (listExpressions.length != target) {
			throw new ParsingException(
					"%s has wrong number of params! Expects %d but got %d.",
					caller.getSimpleName(), target, listExpressions.length);
		}
	}

}
