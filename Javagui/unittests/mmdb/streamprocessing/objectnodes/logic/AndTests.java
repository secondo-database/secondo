package unittests.mmdb.streamprocessing.objectnodes.logic;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.logic.And;

import org.junit.Test;

public class AndTests {

	@Test
	public void testAnd() throws TypeException {
		// True True
		And and = getAnd(true, true);
		and.typeCheck();
		assertEquals(AttributeBool.class, and.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) and.getResult()).isValue());

		// True False
		and = getAnd(true, false);
		and.typeCheck();
		assertEquals(false, ((AttributeBool) and.getResult()).isValue());

		// False True
		and = getAnd(false, true);
		and.typeCheck();
		assertEquals(false, ((AttributeBool) and.getResult()).isValue());

		// False False
		and = getAnd(false, false);
		and.typeCheck();
		assertEquals(false, ((AttributeBool) and.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeBool(
				true), new AttributeBool());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeBool());

		// true AND undefined (FALSE)
		And and = new And(node1, node2);
		and.typeCheck();
		assertEquals(AttributeBool.class, and.getOutputType().getClass());
		assertFalse(((AttributeBool) and.getResult()).isValue());

		// undefined AND true (FALSE)
		and = new And(node2, node1);
		and.typeCheck();
		assertEquals(AttributeBool.class, and.getOutputType().getClass());
		assertFalse(((AttributeBool) and.getResult()).isValue());

		// undefined AND undefined (FALSE)
		and = new And(node2, node2);
		and.typeCheck();
		assertEquals(AttributeBool.class, and.getOutputType().getClass());
		assertFalse(((AttributeBool) and.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testAndFail() throws TypeException {
		AttributeInt integer = new AttributeInt(3);
		ObjectNode integerNode = ConstantNode.createConstantNode(integer,
				integer);

		AttributeBool bool = new AttributeBool(true);
		ObjectNode boolNode = ConstantNode.createConstantNode(bool, bool);
		And and = new And(boolNode, integerNode);
		and.typeCheck();
	}

	private And getAnd(boolean a, boolean b) {
		AttributeBool bool1 = new AttributeBool(a);
		ObjectNode boolNode1 = ConstantNode.createConstantNode(bool1, bool1);

		AttributeBool bool2 = new AttributeBool(b);
		ObjectNode boolNode2 = ConstantNode.createConstantNode(bool2, bool2);

		return new And(boolNode1, boolNode2);
	}

}
