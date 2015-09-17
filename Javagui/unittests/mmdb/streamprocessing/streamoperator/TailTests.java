package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Tail;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class TailTests {

	MemoryRelation rel = null;
	ObjectNode relNode = null;
	ObjectNode attrNode = null;
	Feed feed = null;
	Tail tail = null;

	@Before
	public void setUp() {
		rel = TestUtilRelation.getIntStringRelation(5, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		attrNode = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		feed = new Feed(relNode);
	}

	@Test
	public void testTailReduction() throws TypeException, MemoryException {
		tail = new Tail(feed, attrNode);
		tail.typeCheck();

		assertEquals(MemoryTuple.class, tail.getOutputType().getClass());
		tail.open();
		for (int i = 3; i <= 5; i++) {
			assertEquals(new AttributeInt(i),
					((MemoryTuple) tail.getNext()).getAttribute(1));
		}
		assertNull(tail.getNext());
		tail.close();
	}

	@Test
	public void testTailNegative() throws TypeException, MemoryException {
		attrNode = ConstantNode.createConstantNode(new AttributeInt(-19),
				new AttributeInt());
		tail = new Tail(feed, attrNode);
		tail.typeCheck();

		tail.open();
		assertNull(tail.getNext());
		assertNull(tail.getNext());
		tail.close();
	}

	@Test
	public void testTailNoReduction() throws TypeException, MemoryException {
		attrNode = ConstantNode.createConstantNode(new AttributeInt(7),
				new AttributeInt());
		tail = new Tail(feed, attrNode);
		tail.typeCheck();

		tail.open();
		for (int i = 1; i <= 5; i++) {
			assertEquals(new AttributeInt(i),
					((MemoryTuple) tail.getNext()).getAttribute(1));
		}
		assertNull(tail.getNext());
		tail.close();
	}

	@Test
	public void testTailEmpty() throws TypeException, MemoryException {
		rel = TestUtilRelation.getIntStringRelation(0, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
		tail = new Tail(feed, attrNode);
		tail.typeCheck();

		tail.open();
		assertNull(tail.getNext());
		tail.close();
	}

	@Test
	public void testNullObjectNodeReaction() throws TypeException,
			MemoryException {
		this.attrNode = ConstantNode.createConstantNode(null,
				new AttributeInt());
		tail = new Tail(feed, attrNode);
		tail.typeCheck();

		tail.open();
		assertNull(tail.getNext());
		assertNull(tail.getNext());
		tail.close();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (tail (feed REL) 2)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryRelation expected = TestUtilRelation.getIntStringRelation(5,
				false, false);
		expected.getTuples().remove(0);
		expected.getTuples().remove(0);
		expected.getTuples().remove(0);
		assertEquals(expected, mobject);
	}

}
