package unittests.mmdb.streamprocessing.objectnodes.condition;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.attributes.standard.AttributeBool;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.condition.Contains;

import org.junit.Test;

public class ContainsTests {

	@Test
	public void testContains() throws TypeException {
		// True
		Contains contains = getContains("Haus", "au");
		contains.typeCheck();
		assertEquals(AttributeBool.class, contains.getOutputType().getClass());
		assertEquals(true, ((AttributeBool) contains.getResult()).isValue());

		// False
		contains = getContains("Haus", "ou");
		contains.typeCheck();
		assertEquals(false, ((AttributeBool) contains.getResult()).isValue());
	}

	@Test
	public void testNullReaction() throws TypeException {
		ObjectNode string1Node = ConstantNode.createConstantNode(
				new AttributeString("aaa"), new AttributeString());
		ObjectNode string2Node = ConstantNode.createConstantNode(null,
				new AttributeString());

		// "aaa" contains undefined (UNDEFINED)
		Contains contains = new Contains(string1Node, string2Node);
		contains.typeCheck();
		assertEquals(AttributeBool.class, contains.getOutputType().getClass());
		assertNull(contains.getResult());

		// undefined contains "aaa" (UNDEFINED)
		contains = new Contains(string2Node, string1Node);
		contains.typeCheck();
		assertEquals(AttributeBool.class, contains.getOutputType().getClass());
		assertNull(contains.getResult());
	}

	@Test(expected = TypeException.class)
	public void testContainsFail() throws TypeException {
		AttributeInt integer = new AttributeInt(3);
		ObjectNode integerNode = ConstantNode.createConstantNode(integer,
				integer);

		AttributeString string = new AttributeString("abc");
		ObjectNode stringNode = ConstantNode.createConstantNode(string, string);
		Contains contains = new Contains(stringNode, integerNode);
		contains.typeCheck();
	}

	private Contains getContains(String a, String b) {
		AttributeString string1 = new AttributeString(a);
		ObjectNode stringNode1 = ConstantNode.createConstantNode(string1,
				string1);

		AttributeString string2 = new AttributeString(b);
		ObjectNode stringNode2 = ConstantNode.createConstantNode(string2,
				string2);

		return new Contains(stringNode1, stringNode2);
	}

}
