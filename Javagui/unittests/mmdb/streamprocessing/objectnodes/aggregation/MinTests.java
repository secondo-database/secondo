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
import mmdb.streamprocessing.objectnodes.aggregation.Min;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class MinTests {

	@Test
	public void testMinInt() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				true);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierInt");
		min.typeCheck();
		assertEquals(1, ((AttributeInt) min.getResult()).getValue());
	}

	@Test
	public void testMinReal() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(4, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Min min = new Min(feed, "identifierReal");
		min.typeCheck();
		assertEquals(1f, ((AttributeReal) min.getResult()).getValue(), 0.001f);
	}

	@Test
	public void testMinEmpty() throws TypeException, MemoryException {
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

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (min (feed REL) identifierInt))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();
		MemoryObject expected = new AttributeInt(1);
		assertEquals(expected, mobject);
	}

}
