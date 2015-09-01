package unittests.mmdb.streamprocessing.objectnodes.aggregation;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.aggregation.Min;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class MinTests {

	@Test
	public void testMinInt() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				true);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierInt");
		min.typeCheck();
		assertEquals(1, ((AttributeInt) min.getResult()).getValue());
	}

	@Test
	public void testMinReal() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(4, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierReal");
		min.typeCheck();
		assertEquals(1f, ((AttributeReal) min.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testMinEmpty() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(0, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierReal");
		min.typeCheck();
		assertNull(min.getResult());
	}

	@Test(expected = TypeException.class)
	public void testMinFail() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierWrong");
		min.typeCheck();
	}

}
