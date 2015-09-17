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
import mmdb.streamprocessing.objectnodes.condition.EqualsLess;
import mmdb.streamprocessing.parser.NestedListProcessor;

import org.junit.Test;

public class EqualsLessTests {

	@Test
	public void testLess() throws TypeException, MemoryException {
		// True
		EqualsLess equalsLess = getEqualsLess(new AttributeInt(3),
				new AttributeInt(),
				new AttributeInt(9), new AttributeInt());
		equalsLess.typeCheck();

		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) equalsLess.getResult()).isValue());

		// True
		equalsLess = getEqualsLess(new AttributeInt(5), new AttributeInt(),
				new AttributeInt(5), new AttributeInt());
		equalsLess.typeCheck();

		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) equalsLess.getResult()).isValue());

		// False
		equalsLess = getEqualsLess(new AttributeInt(5), new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		equalsLess.typeCheck();

		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) equalsLess.getResult()).isValue());

	}

	@Test
	public void testNullReaction() throws TypeException, MemoryException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 <(=) undefined (FALSE)
		EqualsLess equalsLess = new EqualsLess(node1, node2);
		equalsLess.typeCheck();
		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertFalse(((AttributeBool) equalsLess.getResult()).isValue());

		// undefined <(=) 3 (TRUE)
		equalsLess = new EqualsLess(node2, node1);
		equalsLess.typeCheck();
		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertTrue(((AttributeBool) equalsLess.getResult()).isValue());

		// undefined (<)= undefined (TRUE)
		equalsLess = new EqualsLess(node2, node2);
		equalsLess.typeCheck();
		assertEquals(AttributeBool.class, equalsLess.getOutputType().getClass());
		assertTrue(((AttributeBool) equalsLess.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testEqualsDifferentTypes() throws TypeException {
		EqualsLess equalsLess = getEqualsLess(new AttributeReal(3),
				new AttributeReal(),
				new AttributeInt(9), new AttributeInt());
		equalsLess.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		String query = "(query (<= 8 9))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				new ArrayList<SecondoObject>());
		result.typeCheck();
		assertEquals(new AttributeBool(true), result.getResult());
	}

	private EqualsLess getEqualsLess(MemoryObject a, MemoryObject typeA,
			MemoryObject b, MemoryObject typeB) {
		ObjectNode objNode1 = ConstantNode.createConstantNode(a, typeA);
		ObjectNode objNode2 = ConstantNode.createConstantNode(b, typeB);
		return new EqualsLess(objNode1, objNode2);
	}

}
