package unittests.mmdb.streamprocessing.objectnodes;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.Attr;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class AttrTests {

	@Test
	public void testAttr() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		feed.typeCheck();

		feed.open();
		ObjectNode tupNode = ConstantNode.createConstantNode(feed.getNext(),
				feed.getOutputType());
		Attr attr = new Attr(tupNode, "identifierInt");
		attr.typeCheck();

		assertEquals(1, ((AttributeInt) attr.getResult()).getValue());

		tupNode = ConstantNode.createConstantNode(feed.getNext(),
				feed.getOutputType());
		attr = new Attr(tupNode, "identifierString");
		attr.typeCheck();

		assertEquals(AttributeString.class, attr.getOutputType().getClass());
		assertEquals("string_2",
				((AttributeString) attr.getResult()).getValue());
		feed.close();
	}
	
	@Test
	public void testNullReaction() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		ObjectNode tupNode = ConstantNode.createConstantNode(null,
				MemoryTuple.createTypecheckInstance(rel.getHeader()));

		// attr(undefined, "Test") (UNDEFINED)
		Attr attr = new Attr(tupNode, "identifierInt");
		attr.typeCheck();
		assertEquals(AttributeInt.class, attr.getOutputType().getClass());
		assertNull(attr.getResult());
	}

	@Test(expected = TypeException.class)
	public void testAttrFail() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(1, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		feed.typeCheck();

		feed.open();
		ObjectNode tupNode = ConstantNode.createConstantNode(feed.getNext(),
				feed.getOutputType());
		Attr attr = new Attr(tupNode, "identifierBool");
		attr.typeCheck();
	}

}
