package unittests.mmdb.streamprocessing.objectnodes.maths;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.maths.Plus;

import org.junit.Test;

public class PlusTests {

	@Test
	public void testIntInt() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(5), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(7), new AttributeInt());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeInt.class, plus.getOutputType().getClass());
		assertEquals(12, ((AttributeInt) plus.getResult()).getValue());
	}

	@Test
	public void testIntReal() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(5), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeReal(7), new AttributeReal());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeReal.class, plus.getOutputType().getClass());
		assertEquals(12f, ((AttributeReal) plus.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testRealInt() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeReal(7), new AttributeReal());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(5), new AttributeInt());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeReal.class, plus.getOutputType().getClass());
		assertEquals(12f, ((AttributeReal) plus.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testRealReal() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeReal(7), new AttributeReal());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(new AttributeReal(
				-7), new AttributeReal());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeReal.class, plus.getOutputType().getClass());
		assertEquals(0f, ((AttributeReal) plus.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testGetResultStringString() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeString("a"), new AttributeString());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeString("b"), new AttributeString());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeString.class, plus.getOutputType().getClass());
		assertEquals("ab", ((AttributeString) plus.getResult()).getValue());
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 + undefined (UNDEFINED)
		Plus plus = new Plus(node1, node2);
		plus.typeCheck();
		assertEquals(AttributeInt.class, plus.getOutputType()
				.getClass());
		assertNull(plus.getResult());

		// undefined + 3 (UNDEFINED)
		plus = new Plus(node2, node1);
		plus.typeCheck();
		assertEquals(AttributeInt.class, plus.getOutputType()
				.getClass());
		assertNull(plus.getResult());

		// undefined + undefined (UNDEFINED)
		plus = new Plus(node2, node2);
		plus.typeCheck();
		assertEquals(AttributeInt.class, plus.getOutputType()
				.getClass());
		assertNull(plus.getResult());
	}

	@Test(expected = TypeException.class)
	public void testNonsummableTypes() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(9), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(new AttributeBool(
				true), new AttributeBool());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testStringSummable() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeString("a"), new AttributeString());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(3), new AttributeInt());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();
	}

}
