package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import gui.SecondoObject;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.Consume;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Hashjoin;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;

public class HashjoinTests {

	@Test
	public void testHashjoin() throws TypeException, MemoryException {
		MemoryRelation rel1 = getRel1();
		MemoryRelation rel2 = getRel2();

		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		Feed feed1 = new Feed(relNode1);

		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		Feed feed2 = new Feed(relNode2);

		Hashjoin hashjoin = new Hashjoin(feed1, feed2, "Augenfarbe",
				"Wandfarbe");

		Consume consume = new Consume(hashjoin);

		consume.typeCheck();

		MemoryObject mobject = consume.getResult();
		MemoryObject expected = getExpectedOutputRelation();
		assertEquals(expected, mobject);

	}

	@Test(expected = StreamStateException.class)
	public void testHashjoinUnopened() throws TypeException, MemoryException {
		MemoryRelation rel1 = getRel1();
		MemoryRelation rel2 = getRel2();

		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		Feed feed1 = new Feed(relNode1);

		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		Feed feed2 = new Feed(relNode2);

		Hashjoin hashjoin = new Hashjoin(feed1, feed2, "Augenfarbe",
				"Wandfarbe");

		hashjoin.typeCheck();
		hashjoin.getNext();
	}

	@Test(expected = TypeException.class)
	public void testIdentifierDuplication() throws TypeException {
		MemoryRelation rel1 = getRel1();
		MemoryRelation rel2 = getRel1();

		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		Feed feed1 = new Feed(relNode1);

		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		Feed feed2 = new Feed(relNode2);

		Hashjoin hashjoin = new Hashjoin(feed1, feed2, "Augenfarbe",
				"Augenfarbe");

		hashjoin.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testIdentifierTypeMismatch() throws TypeException {
		MemoryRelation rel1 = getRel1();
		MemoryRelation rel2 = getRel1();

		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		Feed feed1 = new Feed(relNode1);

		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		Feed feed2 = new Feed(relNode2);

		Hashjoin hashjoin = new Hashjoin(feed1, feed2, "Augenfarbe", "Groesse");

		hashjoin.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		SecondoObject sobject1 = TestUtilParser.getSecondoObject(getRel1(),
				"REL1");
		SecondoObject sobject2 = TestUtilParser.getSecondoObject(getRel2(),
				"REL2");
		String query = "(query (consume (hashjoin (feed REL1) (feed REL2) Augenfarbe Wandfarbe)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject1, sobject2));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryObject expected = getExpectedOutputRelation();
		assertEquals(expected, mobject);
	}

	private MemoryRelation getRel1() {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Name", "string"));
		header.add(new RelationHeaderItem("Augenfarbe", "string"));
		header.add(new RelationHeaderItem("Groesse", "int"));

		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		tuples.add(new MemoryTuple(new AttributeString("Mark"),
				new AttributeString("Blau"), new AttributeInt(180)));
		tuples.add(new MemoryTuple(new AttributeString("Marius"),
				new AttributeString("Grau"), new AttributeInt(185)));
		tuples.add(new MemoryTuple(new AttributeString("Olli"),
				new AttributeString("Blau"), new AttributeInt(172)));
		tuples.add(new MemoryTuple(new AttributeString("Luna"),
				new AttributeString("Braun"), new AttributeInt(159)));
		tuples.add(new MemoryTuple(new AttributeString("Maria"),
				new AttributeString("Braun"), new AttributeInt(157)));
		tuples.add(new MemoryTuple(new AttributeString("Mike"),
				new AttributeString("Blau"), new AttributeInt(198)));
		tuples.add(new MemoryTuple(new AttributeString("Luna"),
				new AttributeString("Braun"), new AttributeInt(159)));
		tuples.add(new MemoryTuple(new AttributeString("Carla"),
				new AttributeString("Gruen"), new AttributeInt(188)));

		MemoryRelation relation = new MemoryRelation(header);
		relation.setTuples(tuples);
		return relation;
	}

	private MemoryRelation getRel2() {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Wandfarbe", "string"));
		header.add(new RelationHeaderItem("Preis", "real"));

		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		tuples.add(new MemoryTuple(new AttributeString("Blau"),
				new AttributeReal(17.90f)));
		tuples.add(new MemoryTuple(new AttributeString("Blau"),
				new AttributeReal(29.99f)));
		tuples.add(new MemoryTuple(new AttributeString("Braun"),
				new AttributeReal(3.95f)));
		tuples.add(new MemoryTuple(new AttributeString("Weiß"),
				new AttributeReal(14.95f)));

		MemoryRelation relation = new MemoryRelation(header);
		relation.setTuples(tuples);
		return relation;
	}

	private MemoryTuple getExpectedOutputType() {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Name", "string"));
		header.add(new RelationHeaderItem("Augenfarbe", "string"));
		header.add(new RelationHeaderItem("Groesse", "int"));
		header.add(new RelationHeaderItem("Wandfarbe", "string"));
		header.add(new RelationHeaderItem("Preis", "real"));
		return MemoryTuple.createTypecheckInstance(header);
	}

	private MemoryRelation getExpectedOutputRelation() {
		MemoryRelation rel = new MemoryRelation(getExpectedOutputType()
				.getTypecheckInfo());
		addTupleToRel(rel, "Mark", "Blau", 180, "Blau", 17.9f);
		addTupleToRel(rel, "Mark", "Blau", 180, "Blau", 29.99f);
		addTupleToRel(rel, "Olli", "Blau", 172, "Blau", 17.9f);
		addTupleToRel(rel, "Olli", "Blau", 172, "Blau", 29.99f);
		addTupleToRel(rel, "Luna", "Braun", 159, "Braun", 3.95f);
		addTupleToRel(rel, "Maria", "Braun", 157, "Braun", 3.95f);
		addTupleToRel(rel, "Mike", "Blau", 198, "Blau", 17.9f);
		addTupleToRel(rel, "Mike", "Blau", 198, "Blau", 29.99f);
		addTupleToRel(rel, "Luna", "Braun", 159, "Braun", 3.95f);
		return rel;
	}

	private void addTupleToRel(MemoryRelation rel, String nameS,
			String augenfarbeS, int groesseS, String wandfarbeS, float preisS) {
		AttributeString name = new AttributeString(nameS);
		AttributeString augenfarbe = new AttributeString(augenfarbeS);
		AttributeInt groesse = new AttributeInt(groesseS);
		AttributeString wandfarbe = new AttributeString(wandfarbeS);
		AttributeReal preis = new AttributeReal(preisS);
		MemoryTuple tuple = new MemoryTuple(name, augenfarbe, groesse,
				wandfarbe, preis);
		rel.getTuples().add(tuple);
	}

}
