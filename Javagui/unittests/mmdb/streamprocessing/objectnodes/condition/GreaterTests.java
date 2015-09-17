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
import mmdb.streamprocessing.objectnodes.condition.Greater;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class GreaterTests {

	@Test
	public void testGreater() throws TypeException, MemoryException {
		// True
		Greater greater = getGreater(new AttributeInt(5), new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		greater.typeCheck();

		assertEquals(AttributeBool.class, greater.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) greater.getResult()).isValue());

		// False
		greater = getGreater(new AttributeInt(3), new AttributeInt(),
				new AttributeInt(9), new AttributeInt());
		greater.typeCheck();

		assertEquals(AttributeBool.class, greater.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) greater.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException, MemoryException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 > undefined (TRUE)
		Greater greater = new Greater(node1, node2);
		greater.typeCheck();
		assertEquals(AttributeBool.class, greater.getOutputType().getClass());
		assertTrue(((AttributeBool) greater.getResult()).isValue());

		// undefined > 3 (FALSE)
		greater = new Greater(node2, node1);
		greater.typeCheck();
		assertEquals(AttributeBool.class, greater.getOutputType().getClass());
		assertFalse(((AttributeBool) greater.getResult()).isValue());

		// undefined > undefined (FALSE)
		greater = new Greater(node2, node2);
		greater.typeCheck();
		assertEquals(AttributeBool.class, greater.getOutputType().getClass());
		assertFalse(((AttributeBool) greater.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testEqualsDifferentTypes() throws TypeException {
		Greater greater = getGreater(new AttributeReal(3), new AttributeReal(),
				new AttributeInt(9), new AttributeInt());
		greater.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		String query = "(query (> 7 4))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		result.typeCheck();
		assertEquals(new AttributeBool(true), result.getResult());
	}

	private Greater getGreater(MemoryObject a, MemoryObject typeA,
			MemoryObject b, MemoryObject typeB) {
		ObjectNode objNode1 = ConstantNode.createConstantNode(a, typeA);
		ObjectNode objNode2 = ConstantNode.createConstantNode(b, typeB);
		return new Greater(objNode1, objNode2);
	}

}
