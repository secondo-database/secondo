package unittests.mmdb.streamprocessing;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.Nodes;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.ConstantNode;

import org.junit.Test;

public class NodesTests {

	@Test
	public void testGetTypeName() {
		Node node = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		String operatorName = Nodes.getTypeName(node.getClass());
		assertEquals("ObjectNode", operatorName);
	}

	@Test
	public void testGetTypeNameFail() {
		String nullString = Nodes.getTypeName(Integer.class);
		assertNull(nullString);
	}

	@Test
	public void testGetTypeClass() {
		Class<? extends Node> nodeClass = Nodes.getTypeClass("ObjectNode");
		assertEquals(ObjectNode.class, nodeClass);
	}

	@Test
	public void testGetTypeClassFail() {
		Class<?> nullClass = Nodes.getTypeClass("TEST");
		assertNull(nullClass);
	}

	@Test
	public void testFullCoverageDummy() {
		@SuppressWarnings("unused")
		Nodes nodes = new Nodes() {
		};
		Nodes.NodeType.values();
		Nodes.NodeType.valueOf("StreamOperator");
	}
}
