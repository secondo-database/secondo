package mmdb.streamprocessing;

import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.StreamOperator;

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
	 * @return the type name if it is found, else null
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
	 * @return the type class if it is found, else null
	 */
	public static Class<? extends Node> getTypeClass(String typeName) {
		for (NodeType type : NodeType.values()) {
			if (type.toString().equals(typeName)) {
				return type.nodeInterface;
			}
		}
		return null;
	}

}
