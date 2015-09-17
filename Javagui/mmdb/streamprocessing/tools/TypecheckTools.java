package mmdb.streamprocessing.tools;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryObjects;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.functionoperators.ParameterFunction;

/**
 * Tool class for general typecheck operations.
 * 
 * @author Björn Clasen
 */
public abstract class TypecheckTools {

	/**
	 * Checks if the given node is of the expected type (or any subtype of it).
	 * 
	 * @param node
	 *            the node whose type is to be checked.
	 * @param expectedType
	 *            the expected type for the node.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the node is of another type than the expected type.
	 */
	public static void checkNodeType(Node node,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (!(expectedType.isInstance(node))) {
			throw new TypeException("%s needs %s as %s input but got %s.",
					caller.getSimpleName(), Nodes.getTypeName(expectedType),
					ordinalNumber(inputNumber), Nodes.getTypeName(node
							.getClass()));
		}
	}

	/**
	 * Checks if the given Node is exactly of the expected type.<br>
	 * Subtypes do not count.
	 * 
	 * @param node
	 *            the node whose type is to be checked.
	 * @param expectedType
	 *            the exact expected type for the node.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the node is of any other type than the expected type.
	 */
	public static void checkSpecificNodeType(Node node,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (expectedType != node.getClass()) {
			throw new TypeException("%s needs %s as %s input but got %s.",
					caller.getSimpleName(), expectedType.getSimpleName(),
					ordinalNumber(inputNumber), node.getClass().getSimpleName());
		}
	}

	/**
	 * Checks if the given Node is of any of the expected types (or any subtype
	 * of them).
	 * 
	 * @param node
	 *            the node whose type is to be checked.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @param expectedTypes
	 *            a list of the types the Node may have.
	 * @throws TypeException
	 *             if the Node is neither an instance of the expected types nor
	 *             their subtypes.
	 */
	public static void checkMultipleNodeTypes(Node node,
			Class<? extends Node> caller, int inputNumber,
			Class<? extends Node>... expectedTypes) throws TypeException {

		boolean typeMatched = false;
		for (Class<? extends Node> expectedType : expectedTypes) {
			if (expectedType.isInstance(node)) {
				typeMatched = true;
			}
		}

		if (!typeMatched) {
			String expectedList = "";
			for (Class<? extends Node> expectedType : expectedTypes) {
				expectedList += expectedType.getSimpleName();
				expectedList += " or ";
			}
			expectedList = expectedList.substring(0, expectedList.length() - 4);

			throw new TypeException("%s needs %s as %s input but got %s.",
					caller.getSimpleName(), expectedList,
					ordinalNumber(inputNumber),
					Nodes.getTypeName(node.getClass()));
		}

	}

	/**
	 * Checks the output type of the given Node.
	 * 
	 * @param node
	 *            the node whose output type is to be checked.
	 * @param expectedOutput
	 *            the output type that the Node should have.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the output type of the Node differs from the expected
	 *             output type.
	 */
	public static void checkOutputType(Node node,
			Class<? extends MemoryObject> expectedOutput,
			Class<? extends Node> caller, int inputNumber) throws TypeException {

		if (!expectedOutput.isInstance(node.getOutputType())) {
			throw new TypeException(
					"%s's %s input needs to provide %s, but provides %s.",
					caller.getSimpleName(), ordinalNumber(inputNumber),
					MemoryObjects.getTypeName(expectedOutput),
					MemoryObjects.getTypeName(node.getOutputType().getClass()));
		}

	}

	/**
	 * Checks if the output type of the given Node fulfills the given interface.
	 * 
	 * @param node
	 *            the node whose output type is to be checked.
	 * @param iFace
	 *            the interface the output type should fulfill.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the output type of the given node does not fulfill the
	 *             given interface.
	 */
	public static void checkOutputTypeHasIFace(Node node, Class<?> iFace,
			Class<? extends Node> caller, int inputNumber) throws TypeException {
		if (!iFace.isAssignableFrom(node.getOutputType().getClass())) {
			throw new TypeException(
					"%s's %s input needs to provide an object implementing %s, but provides %s.",
					caller.getSimpleName(), ordinalNumber(inputNumber), iFace
							.getSimpleName(),
					MemoryObjects.getTypeName(node.getOutputType().getClass()));
		}
	}

	/**
	 * Checks the type of Operator/Node that the given ParameterFunction
	 * returns. This can usually be an ObjectNode or a StreamOperator.
	 * 
	 * @param function
	 *            the ParameterFunction whose output Operator should be of the
	 *            expected type.
	 * @param expectedType
	 *            the type the Operator should be of.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the ParameterFunction returns a different type of
	 *             operator.
	 */
	public static void checkFunctionOperatorType(ParameterFunction function,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (!(expectedType.isAssignableFrom(function.getOperatorType()))) {
			throw new TypeException(
					"%s's %s (%s input) needs to provide %s but provides %s.",
					caller.getSimpleName(), Nodes.NodeType.ParameterFunction,
					ordinalNumber(inputNumber),
					Nodes.getTypeName(expectedType),
					Nodes.getTypeName(function.getOperatorType()));
		}

	}

	/**
	 * Checks the output type of the Operator/Node that the given
	 * ParameterFunction returns.
	 * 
	 * @param function
	 *            the ParameterFunction which puts out the Operator whose output
	 *            type is to be checked.
	 * @param expectedType
	 *            the output type the Operator should have.
	 * @param caller
	 *            the class of the calling operator (for exception message
	 *            purposes).
	 * @param inputNumber
	 *            the position of the Node in the caller's parameters (for
	 *            exception message purposes).
	 * @throws TypeException
	 *             if the Operator the ParameterFunction puts out has a
	 *             different output type.
	 */
	public static void checkFunctionOutputType(ParameterFunction function,
			Class<? extends MemoryObject> expectedType,
			Class<? extends Node> caller, int inputNumber) throws TypeException {

		if (!(expectedType.isInstance(function.getOutputType()))) {
			throw new TypeException(
					"%s's %s (%s input) needs to provide a Node providing %s but provides a Node providing %s.",
					caller.getSimpleName(), Nodes.NodeType.ParameterFunction,
					ordinalNumber(inputNumber), MemoryObjects
							.getTypeName(expectedType),
					MemoryObjects.getTypeName(function.getOutputType()
							.getClass()));
		}

	}

	/**
	 * A simple method for exception message purposes that formats an integer as
	 * an ordinal number in String format.
	 * 
	 * @param inputNumber
	 *            the integer to make an ordinal number of.
	 * @return the ordinal number as a String.
	 */
	public static String ordinalNumber(int inputNumber) {
		String numAsString = Integer.toString(inputNumber);
		if (numAsString.length() > 1
				&& numAsString.charAt(numAsString.length() - 2) == '1') {
			return numAsString + "th";
		}
		if (numAsString.charAt(numAsString.length() - 1) == '1') {
			return numAsString + "st";
		}
		if (numAsString.charAt(numAsString.length() - 1) == '2') {
			return numAsString + "nd";
		}
		if (numAsString.charAt(numAsString.length() - 1) == '3') {
			return numAsString + "rd";
		}
		return numAsString + "th";
	}

}
