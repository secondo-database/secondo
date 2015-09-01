package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeReal;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Hashjoin;

import org.junit.Test;

public class HashjoinTests {

	@Test
	public void testHashjoin() throws TypeException {
		MemoryRelation rel1 = getRel1();
		MemoryRelation rel2 = getRel2();

		ObjectNode relNode1 = ConstantNode.createConstantNode(rel1, rel1);
		Feed feed1 = new Feed(relNode1);

		ObjectNode relNode2 = ConstantNode.createConstantNode(rel2, rel2);
		Feed feed2 = new Feed(relNode2);

		Hashjoin hashjoin = new Hashjoin(feed1, feed2, "Augenfarbe",
				"Wandfarbe");

		hashjoin.typeCheck();

		MemoryTuple a = getExpectedOutputType();
		MemoryTuple b = (MemoryTuple) hashjoin.getOutputType();
		assertEquals(a.getTypecheckInfo(), b.getTypecheckInfo());

		hashjoin.open();

		MemoryTuple tuple;

		tuple = (MemoryTuple) hashjoin.getNext();
		assertEquals("Mark",
				((AttributeString) tuple.getAttribute(0)).getValue());
		assertEquals("Blau",
				((AttributeString) tuple.getAttribute(1)).getValue());
		assertEquals(180, ((AttributeInt) tuple.getAttribute(2)).getValue());
		assertEquals("Blau",
				((AttributeString) tuple.getAttribute(3)).getValue());
		assertEquals(17.9f, ((AttributeReal) tuple.getAttribute(4)).getValue(),
				0.1f);

		tuple = (MemoryTuple) hashjoin.getNext();
		tuple = (MemoryTuple) hashjoin.getNext();
		tuple = (MemoryTuple) hashjoin.getNext();
		assertEquals("Olli",
				((AttributeString) tuple.getAttribute(0)).getValue());
		assertEquals("Blau",
				((AttributeString) tuple.getAttribute(1)).getValue());
		assertEquals(172, ((AttributeInt) tuple.getAttribute(2)).getValue());
		assertEquals("Blau",
				((AttributeString) tuple.getAttribute(3)).getValue());
		assertEquals(29.99f,
				((AttributeReal) tuple.getAttribute(4)).getValue(), 0.1f);

		tuple = (MemoryTuple) hashjoin.getNext();
		tuple = (MemoryTuple) hashjoin.getNext();
		assertEquals("Maria",
				((AttributeString) tuple.getAttribute(0)).getValue());
		assertEquals("Braun",
				((AttributeString) tuple.getAttribute(1)).getValue());
		assertEquals(157, ((AttributeInt) tuple.getAttribute(2)).getValue());
		assertEquals("Braun",
				((AttributeString) tuple.getAttribute(3)).getValue());
		assertEquals(3.95f, ((AttributeReal) tuple.getAttribute(4)).getValue(),
				0.1f);
		hashjoin.close();

	}

	@Test(expected = StreamStateException.class)
	public void testHashjoinUnopened() throws TypeException {
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

}
