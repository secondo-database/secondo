package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class FeedTests {

	@Test
	public void testFeed() throws TypeException {
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
	public void testFeedEmpty() throws TypeException {
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
	
	@Test(expected = TypeException.class)
	public void testFeedInvalidRelation() throws TypeException {
		List<RelationHeaderItem> header = new ArrayList<>();
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

}
