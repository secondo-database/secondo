package unittests.mmdb.streamprocessing.objectnodes.condition;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import gui.SecondoObject;

import java.util.ArrayList;

import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.condition.Equals;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class EqualsTests {

	@Test
	public void testEquals() throws TypeException, MemoryException {
		// True
		Equals equals = getEquals(new AttributeInt(3), new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		equals.typeCheck();

		assertEquals(AttributeBool.class, equals.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) equals.getResult()).isValue());

		// False
		equals = getEquals(new AttributeInt(9), new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		equals.typeCheck();

		assertEquals(AttributeBool.class, equals.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) equals.getResult()).isValue());
	}

	@Test
	public void testEqualsDifferentTypes() throws TypeException,
			MemoryException {
		Equals equals = getEquals(new AttributeReal(3), new AttributeReal(),
				new AttributeInt(9), new AttributeInt());
		equals.typeCheck();
		assertEquals(false, ((AttributeBool) equals.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException, MemoryException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 = undefined (FALSE)
		Equals equals = new Equals(node1, node2);
		equals.typeCheck();
		assertEquals(AttributeBool.class, equals.getOutputType().getClass());
		assertFalse(((AttributeBool) equals.getResult()).isValue());

		// undefined = 3 (FALSE)
		equals = new Equals(node2, node1);
		equals.typeCheck();
		assertEquals(AttributeBool.class, equals.getOutputType().getClass());
		assertFalse(((AttributeBool) equals.getResult()).isValue());

		// undefined = undefined (TRUE)
		equals = new Equals(node2, node2);
		equals.typeCheck();
		assertEquals(AttributeBool.class, equals.getOutputType().getClass());
		assertTrue(((AttributeBool) equals.getResult()).isValue());
	}

	@Test
	public void testQuery() throws Exception {
		String query = "(query (= 'abc' 'abc'))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		result.typeCheck();
		assertEquals(new AttributeBool(true), result.getResult());
	}

	private Equals getEquals(MemoryObject a, MemoryObject typeA,
			MemoryObject b, MemoryObject typeB) {
		ObjectNode objNode1 = ConstantNode.createConstantNode(a, typeA);
		ObjectNode objNode2 = ConstantNode.createConstantNode(b, typeB);
		return new Equals(objNode1, objNode2);
	}

}
