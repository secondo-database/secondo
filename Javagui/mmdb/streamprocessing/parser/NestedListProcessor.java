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
import mmdb.error.streamprocessing.ParsingException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.nestedlist.Atom;
import mmdb.streamprocessing.parser.nestedlist.BooleanAtom;
import mmdb.streamprocessing.parser.nestedlist.IntegerAtom;
import mmdb.streamprocessing.parser.nestedlist.ListNode;
import mmdb.streamprocessing.parser.nestedlist.NestedListNode;
import mmdb.streamprocessing.parser.nestedlist.RealAtom;
import mmdb.streamprocessing.parser.nestedlist.StringAtom;
import mmdb.streamprocessing.parser.nestedlist.SymbolAtom;
import mmdb.streamprocessing.parser.tools.Environment;
import mmdb.streamprocessing.parser.tools.OperatorLookup;

public class NestedListProcessor {

	public static ObjectNode buildOperatorTree(String query,
			List<SecondoObject> existingObjects) throws ParsingException {
		parser parse = new parser(query);
		NestedListNode result = null;
		try {
			result = parse.parseToNestedList();
		} catch (Exception e) {
			throw new ParsingException(
					"Query String was not a valid nested list.");
		}

		if (!(result instanceof ListNode)) {
			throw new ParsingException("Query String was not in NL-format");
		}
		ListNode listResult = (ListNode) result;

		// Check for (query (...))
		NestedListNode firstChild = listResult.getChildren().get(0);
		if (!(firstChild instanceof SymbolAtom && ((SymbolAtom) firstChild)
				.getContent().equals("query"))) {
			throw new ParsingException(
					"Query String must start with \"(query\"");
		}
		if (listResult.getChildren().size() != 2) {
			throw new ParsingException("\"query\" must only have one child");
		}

		NestedListNode secondChild = listResult.getChildren().get(1);

		// Initialize Environment
		Environment environment = new Environment(existingObjects);

		// Build tree
		Node nodeResult = nlToNode(secondChild, environment);

		// Is node an ObjectNode?
		if (!(nodeResult instanceof ObjectNode)) {
			throw new ParsingException(
					"Expression not evaluable: Result is not an Object (Stream?)");
		}

		return (ObjectNode) nodeResult;
	}

	public static Node nlToNode(NestedListNode nlNode, Environment environment)
			throws ParsingException {
		if (nlNode instanceof Atom) {
			return atomToNode((Atom) nlNode, environment);
		} else {
			return listToNode((ListNode) nlNode, environment);
		}
	}

	public static String nlToIdentifier(NestedListNode nestedListNode)
			throws ParsingException {
		if (!(nestedListNode instanceof SymbolAtom)) {
			throw new ParsingException("Expected Identifier but got: "
					+ nestedListNode.printValueList());
		}
		return ((SymbolAtom) nestedListNode).getContent();
	}

	public static String[] nlToIdentifierArray(NestedListNode nestedListNode)
			throws ParsingException {
		if (!(nestedListNode instanceof ListNode)) {
			throw new ParsingException("Expected Identifier-List but got: "
					+ nestedListNode.printValueList());
		}
		ListNode listNode = (ListNode) nestedListNode;

		String[] identifiers = new String[listNode.getChildren().size()];
		for (int i = 0; i < listNode.getChildren().size(); i++) {
			if (!(listNode.getChildren().get(i) instanceof SymbolAtom)) {
				throw new ParsingException("Expected Identifier but got: "
						+ nestedListNode.printValueList());
			}
			identifiers[i] = ((SymbolAtom) listNode.getChildren().get(i))
					.getContent();
		}
		return identifiers;
	}

	public static String nlToFunctionEnvironmentName(
			NestedListNode nestedListNode) throws ParsingException {
		if (!(nestedListNode instanceof ListNode)) {
			throw new ParsingException(
					"Expected Function Environment definition but got: "
							+ nestedListNode.printValueList());
		}
		ListNode listNode = (ListNode) nestedListNode;
		if (!(listNode.getChildren().size() == 2
				&& listNode.getChildren().get(0) instanceof SymbolAtom && listNode
				.getChildren().get(1) instanceof SymbolAtom)) {
			throw new ParsingException(
					"Expected Function Environment definition but got: "
							+ nestedListNode.printValueList());
		}
		return ((SymbolAtom) listNode.getChildren().get(0)).getContent();
	}

