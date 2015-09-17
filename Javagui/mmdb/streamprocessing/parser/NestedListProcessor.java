package mmdb.streamprocessing.parser;

import gui.SecondoObject;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.attributes.standard.AttributeText;
import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import sj.lang.ListExpr;

/**
 * Class for any operations on nested lists during operator tree buildup.
 * 
 * @author Björn Clasen
 */
public class NestedListProcessor {

	/**
	 * The name of the fromNL method
	 */
	private static final String FROM_NL_METHODNAME = "fromNL";

	/**
	 * Completely builds up an operator tree based on the query in nested list
	 * format.<br>
	 * Checks if the beginning of the query is well formed (like
	 * <i>"(query ...)"</i>)
	 * 
	 * @param nlQuery
	 *            the query to build the operator tree for. It must be in valid
	 *            nested list format.
	 * @param existingObjects
	 *            a list of SecondoObjects representing the known objects during
	 *            the parsing phase.
	 * @return the root ObjectNode of the tree representing the query.
	 * @throws ParsingException
	 *             if any errors occured during operator tree buildup.
	 */
	public static ObjectNode buildOperatorTree(String nlQuery,
			List<SecondoObject> existingObjects) throws ParsingException {
		ListExpr result = new ListExpr();
		if (result.readFromString(nlQuery) != 0) {
			throw new ParsingException(
					"Query String was not a valid nested list.");
		}

		// Check for (query (...))
		ListExpr querySymbol = result.first();
		if (!(querySymbol.atomType() == 5 && querySymbol.symbolValue().equals("query"))) {
			throw new ParsingException(
					"NL-Query String must start with \"(query\"");
		}
		if (result.listLength() != 2) {
			throw new ParsingException("\"query\" must only have one child");
		}

		ListExpr content = result.second();

		// Initialize Environment
		Environment environment = new Environment(existingObjects);

		// Build tree
		Node nodeResult = nlToNode(content, environment);

		// Is node an ObjectNode?
		if (!(nodeResult instanceof ObjectNode)) {
			throw new ParsingException(
					"Expression not evaluable: Result is not an Object (Stream?)");
		}

		return (ObjectNode) nodeResult;
	}

	/**
	 * Creates a Node (and eventually recursively its Subnodes) representing the
	 * given ListExpr.
	 * 
	 * @param listExpr
	 *            the ListExpr to create a Node for.
	 * @param environment
	 *            the environment containing all yet known objects in the
	 *            current context.
	 * @return the newly created Node.
	 * @throws ParsingException
	 *             if any errors occur during Node creation.
	 */
	public static Node nlToNode(ListExpr listExpr, Environment environment)
			throws ParsingException {
		if (listExpr.isAtom()) {
			return atomToNode(listExpr, environment);
		} else {
			return listToNode(listExpr, environment);
		}
	}

	/**
	 * Converts a ListExpr to an identifier represented as a String.<br>
	 * ListExpr should be a symbol atom.
	 * 
	 * @param listExpr
	 *            the ListExpr representing an identifier.
	 * @return the identifier as a String.
	 * @throws ParsingException
	 *             if the given ListExpr is not a single identifier.
	 */
	public static String nlToIdentifier(ListExpr listExpr)
			throws ParsingException {
		if (listExpr.atomType() != 5) {
			throw new ParsingException("Expected Identifier but got: "
					+ listExpr.toString());
		}
		return listExpr.symbolValue();
	}

	/**
	 * Converts a ListExpr to an array of identifiers represented as a Strings.<br>
	 * ListExpr should be a list of symbol atoms.
	 * 
	 * @param listExpr
	 *            the ListExpr representing a list of identifiers.
	 * @return a String[] containing all identifiers in the ListExpr.
	 * @throws ParsingException
	 *             if the given ListExpr is not a list of identifiers (symbol
	 *             atoms).
	 */
	public static String[] nlToIdentifierArray(ListExpr listExpr)
			throws ParsingException {
		if (listExpr.isAtom()) {
			throw new ParsingException("Expected Identifier list but got: "
					+ listExpr.toString());
		}
		String[] identifiers = new String[listExpr.listLength()];
		int i = 0;
		while (!listExpr.isEmpty()) {
			ListExpr first = listExpr.first();
			listExpr = listExpr.rest();
			if (first.atomType() != 5) {
				throw new ParsingException("Expected Identifier but got: "
						+ first.toString());
			}
			identifiers[i] = first.symbolValue();
			i++;
		}
		return identifiers;
	}

