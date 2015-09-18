package mmdb.streamprocessing;

import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.Environment;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import sj.lang.ListExpr;

/**
 * Class to store and manage all subclasses of Node.
 * 
 * @author Bjoern Clasen
 */
public abstract class Nodes {

	/**
	 * Enum for collecting all node types.
	 */
	public static enum NodeType {
		StreamOperator(StreamOperator.class), ObjectNode(ObjectNode.class), ParameterFunction(
				ParameterFunction.class);

		final Class<? extends Node> nodeInterface;

		NodeType(Class<? extends Node> nodeInterface) {
			this.nodeInterface = nodeInterface;
		}
	}

	/**
	 * Retrieves the type name for a given type class.
	 * 
	 * @param typeClass
	 *            the type class whose type name is being searched
	 * @return the type name if it is found, null otherwise
	 */
	public static String getTypeName(Class<?> typeClass) {
		for (NodeType type : NodeType.values()) {
			if (type.nodeInterface.isAssignableFrom(typeClass)) {
				return type.toString();
			}
		}
		return null;
	}

	/**
	 * Retrieves the type class for a given type name.
	 * 
	 * @param typeName
	 *            the type name whose type class is being searched
	 * @return the type class if it is found, null otherwise
	 */
	public static Class<? extends Node> getTypeClass(String typeName) {
		for (NodeType type : NodeType.values()) {
			if (type.toString().equals(typeName)) {
				return type.nodeInterface;
			}
		}
		return null;
	}

	/**
	 * Centralized Javadoc for fromNL() mehtod:<br>
	 * <br>
	 * Factory method for creating an Operator instance.<br>
	 * Is called via reflection and thus must always be of exactly the same
	 * signature.
	 * 
	 * @param params
	 *            the parameters for this newly instantiated operator as
	 *            ListExprs.
	 * @param environment
	 *            the Environment containing all yet known database objects.
	 * @return the newly instantiated operator as a Node.
	 * @throws ParsingException
	 *             if any error occured during operator instantiation.
	 */
	public static Node fromNL(ListExpr[] params, Environment environment)
			throws ParsingException {
		return null;
	}

}
