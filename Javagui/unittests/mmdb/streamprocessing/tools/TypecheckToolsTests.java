package unittests.mmdb.streamprocessing.tools;

import static org.junit.Assert.assertEquals;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.streamoperators.Filter;
import mmdb.streamprocessing.streamoperators.Rename;
import mmdb.streamprocessing.tools.TypecheckTools;

import org.junit.Test;

public class TypecheckToolsTests {

	@Test
	public void testFullCoverageDummy() {
		@SuppressWarnings("unused")
		TypecheckTools tools = new TypecheckTools() {
		};
	}

	@Test(expected = TypeException.class)
	public void testCheckMultipleNodeTypesFail() throws TypeException {
		Node node = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Class<? extends Node> caller = Filter.class;
		Class<? extends Node> expected1 = ParameterFunction.class;
		Class<? extends Node> expected2 = Rename.class;
		try {
			TypecheckTools.checkMultipleNodeTypes(node, caller, 1, expected1,
					expected2);
		} catch (TypeException e) {
			assertEquals(
					"Filter needs ParameterFunction or Rename as 1. input but got ObjectNode.",
					e.getLocalizedMessage());
			throw e;
		}
	}
}
