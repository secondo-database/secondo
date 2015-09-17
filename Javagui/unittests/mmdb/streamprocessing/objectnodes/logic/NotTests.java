package unittests.mmdb.streamprocessing.objectnodes.logic;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;

import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.logic.Not;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class NotTests {

	@Test
	public void testNot() throws TypeException, MemoryException {
		// False
		AttributeBool bool = new AttributeBool(true);
		ObjectNode boolNode = ConstantNode.createConstantNode(bool, bool);

		Not not = new Not(boolNode);
		not.typeCheck();

		assertEquals(AttributeBool.class, not.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) not.getResult()).isValue());

		// True
		bool = new AttributeBool(false);
		boolNode = ConstantNode.createConstantNode(bool, bool);

		not = new Not(boolNode);
		not.typeCheck();

		assertEquals(AttributeBool.class, not.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) not.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException, MemoryException {
		ObjectNode node = ConstantNode.createConstantNode(null,
				new AttributeBool());

		// NOT undefined (UNDEFINED)
		Not not = new Not(node);
		not.typeCheck();
		assertEquals(AttributeBool.class, not.getOutputType().getClass());
		assertNull(not.getResult());
	}

	@Test(expected = TypeException.class)
	public void testNotFail() throws TypeException {
		AttributeInt integer = new AttributeInt(3);
		ObjectNode integerNode = ConstantNode.createConstantNode(integer,
				integer);
		Not not = new Not(integerNode);
		not.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		String query = "(query (not FALSE))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		result.typeCheck();
		assertEquals(new AttributeBool(true), result.getResult());
	}

}
