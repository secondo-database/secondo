package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.Node;
import mmdb.streamprocessing.functionoperators.ParameterFunction;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.FunctionEnvironment;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.objectnodes.aggregation.Max;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Groupby;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;

public class GroupbyTests {

	@Test
	public void testGroupbyStandard() throws TypeException, MemoryException {
		Groupby groupBy = getGroupBy();
		groupBy.typeCheck();

		// Tests
		MemoryTuple tupleInfo = (MemoryTuple) groupBy.getOutputType();
		List<RelationHeaderItem> list = tupleInfo.getTypecheckInfo();
		assertEquals("Augenfarbe", list.get(0).getIdentifier());
		assertEquals("Max_Groesse", list.get(1).getIdentifier());

		groupBy.open();
		MemoryTuple tuple = (MemoryTuple) groupBy.getNext();
		assertEquals("Blau",
				((AttributeString) tuple.getAttribute(0)).getValue());
		assertEquals(198, ((AttributeInt) tuple.getAttribute(1)).getValue());

		tuple = (MemoryTuple) groupBy.getNext();
		assertEquals("Braun",
				((AttributeString) tuple.getAttribute(0)).getValue());
		assertEquals(159, ((AttributeInt) tuple.getAttribute(1)).getValue());

		tuple = (MemoryTuple) groupBy.getNext();
		assertNull(tuple);
		groupBy.close();
	}

	@Test(expected = StreamStateException.class)
	public void testGroupbyStreamState() throws TypeException, MemoryException {
		Groupby groupBy = getGroupBy();
		groupBy.typeCheck();

		groupBy.getNext();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = getTestRelation();
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (groupby (feed REL) (Augenfarbe) ("
				+ "(Gesamtgroesse (fun (group1 GROUP) (sum (feed group1) Groesse)))))))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryRelation expected = getTestRelation();
		expected.getHeader().remove(0);
		expected.getHeader().remove(1);
		expected.getHeader()
				.add(new RelationHeaderItem("Gesamtgroesse", "int"));

		expected.getTuples().removeAll(rel.getTuples());
		expected.getTuples().add(
				new MemoryTuple(new AttributeString("Blau"), new AttributeInt(
						550)));
		expected.getTuples().add(
				new MemoryTuple(new AttributeString("Braun"), new AttributeInt(
						316)));
		assertEquals(expected, mobject);
	}

	private Groupby getGroupBy() {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Function
		FunctionEnvironment funEnv = new FunctionEnvironment();
		Feed feedFun = new Feed(funEnv);
		Max max = new Max(feedFun, "Groesse");
		ParameterFunction fun = new ParameterFunction(new Node[] { funEnv },
				max);

		// Hashmap
		LinkedHashMap<String, Node> map = new LinkedHashMap<String, Node>();
		map.put("Max_Groesse", fun);

		// Identifier
		String[] groupingColumns = { "Augenfarbe" };

		// Groupby
		return new Groupby(feed, groupingColumns, map);
	}

	private MemoryRelation getTestRelation() {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Name", "string"));
		header.add(new RelationHeaderItem("Augenfarbe", "string"));
		header.add(new RelationHeaderItem("Groesse", "int"));

		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		tuples.add(new MemoryTuple(new AttributeString("Mark"),
				new AttributeString("Blau"), new AttributeInt(180)));
		tuples.add(new MemoryTuple(new AttributeString("Olli"),
				new AttributeString("Blau"), new AttributeInt(172)));
		tuples.add(new MemoryTuple(new AttributeString("Mike"),
				new AttributeString("Blau"), new AttributeInt(198)));
		tuples.add(new MemoryTuple(new AttributeString("Luna"),
				new AttributeString("Braun"), new AttributeInt(159)));
		tuples.add(new MemoryTuple(new AttributeString("Maria"),
				new AttributeString("Braun"), new AttributeInt(157)));

		MemoryRelation relation = new MemoryRelation(header);
		relation.setTuples(tuples);
		return relation;
	}

}
