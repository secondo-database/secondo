package unittests.mmdb.streamprocessing.objectnodes.aggregation;

import static org.junit.Assert.assertEquals;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.aggregation.Average;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class AverageTests {

	@Test
	public void testAverageInt() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Average average = new Average(feed, "identifierInt");
		average.typeCheck();
		assertEquals(AttributeReal.class, average.getOutputType().getClass());
		assertEquals(1.5f, ((AttributeReal) average.getResult()).getValue(),
				0.001f);
	}

	@Test
	public void testAverageReal() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getRealStringRelation(4, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Average average = new Average(feed, "identifierReal");
		average.typeCheck();
		assertEquals(2.5f, ((AttributeReal) average.getResult()).getValue(),
				0.001f);
	}

	@Test
	public void testAverageEmpty() throws TypeException, MemoryException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(0, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Average average = new Average(feed, "identifierInt");
		average.typeCheck();
		assertEquals(null, average.getResult());
	}

	@Test(expected = TypeException.class)
	public void testAverageFail1() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Average average = new Average(feed, "identifierString");
		average.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testAverageFail2() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(2, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Average average = new Average(feed, "identifierWrong");
		average.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (avg (feed REL) identifierInt))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();
		MemoryObject expected = new AttributeReal(3);
		assertEquals(expected, mobject);
	}

}
