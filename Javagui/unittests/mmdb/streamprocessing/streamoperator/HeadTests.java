package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Count;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Head;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class HeadTests {

	MemoryRelation rel = null;
	ObjectNode relNode = null;
	ObjectNode attrNode = null;
	Feed feed = null;
	Head head = null;
	Count count = null;

	@Before
	public void setUp() {
		rel = TestUtilRelation.getIntStringRelation(5, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		attrNode = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		feed = new Feed(relNode);
	}

	@Test
	public void testHead() throws TypeException, MemoryException {
		head = new Head(feed, attrNode);
		head.typeCheck();

		head.open();
		for (int i = 0; i < 3; i++) {
			MemoryTuple tuple = (MemoryTuple) head.getNext();
			assertNotNull(tuple);

			MemoryTuple relationTuple = rel.getTuples().get(i);
			assertEquals(relationTuple, tuple);
		}
		assertNull(head.getNext());
		this.testHeader();
		head.close();
	}

	@Test
	public void testHeadEmpty() throws TypeException, MemoryException {
		rel = TestUtilRelation.getIntStringRelation(0, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
		head = new Head(feed, attrNode);
		head.typeCheck();

		head.open();
		assertNull(head.getNext());
		this.testHeader();
		head.close();
	}

	@Test
	public void testHeadNegative() throws TypeException, MemoryException {
		attrNode = ConstantNode.createConstantNode(new AttributeInt(-7),
				new AttributeInt());
		head = new Head(feed, attrNode);
		head.typeCheck();

		head.open();
		assertNull(head.getNext());
		assertNull(head.getNext());
		this.testHeader();
		head.close();
	}

	private void testHeader() {
		assertNotNull(head.getOutputType());
		assertNotNull(feed.getOutputType());
		assertNotNull(rel.getHeader());
		assertEquals(feed.getOutputType(), head.getOutputType());
		assertEquals(rel.getHeader(),
				((MemoryTuple) head.getOutputType()).getTypecheckInfo());
	}

	@Test
	public void testHeadWithCount() throws TypeException, MemoryException {
		head = new Head(feed, attrNode);
		count = new Count(head);
		count.typeCheck();

		assertEquals(3, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testHeadWithCountMultiple() throws TypeException,
			MemoryException {
		head = new Head(feed, attrNode);
		count = new Count(head);
		count.typeCheck();

		assertEquals(3, ((AttributeInt) count.getResult()).getValue());
		assertEquals(3, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testNullObjectNodeReaction() throws TypeException,
			MemoryException {
		this.attrNode = ConstantNode.createConstantNode(null,
				new AttributeInt());
		head = new Head(feed, attrNode);
		head.typeCheck();

		head.open();
		assertNull(head.getNext());
		assertNull(head.getNext());
		head.close();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (head (feed REL) 2)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryRelation expected = TestUtilRelation.getIntStringRelation(2,
				false, false);
		assertEquals(expected, mobject);
	}

}