	/**
	 * Retrieves the name of a FunctionEnvironment given as a ListExpr.<br>
	 * A FunctionEnvironment in nested list format is a list of two symbol
	 * atoms.<br>
	 * An example: <i>(tuple1 TUPLE)</i>.<br>
	 * Since the type of the FunctionEnvironment is automatically detected the
	 * second part of its nested list representation is ignored and just its
	 * name is extracted.
	 * 
	 * @param listExpr
	 *            the ListExpr representing a FunctionEnvironment.
	 * @return the name of the FunctionEnvironment.
	 * @throws ParsingException
	 *             if the ListExpr is not a valid FunctionEnvironment
	 *             representation.
	 */
	public static String nlToFunctionEnvironmentName(ListExpr listExpr)
			throws ParsingException {
		if (!(listExpr.listLength() == 2 && listExpr.first().atomType() == 5 && listExpr
				.second().atomType() == 5)) {
			throw new ParsingException(
					"Expected Function Environment definition but got: "
							+ listExpr.toString());
		}
		return listExpr.first().symbolValue();
	}

	/**
	 * Converts a ListExpr to pairs of identifiers.<br>
	 * Thus the ListExpr should be a list of lists of two symbol atoms.<br>
	 * Example: <code>((abc bcd)(cde def))</code>
	 * 
	 * @param listExpr
	 *            the ListExpr representing identifier pairs.
	 * @return a map of Strings containing all identifier pairs.
	 * @throws ParsingException
	 *             if the given ListExpr is not a list of identifier pars.
	 */
	public static Map<String, String> nlToIdentifierPairs(ListExpr listExpr)
			throws ParsingException {
		Map<String, String> retVal = new LinkedHashMap<String, String>();
		while (!listExpr.isEmpty()) {
			ListExpr first = listExpr.first();
			listExpr = listExpr.rest();
			if (!(first.listLength() == 2 && first.first().atomType() == 5 && first
					.second().atomType() == 5)) {
				throw new ParsingException(
						"Expected Identifier-Pairs but got: "
								+ first.toString());
			}
			retVal.put(first.first().symbolValue(), first.second()
					.symbolValue());
		}
		return retVal;
	}

	/**
	 * Converts a ListExpr to <i>Identifier:Node</i> pairs.<br>
	 * Thus the ListExpr should be a list of lists of a symbol atom and another
	 * list. Example: <code>((abc (...))(cde (...)))</code>
	 * 
	 * @param listExpr
	 *            the ListExpr representing pairs of identifier and Node.
	 * @param environment
	 *            the environment containing all yet known objects in the
	 *            current context.
	 * @return a String to Node map containing all pairs contained in the
	 *         ListExpr.
	 * @throws ParsingException
	 *             if the ListExpr is not a list of identifier:Node pairs
	 */
	public static Map<String, Node> nlToIdentifierNodePairs(ListExpr listExpr,
			Environment environment) throws ParsingException {
		Map<String, Node> retVal = new LinkedHashMap<String, Node>();
		while (!listExpr.isEmpty()) {
			ListExpr first = listExpr.first();
			listExpr = listExpr.rest();
			if (!(first.listLength() == 2 && first.first().atomType() == 5 && !first
					.second().isAtom())) {
				throw new ParsingException(
						"Expected Identifier-Node-Pair but got: "
								+ first.toString());
			}
			retVal.put(first.first().symbolValue(),
					nlToNode(first.second(), environment));
		}
		return retVal;
	}

