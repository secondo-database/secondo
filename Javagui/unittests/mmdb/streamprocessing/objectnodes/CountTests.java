package unittests.mmdb.streamprocessing.objectnodes;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Count;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class CountTests {

	@Test
	public void testOperator() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Count count = new Count(feed);
		count.typeCheck();

		assertEquals(AttributeInt.class, count.getOutputType().getClass());
		assertEquals(5, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testRelation() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(17, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Count count = new Count(relNode);
		count.typeCheck();

		assertEquals(17, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testEmpty() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Count count = new Count(feed);
		count.typeCheck();

		assertEquals(AttributeInt.class, count.getOutputType().getClass());
		assertEquals(0, ((AttributeInt) count.getResult()).getValue());
	}

	@Test
	public void testNull() throws Exception {
		ObjectNode constant = ConstantNode.createConstantNode(null,
				MemoryRelation.createTypecheckInstance(TestUtilRelation
						.getIntStringRelation(5, true, true).getHeader()));
		Count count = new Count(constant);
		count.typeCheck();
		assertNull(count.getResult());
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "R1: REL");
		String query = "(query (count (feed R1)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();
		MemoryObject expected = new AttributeInt(5);
		assertEquals(expected, mobject);
	}

}
