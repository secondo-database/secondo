package unittests.mmdb.streamprocessing.objectnodes.maths;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.maths.Minus;

import org.junit.Test;

public class MinusTests {

	@Test
	public void testIntInt() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(5), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(7), new AttributeInt());
		Minus minus = new Minus(attrNode1, attrNode2);
		minus.typeCheck();

		assertEquals(AttributeInt.class, minus.getOutputType().getClass());
		assertEquals(-2, ((AttributeInt) minus.getResult()).getValue());
	}

	@Test
	public void testIntReal() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(9), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeReal(6), new AttributeReal());
		Minus minus = new Minus(attrNode1, attrNode2);
		minus.typeCheck();

		assertEquals(AttributeReal.class, minus.getOutputType().getClass());
		assertEquals(3f, ((AttributeReal) minus.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testRealInt() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeReal(6), new AttributeReal());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(9), new AttributeInt());
		Minus minus = new Minus(attrNode1, attrNode2);
		minus.typeCheck();

		assertEquals(AttributeReal.class, minus.getOutputType().getClass());
		assertEquals(-3, ((AttributeReal) minus.getResult()).getValue(), 0f);
	}

	@Test
	public void testRealReal() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeReal(600), new AttributeReal());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeReal(6), new AttributeReal());
		Minus minus = new Minus(attrNode1, attrNode2);
		minus.typeCheck();

		assertEquals(AttributeReal.class, minus.getOutputType().getClass());
		assertEquals(594f, ((AttributeReal) minus.getResult()).getValue(),
				0.001f);
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 - undefined (UNDEFINED)
		Minus minus = new Minus(node1, node2);
		minus.typeCheck();
		assertEquals(AttributeInt.class, minus.getOutputType().getClass());
		assertNull(minus.getResult());

		// undefined - 3 (UNDEFINED)
		minus = new Minus(node2, node1);
		minus.typeCheck();
		assertEquals(AttributeInt.class, minus.getOutputType().getClass());
		assertNull(minus.getResult());

		// undefined - undefined (UNDEFINED)
		minus = new Minus(node2, node2);
		minus.typeCheck();
		assertEquals(AttributeInt.class, minus.getOutputType().getClass());
		assertNull(minus.getResult());
	}

	@Test(expected = TypeException.class)
	public void testNonsummableTypes() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(9), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeBool(true), new AttributeBool());
		Minus minus = new Minus(attrNode1, attrNode2);
		minus.typeCheck();
	}

}
