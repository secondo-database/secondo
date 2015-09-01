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
import mmdb.streamprocessing.objectnodes.condition.Less;

import org.junit.Test;

public class LessTests {

	@Test
	public void testLess() throws TypeException {
		// True
		Less less = getLess(new AttributeInt(3), new AttributeInt(),
				new AttributeInt(9), new AttributeInt());
		less.typeCheck();

		assertEquals(AttributeBool.class, less.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) less.getResult()).isValue());

		// False
		less = getLess(new AttributeInt(5), new AttributeInt(),
				new AttributeInt(3), new AttributeInt());
		less.typeCheck();

		assertEquals(AttributeBool.class, less.getOutputType().getClass());
		assertEquals(false, ((AttributeBool) less.getResult()).isValue());

	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode node1 = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		ObjectNode node2 = ConstantNode.createConstantNode(null,
				new AttributeInt());

		// 3 < undefined (FALSE)
		Less less = new Less(node1, node2);
		less.typeCheck();
		assertEquals(AttributeBool.class, less.getOutputType().getClass());
		assertFalse(((AttributeBool) less.getResult()).isValue());

		// undefined < 3 (TRUE)
		less = new Less(node2, node1);
		less.typeCheck();
		assertEquals(AttributeBool.class, less.getOutputType().getClass());
		assertTrue(((AttributeBool) less.getResult()).isValue());

		// undefined < undefined (FALSE)
		less = new Less(node2, node2);
		less.typeCheck();
		assertEquals(AttributeBool.class, less.getOutputType().getClass());
		assertFalse(((AttributeBool) less.getResult()).isValue());
	}

	@Test(expected = TypeException.class)
	public void testEqualsDifferentTypes() throws TypeException {
		Less less = getLess(new AttributeReal(3), new AttributeReal(),
				new AttributeInt(9), new AttributeInt());
		less.typeCheck();
	}

	private Less getLess(MemoryObject a, MemoryObject typeA,
			MemoryObject b, MemoryObject typeB) {
		ObjectNode objNode1 = ConstantNode.createConstantNode(a, typeA);
		ObjectNode objNode2 = ConstantNode.createConstantNode(b, typeB);
		return new Less(objNode1, objNode2);
	}

}