	public static Map<String, String> nlToIdentifierPairs(
			NestedListNode nestedListNode) throws ParsingException {
		if (!(nestedListNode instanceof ListNode)) {
			throw new ParsingException("Expected Identifier-Pairs but got: "
					+ nestedListNode.printValueList());
		}
		ListNode listNode = (ListNode) nestedListNode;
		Map<String, String> retVal = new LinkedHashMap<>();
		for (NestedListNode nlNode : listNode.getChildren()) {
			if (!(nlNode instanceof ListNode)) {
				throw new ParsingException(
						"Expected Identifier-Pairs but got: "
								+ nestedListNode.printValueList());
			}
			ListNode listNode2 = (ListNode) nlNode;
			if (!(listNode2.getChildren().size() == 2
					&& listNode2.getChildren().get(0) instanceof SymbolAtom && listNode2
					.getChildren().get(1) instanceof SymbolAtom)) {
				throw new ParsingException(
						"Expected Identifier-Pairs but got: "
								+ nestedListNode.printValueList());
			}
			retVal.put(
					((SymbolAtom) listNode2.getChildren().get(0)).getContent(),
					((SymbolAtom) listNode2.getChildren().get(1)).getContent());
		}
		return retVal;
	}

	public static Map<String, Node> nlToIdentifierNodePairs(
			NestedListNode nestedListNode, Environment environment)
			throws ParsingException {
		if (!(nestedListNode instanceof ListNode)) {
			throw new ParsingException(
					"Expected Identifier-Node-Pairs but got: "
							+ nestedListNode.printValueList());
		}
		ListNode listNode = (ListNode) nestedListNode;
		Map<String, Node> retVal = new LinkedHashMap<>();
		for (NestedListNode nlNode : listNode.getChildren()) {
			if (!(nlNode instanceof ListNode)) {
				throw new ParsingException(
						"Expected Identifier-Node-Pair but got: "
								+ nestedListNode.printValueList());
			}
			ListNode listNode2 = (ListNode) nlNode;
			if (!(listNode2.getChildren().size() == 2
					&& listNode2.getChildren().get(0) instanceof SymbolAtom && listNode2
					.getChildren().get(1) instanceof ListNode)) {
				throw new ParsingException(
						"Expected Identifier-Node-Pair but got: "
								+ nestedListNode.printValueList());
			}
			retVal.put(
					((SymbolAtom) listNode2.getChildren().get(0)).getContent(),
					nlToNode(listNode2.getChildren().get(1), environment));
		}
		return retVal;
	}

	private static Node atomToNode(Atom atom, Environment environment)
			throws ParsingException {
		MemoryObject object = null, typeCheckInstance = null;
		if (atom instanceof SymbolAtom) {
			String symContent = ((SymbolAtom) atom).getContent();
			ObjectNode symObject = environment.getEnvironmentObject(symContent);
			if (symObject == null) {
				throw new ParsingException("Unknown Identifier: \""
						+ symContent + "\"");
			}
			return symObject;
		} else if (atom instanceof BooleanAtom) {
			boolean boolContent = ((BooleanAtom) atom).getContent();
			object = new AttributeBool(boolContent);
			typeCheckInstance = new AttributeBool();
		} else if (atom instanceof IntegerAtom) {
			int intContent = ((IntegerAtom) atom).getContent();
			object = new AttributeInt(intContent);
			typeCheckInstance = new AttributeInt();
		} else if (atom instanceof RealAtom) {
			float realContent = ((RealAtom) atom).getContent();
			object = new AttributeReal(realContent);
			typeCheckInstance = new AttributeReal();
		} else if (atom instanceof StringAtom) {
			String strContent = ((StringAtom) atom).getContent();
			object = new AttributeString(strContent);
			typeCheckInstance = new AttributeString();
		}
		return new ConstantNode(object, typeCheckInstance);
	}

	private static Node listToNode(ListNode listNode, Environment environment)
			throws ParsingException {
		// First child must be an operator
		NestedListNode firstChild = listNode.getChildren().get(0);
		if (!(firstChild instanceof SymbolAtom)) {
			throw new ParsingException(
					"First element in a list must be an operator but was: "
							+ firstChild.printValueList());
		}
		String symbolContent = ((SymbolAtom) firstChild).getContent();
		if (OperatorLookup.getInstance().lookup(symbolContent) == null) {
			throw new ParsingException("Operator not recognized: "
					+ symbolContent);
		}
		Class<? extends Node> operatorClass = OperatorLookup.getInstance()
				.lookup(((SymbolAtom) firstChild).getContent());

		return callFromNL(operatorClass, listNode, environment);
	}

	private static Node callFromNL(Class<? extends Node> operatorClass,
			ListNode listNode, Environment environment) throws ParsingException {
		Node resultNode = null;
		try {
			Method method = operatorClass.getMethod("fromNL",
					NestedListNode[].class, Environment.class);
			NestedListNode[] params = new NestedListNode[listNode.getChildren()
					.size() - 1];
			for (int i = 1; i < listNode.getChildren().size(); i++) {
				params[i - 1] = listNode.getChildren().get(i);
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
