package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class FeedTests {

	@Test
	public void testFeed() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode ObjectNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(ObjectNode);
		feed.typeCheck();
		feed.open();
		for (int i = 0; i < 5; i++) {
			assertNotNull(feed.getNext());
		}
		assertNull(feed.getNext());
		assertNull(feed.getNext());
		feed.close();
	}

	@Test
	public void testFeedEmpty() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, true,
				false);
		ObjectNode ObjectNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(ObjectNode);
		feed.typeCheck();
		feed.open();
		assertNull(feed.getNext());
		assertNull(feed.getNext());
		feed.close();
	}
	
	@Test
	public void testFeedNull() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, true,
				false);
		ObjectNode ObjectNode = ConstantNode.createConstantNode(null, rel);
		Feed feed = new Feed(ObjectNode);
		feed.typeCheck();
		feed.open();
		assertNull(feed.getNext());
		assertNull(feed.getNext());
		feed.close();
	}

	@Test(expected = TypeException.class)
	public void testFeedInvalidRelation() throws TypeException {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Test", "string"));
		header.add(new RelationHeaderItem("Test", "string"));
		MemoryRelation rel = new MemoryRelation(header);
		Node node = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(node);
		feed.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testFeedWrongInput() throws TypeException {
		Node node = ConstantNode.createConstantNode(new AttributeInt(3),
				new AttributeInt());
		Feed feed = new Feed(node);
		feed.typeCheck();
	}

	@Test(expected = StreamStateException.class)
	public void testFeedUnopened() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode ObjectNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(ObjectNode);
		feed.typeCheck();
		feed.getNext();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (feed REL)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		assertEquals(rel, mobject);
	}

}
