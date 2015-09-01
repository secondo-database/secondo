package unittests.mmdb.streamprocessing.objectnodes;

import static org.junit.Assert.assertEquals;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Count;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class CountTests {

	@Test
	public void testCountOperator() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5,
				true, false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Count count = new Count(feed);
		count.typeCheck();

		assertEquals(AttributeInt.class, count.getOutputType().getClass());
		assertEquals(5, ((AttributeInt) count.getResult()).getValue());
	}
	
	@Test
	public void testCountRelation() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(17, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Count count = new Count(relNode);
		count.typeCheck();

		assertEquals(17, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testCountEmpty() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0,
				true, false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Count count = new Count(feed);
		count.typeCheck();

		assertEquals(AttributeInt.class, count.getOutputType().getClass());
		assertEquals(0, ((AttributeInt) count.getResult()).getValue());
	}

}
