package unittests.mmdb.streamprocessing.objectnodes.aggregation;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.aggregation.Max;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class MaxTests {

	@Test
	public void testMaxInt() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				true);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Max max = new Max(feed, "identifierInt");
		max.typeCheck();
		assertEquals(5, ((AttributeInt) max.getResult()).getValue());
	}

	@Test
	public void testMaxReal() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(4, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Max max = new Max(feed, "identifierReal");
		max.typeCheck();
		assertEquals(4f, ((AttributeReal) max.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testMaxEmpty() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(0, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Max max = new Max(feed, "identifierReal");
		max.typeCheck();
		assertNull(max.getResult());
	}

	@Test(expected = TypeException.class)
	public void testMaxFail2() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Max max = new Max(feed, "identifierWrong");
		max.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (max (feed REL) identifierInt))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();
		MemoryObject expected = new AttributeInt(5);
		assertEquals(expected, mobject);
	}

}
