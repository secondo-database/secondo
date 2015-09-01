package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.MemoryAttribute;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.Attr;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.condition.Equals;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Filter;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class FilterTests {

	@Test
	public void testFilterInt() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		Filter filter = createFilter(rel, new AttributeInt(3), "identifierInt");
		filter.typeCheck();

		filter.open();
		assertEquals(MemoryTuple.class, filter.getOutputType().getClass());
		assertEquals(rel.getTypecheckInfo(),
				((MemoryTuple) filter.getOutputType()).getTypecheckInfo());
		assertEquals(new AttributeInt(3),
				((MemoryTuple) filter.getNext()).getAttribute(0));
		assertNull(filter.getNext());
		filter.close();
	}

	@Test
	public void testFilterString() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				true);
		Filter filter = createFilter(rel, new AttributeString("string_4"),
				"identifierString");
		filter.typeCheck();

		filter.open();
		assertEquals(new AttributeString("string_4"),
				((MemoryTuple) filter.getNext()).getAttribute(0));
		assertNull(filter.getNext());
		filter.close();
	}

	@Test
	public void testFilterIntMultiple() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		// Should never be done
		rel.getTuples().add(
				new MemoryTuple(new AttributeInt(3), new AttributeString(
						"extra")));
		Filter filter = createFilter(rel, new AttributeInt(3), "identifierInt");
		filter.typeCheck();

		filter.open();
		assertEquals(new AttributeString("string_3"),
				((MemoryTuple) filter.getNext()).getAttribute(1));
		assertEquals(new AttributeString("extra"),
				((MemoryTuple) filter.getNext()).getAttribute(1));
		assertNull(filter.getNext());
		filter.close();
	}

	@Test(expected = TypeException.class)
	public void testFilterFail() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		Filter filter = createFilter(rel, new AttributeInt(3),
				"identifierNonPresent");
		filter.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testFilterFailException() throws TypeException {
		// Feed
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		ObjectNode attrConst = ConstantNode.createConstantNode(
				new AttributeString("a"), new AttributeString());
		FunctionEnvironment funEnv = new FunctionEnvironment();
		ParameterFunction fun = new ParameterFunction(new Node[] {funEnv}, attrConst);

		Filter filter = new Filter(feed, fun);
		filter.typeCheck();
	}

	@Test
	public void testNullObjectNodeReaction() throws TypeException {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		Filter filter = createFilter(rel, null, new AttributeInt(), "identifierInt");
		filter.typeCheck();
		
		filter.open();
		assertNull(filter.getNext());
		filter.close();
	}

	// Helper to create trees
	private Filter createFilter(MemoryRelation rel, MemoryAttribute constAttr,
			String identifier) {
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Function
		ObjectNode attrConst = ConstantNode.createConstantNode(constAttr,
				constAttr);
		FunctionEnvironment funEnv = new FunctionEnvironment();
		Attr attr = new Attr(funEnv, identifier);
		Equals equals = new Equals(attr, attrConst);
		ParameterFunction fun = new ParameterFunction(new Node[] {funEnv}, equals);

		// Filter
		return new Filter(feed, fun);
	}

	// Helper to create trees
	private Filter createFilter(MemoryRelation rel, MemoryAttribute constAttr,
			MemoryAttribute constType, String identifier) {
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Function
		ObjectNode attrConst = ConstantNode.createConstantNode(constAttr,
				constType);
		FunctionEnvironment funEnv = new FunctionEnvironment();
		Attr attr = new Attr(funEnv, identifier);
		Equals equals = new Equals(attr, attrConst);
		ParameterFunction fun = new ParameterFunction(new Node[] {funEnv}, equals);

		// Filter
		return new Filter(feed, fun);
	}

}
