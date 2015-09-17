package unittests.mmdb.streamprocessing.objectnodes.maths;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;

import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.data.attributes.standard.AttributeText;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.maths.Plus;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class PlusTests {

	@Test
	public void testIntInt() throws TypeException, MemoryException {
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
	public void testIntReal() throws TypeException, MemoryException {
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
	public void testRealInt() throws TypeException, MemoryException {
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
	public void testRealReal() throws TypeException, MemoryException {
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
	public void testStringString() throws TypeException,
			MemoryException {
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
	public void testTextText() throws TypeException, MemoryException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeText("a"), new AttributeText());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeText("b"), new AttributeText());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();

		assertEquals(AttributeText.class, plus.getOutputType().getClass());
		assertEquals("ab", ((AttributeText) plus.getResult()).getValue());
	}

	@Test
	public void testPlusNullReaction() throws TypeException, MemoryException {
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
	public void testPlusIntBool() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeInt(9), new AttributeInt());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(new AttributeBool(
				true), new AttributeBool());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testPlusStringInt() throws TypeException {
		ObjectNode attrNode1 = ConstantNode.createConstantNode(
				new AttributeString("a"), new AttributeString());
		ObjectNode attrNode2 = ConstantNode.createConstantNode(
				new AttributeInt(3), new AttributeInt());
		Plus plus = new Plus(attrNode1, attrNode2);
		plus.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		String query = "(query (+ 3 4))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		result.typeCheck();
		assertEquals(new AttributeInt(7), result.getResult());
	}

}
