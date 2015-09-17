package unittests.mmdb.streamprocessing.tools;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.maths.Plus;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Filter;
import mmdb.streamprocessing.streamoperators.Rename;
import mmdb.streamprocessing.streamoperators.StreamOperator;
import mmdb.streamprocessing.tools.TypecheckTools;

import org.junit.Test;

public class TypecheckToolsTests {

	@Test
	public void testFullCoverageDummy() {
		@SuppressWarnings("unused")
		TypecheckTools tools = new TypecheckTools() {
		};
	}

	@Test
	@SuppressWarnings("unchecked")
	public void testCheckMultipleNodeTypesFail() throws TypeException {
		Node node = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Class<? extends Node> caller = Filter.class;
		Class<? extends Node> expected1 = ParameterFunction.class;
		Class<? extends Node> expected2 = Rename.class;
		boolean thrown = false;
		try {
			TypecheckTools.checkMultipleNodeTypes(node, caller, 1, expected1,
					expected2);
		} catch (TypeException e) {
			assertEquals(
					"Filter needs ParameterFunction or Rename as 1st input but got ObjectNode.",
					e.getLocalizedMessage());
			thrown = true;
		}
		assertTrue(thrown);
	}

	@Test(expected = TypeException.class)
	public void testCheckNodeTypeFail() throws TypeException {
		TypecheckTools.checkNodeType(new Feed(null), ObjectNode.class,
				Feed.class, 1);
	}

	@Test(expected = TypeException.class)
	public void testCheckSpecificNodeTypeFail() throws TypeException {
		TypecheckTools.checkSpecificNodeType(new Feed(null), Plus.class,
				Feed.class, 1);
	}

	@Test(expected = TypeException.class)
	public void testCheckOutputTypeHasIFaceFail() throws TypeException {
		// MemoryRelation is not a realistic example!
		TypecheckTools.checkOutputTypeHasIFace(ConstantNode.createConstantNode(
				new MemoryRelation(null), new MemoryRelation(null)),
				Comparable.class, Feed.class, 1);
	}

	@Test(expected = TypeException.class)
	public void testcheckFunctionOperatorTypeFail() throws TypeException {
		ParameterFunction function = new ParameterFunction(new Node[] {},
				ConstantNode.createConstantNode(new AttributeInt(7),
						new AttributeInt()));
		function.typeCheck();
		TypecheckTools.checkFunctionOperatorType(function,
				StreamOperator.class, Feed.class, 1);
	}

	@Test
	public void testOrdinalNumber() {
		assertEquals("1st", TypecheckTools.ordinalNumber(1));
		assertEquals("2nd", TypecheckTools.ordinalNumber(2));
		assertEquals("3rd", TypecheckTools.ordinalNumber(3));
		assertEquals("11th", TypecheckTools.ordinalNumber(11));
		assertEquals("21st", TypecheckTools.ordinalNumber(21));
		assertEquals("99th", TypecheckTools.ordinalNumber(99));
	}
}
