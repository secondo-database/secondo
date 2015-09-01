package unittests.mmdb.streamprocessing.objectnodes.aggregation;

import static org.junit.Assert.assertEquals;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.aggregation.Sum;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class SumTests {

	@Test
	public void testSumInt() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierInt");
		sum.typeCheck();
		assertEquals(sum.getOutputType().getClass(), AttributeInt.class);
		assertEquals(3, ((AttributeInt) sum.getResult()).getValue());
	}

	@Test
	public void testSumEmptyInt() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierInt");
		sum.typeCheck();
		assertEquals(null, sum.getResult());
	}

	@Test
	public void testSumReal() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(2, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierReal");
		sum.typeCheck();
		assertEquals(3.0f, ((AttributeReal) sum.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testSumEmptyReal() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(0, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierReal");
		sum.typeCheck();
		assertEquals(null, sum.getResult());
	}

	@Test(expected = TypeException.class)
	public void testSumFail1() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierString");
		sum.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testSumFail2() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Sum sum = new Sum(feed, "identifierWrong");
		sum.typeCheck();
	}

}