	/**
	 * Converts a ListExpr containing an atom to a corresponding ObjectNode. If
	 * the ListExpr is a symbol atom it is looked up in the environment.
	 * 
	 * @param listExpr
	 *            the ListExpr containing an atom.
	 * @param environment
	 *            the Environment to look up objects in given as symbol atoms.
	 * @return the ObjectNode representing the ListExpr's value.
	 * @throws ParsingException
	 *             if the ListExpr was a symbol atom that was not found in the
	 *             environment.
	 */
	private static Node atomToNode(ListExpr listExpr, Environment environment)
			throws ParsingException {
		MemoryObject object = null, typeCheckInstance = null;
		if (listExpr.atomType() == 5) {
			String symContent = listExpr.symbolValue();
			ObjectNode symObject = environment.getEnvironmentObject(symContent);
			if (symObject == null) {
				throw new ParsingException("Unknown Identifier: \""
						+ symContent + "\"");
			}
			return symObject;
		} else if (listExpr.atomType() == 3) {
			boolean boolContent = listExpr.boolValue();
			object = new AttributeBool(boolContent);
			typeCheckInstance = new AttributeBool();
		} else if (listExpr.atomType() == 1) {
			int intContent = listExpr.intValue();
			object = new AttributeInt(intContent);
			typeCheckInstance = new AttributeInt();
		} else if (listExpr.atomType() == 2) {
			float realContent = (float) listExpr.realValue();
			object = new AttributeReal(realContent);
			typeCheckInstance = new AttributeReal();
		} else if (listExpr.atomType() == 4) {
			String strContent = listExpr.stringValue();
			object = new AttributeString(strContent);
			typeCheckInstance = new AttributeString();
		} else if (listExpr.atomType() == 6) {
			String textContent = listExpr.textValue();
			object = new AttributeText(textContent);
			typeCheckInstance = new AttributeText();
		}
		return ConstantNode.createConstantNode(object, typeCheckInstance);
	}

	/**
	 * Converts a ListExpr containing a list to a Node.<br>
	 * If this ListExpr is to represent a Node its first child should be an
	 * operator.<br>
	 * This operator is being instantiated here.
	 * 
	 * @param listExpr
	 *            the ListExpr representing a Node.
	 * @param environment
	 *            the Environment containing all yet known database objects.
	 * @return the newly created Node representing the ListExpr.
	 * @throws ParsingException
	 *             if the ListExpr could not be transformed to a Node.
	 */
	private static Node listToNode(ListExpr listExpr, Environment environment)
			throws ParsingException {
		// First child must be an operator
		if (listExpr.first().atomType() != 5) {
			throw new ParsingException(
					"First element in a list must be an operator but was: "
							+ listExpr.first().toString());
		}
		String symbolContent = listExpr.first().symbolValue();
		if (OperatorLookup.lookup(symbolContent) == null) {
			throw new ParsingException("Operator not recognized: "
					+ symbolContent);
		}
		Class<? extends Node> operatorClass = OperatorLookup.lookup(listExpr
				.first().symbolValue());

		return callMethodFromNL(operatorClass, listExpr, environment);
	}

	/**
	 * Calls the fromNL(...) factory method on the given operator class using
	 * reflection.<br>
	 * Correctly forwards any ParsingException that may have been thrown in
	 * fromNL(...).
	 * 
	 * @param operatorClass
	 *            the class to instantiate.
	 * @param listExpr
	 *            the ListExpr containing the operator's name and all of its
	 *            parameters.
	 * @param environment
	 *            the Environment containing all yet known database objects.
	 * @return the newly created Node of the type of operatorClass.
	 * @throws ParsingException
	 *             if any Exception occured during call of the fromNL() factory
	 *             method.
	 */
	private static Node callMethodFromNL(Class<? extends Node> operatorClass,
			ListExpr listExpr, Environment environment) throws ParsingException {
		Node resultNode = null;
		try {
			Method method = operatorClass.getMethod(FROM_NL_METHODNAME,
					ListExpr[].class,
					Environment.class);
			ListExpr[] params = new ListExpr[listExpr.listLength() - 1];

			// First element is operator-symbol, just the rest are parameters
			listExpr = listExpr.rest();
			int i = 0;
			while (!listExpr.isEmpty()) {
				params[i] = listExpr.first();
				listExpr = listExpr.rest();
				i++;
			}

			resultNode = (Node) method.invoke(null, (Object[]) params,
					environment);
		} catch (InvocationTargetException ite) {
			if (ite.getCause() instanceof ParsingException) {
				throw (ParsingException) ite.getCause();
			} else {
				throw new ParsingException(ite.getTargetException()
						.getMessage());
			}
		} catch (Exception e) {
			throw new ParsingException(
					String.format(
							"Operator %s's fromNL-Function is missing or has a faulty implementation: ",
							operatorClass.getSimpleName()));
		}
		return resultNode;
	}

}
