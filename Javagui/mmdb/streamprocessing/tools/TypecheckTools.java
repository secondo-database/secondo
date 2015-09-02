package mmdb.streamprocessing.tools;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryObjects;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.functionoperators.ParameterFunction;

public abstract class TypecheckTools {

	public static void checkNodeType(Node node,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (!(expectedType.isInstance(node))) {
			throw new TypeException("%s needs %s as %s. input but got %s.",
					caller.getSimpleName(), Nodes.getTypeName(expectedType),
					inputNumber, Nodes.getTypeName(node.getClass()));
		}
	}

	public static void checkSpecificNodeType(Node node,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (expectedType != node.getClass()) {
			throw new TypeException("%s needs %s as %s. input but got %s.",
					caller.getSimpleName(), expectedType.getSimpleName(),
					inputNumber, node.getClass().getSimpleName());
		}
	}

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

			throw new TypeException("%s needs %s as %s. input but got %s.",
					caller.getSimpleName(), expectedList, inputNumber,
					Nodes.getTypeName(node.getClass()));
		}

	}

	public static void checkOutputType(Node node,
			Class<? extends MemoryObject> expectedOutput,
			Class<? extends Node> caller, int inputNumber) throws TypeException {

		if (!expectedOutput.isInstance(node.getOutputType())) {
			throw new TypeException(
					"%s's %s. input needs to provide %s, but provides %s.",
					caller.getSimpleName(), inputNumber,
					MemoryObjects.getTypeName(expectedOutput),
					MemoryObjects.getTypeName(node.getOutputType().getClass()));
		}

	}

	public static void checkOutputTypeHasIFace(Node node, Class<?> iFace,
			Class<? extends Node> caller, int inputNumber) throws TypeException {
		if (!iFace.isAssignableFrom(node.getOutputType().getClass())) {
			throw new TypeException(
					"%s's %s. input needs to provide an object implementing %s, but provides %s.",
					caller.getSimpleName(), inputNumber, iFace.getSimpleName(),
					MemoryObjects.getTypeName(node.getOutputType().getClass()));
		}
	}

	public static void checkFunctionOperatorType(ParameterFunction function,
			Class<? extends Node> expectedType, Class<? extends Node> caller,
			int inputNumber) throws TypeException {

		if (!(expectedType.isAssignableFrom(function.getOperatorType()))) {
			throw new TypeException(
					"%s's %s (%s. input) needs to provide %s but provides %s.",
					caller.getSimpleName(), Nodes.NodeType.ParameterFunction,
					inputNumber, Nodes.getTypeName(expectedType),
					Nodes.getTypeName(function.getOperatorType()));
		}

	}

	public static void checkFunctionOutputType(ParameterFunction function,
			Class<? extends MemoryObject> expectedType,
			Class<? extends Node> caller, int inputNumber) throws TypeException {

		if (!(expectedType.isInstance(function.getOutputType()))) {
			throw new TypeException(
					"%s's %s (%s. input) needs to provide a Node providing %s but provides a Node providing %s.",
					caller.getSimpleName(), Nodes.NodeType.ParameterFunction,
					inputNumber, MemoryObjects.getTypeName(expectedType),
					MemoryObjects.getTypeName(function.getOutputType()
							.getClass()));
		}

	}

}
