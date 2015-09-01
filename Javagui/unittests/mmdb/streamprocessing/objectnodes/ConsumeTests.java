package unittests.mmdb.streamprocessing.objectnodes;

import static org.junit.Assert.assertEquals;
import mmdb.data.MemoryRelation;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Consume;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class ConsumeTests {

	@Test
	public void testConsume() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Consume consume = new Consume(feed);
		consume.typeCheck();

		assertEquals(rel.getTuples(), consume.getResult().getTuples());
		assertEquals(rel.getHeader(), consume.getResult().getHeader());
	}

	@Test
	public void testConsumeChain() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Consume consume = new Consume(feed);
		Feed feed2 = new Feed(consume);
		Consume consume2 = new Consume(feed2);
		consume2.typeCheck();

		assertEquals(rel.getTuples(), consume2.getResult().getTuples());
		assertEquals(rel.getHeader(), consume2.getResult().getHeader());
	}

	@Test
	public void testConsumeEmpty() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Consume consume = new Consume(feed);
		consume.typeCheck();

		assertEquals(rel.getHeader(), consume.getOutputType().getHeader());
		assertEquals(0, consume.getResult().getTuples().size());
		assertEquals(rel.getTuples(), consume.getResult().getTuples());
	}

}
