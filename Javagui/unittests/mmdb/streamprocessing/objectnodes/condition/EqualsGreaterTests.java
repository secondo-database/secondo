package unittests.mmdb.streamprocessing.objectnodes.condition;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import mmdb.data.MemoryObject;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.condition.EqualsGreater;

import org.junit.Test;

public class EqualsGreaterTests {

	@Test
	public void testGreater() throws TypeException {
		// True
		EqualsGreater equalsGreater = getEqualsGreater(new AttributeInt(5),
				new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		equalsGreater.typeCheck();

		assertEquals(AttributeBool.class, equalsGreater.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) equalsGreater.getResult()).isValue());

		// True
		equalsGreater = getEqualsGreater(new AttributeInt(9),
				new AttributeInt(), new AttributeInt(9), new AttributeInt());
		equalsGreater.typeCheck();

		assertEquals(AttributeBool.class, equalsGreater.getOutputType()
				.getClass());
		assertEquals(true,
				((AttributeBool) equalsGreater.getResult()).isValue());

		// False
		equalsGreater = getEqualsGreater(new AttributeInt(3), new AttributeInt(),
				new AttributeInt(9), new AttributeInt());
		equalsGreater.typeCheck();

		assertEquals(AttributeBool.class, equalsGreater.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) equalsGreater.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 >(=) undefined (TRUE)
		EqualsGreater equalsGreater = new EqualsGreater(node1, node2);
		equalsGreater.typeCheck();
		assertEquals(AttributeBool.class, equalsGreater.getOutputType()
				.getClass());
		assertTrue(((AttributeBool) equalsGreater.getResult()).isValue());

		// undefined >(=) 3 (FALSE)
		equalsGreater = new EqualsGreater(node2, node1);
		equalsGreater.typeCheck();
		assertEquals(AttributeBool.class, equalsGreater.getOutputType()
				.getClass());
		assertFalse(((AttributeBool) equalsGreater.getResult()).isValue());

		// undefined (>)= undefined (TRUE)
		equalsGreater = new EqualsGreater(node2, node2);
		equalsGreater.typeCheck();
		assertEquals(AttributeBool.class, equalsGreater.getOutputType()
				.getClass());
		assertTrue(((AttributeBool) equalsGreater.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testEqualsDifferentTypes() throws TypeException {
		EqualsGreater equalsGreater = getEqualsGreater(new AttributeReal(3),
				new AttributeReal(),
				new AttributeInt(9), new AttributeInt());
		equalsGreater.typeCheck();
	}

	private EqualsGreater getEqualsGreater(MemoryObject a, MemoryObject typeA,
			MemoryObject b, MemoryObject typeB) {
		ObjectNode objNode1 = ConstantNode.createConstantNode(a, typeA);
		ObjectNode objNode2 = ConstantNode.createConstantNode(b, typeB);
		return new EqualsGreater(objNode1, objNode2);
	}

}
