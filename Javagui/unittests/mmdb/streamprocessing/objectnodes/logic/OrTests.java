package unittests.mmdb.streamprocessing.objectnodes.logic;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.logic.Or;

import org.junit.Test;

public class OrTests {

	@Test
	public void testOr() throws TypeException {
		// True True
		Or or = getOr(true, true);
		or.typeCheck();
		assertEquals(AttributeBool.class, or.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) or.getResult()).isValue());

		// True False
		or = getOr(true, false);
		or.typeCheck();
		assertEquals(true, ((AttributeBool) or.getResult()).isValue());

		// False True
		or = getOr(false, true);
		or.typeCheck();
		assertEquals(true, ((AttributeBool) or.getResult()).isValue());

		// False False
		or = getOr(false, false);
		or.typeCheck();
		assertEquals(false, ((AttributeBool) or.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeBool(
				true), new AttributeBool());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeBool());

		// true OR undefined (TRUE)
		Or or = new Or(node1, node2);
		or.typeCheck();
		assertEquals(AttributeBool.class, or.getOutputType().getClass());
		assertTrue(((AttributeBool) or.getResult()).isValue());

		// undefined OR true (TRUE)
		or = new Or(node2, node1);
		or.typeCheck();
		assertEquals(AttributeBool.class, or.getOutputType().getClass());
		assertTrue(((AttributeBool) or.getResult()).isValue());

		// undefined OR undefined (FALSE)
		or = new Or(node2, node2);
		or.typeCheck();
		assertEquals(AttributeBool.class, or.getOutputType().getClass());
		assertFalse(((AttributeBool) or.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testOrFail() throws TypeException {
		AttributeInt integer = new AttributeInt(3);
		ObjectNode integerNode = ConstantNode.createConstantNode(integer,
				integer);

		AttributeBool bool = new AttributeBool(true);
		ObjectNode boolNode = ConstantNode.createConstantNode(bool, bool);
		Or or = new Or(boolNode, integerNode);
		or.typeCheck();
	}

	private Or getOr(boolean a, boolean b) {
		AttributeBool bool1 = new AttributeBool(a);
		ObjectNode boolNode1 = ConstantNode.createConstantNode(bool1, bool1);

		AttributeBool bool2 = new AttributeBool(b);
		ObjectNode boolNode2 = ConstantNode.createConstantNode(bool2, bool2);

		return new Or(boolNode1, boolNode2);
	}

}
