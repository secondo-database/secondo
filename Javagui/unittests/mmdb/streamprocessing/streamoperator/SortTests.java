package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.ArrayList;
import java.util.List;

import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.StreamStateException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Sort;

import org.junit.Test;

public class SortTests {

	@Test
	public void testSort() throws TypeException {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Sort
		Sort sort = new Sort(feed);
		sort.typeCheck();

		assertEquals(feed.getOutputType(), sort.getOutputType());

		sort.open();
		assertEquals(rel.getTuples().get(2), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(0), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(5), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(4), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(3), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(6), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(1), (MemoryTuple) sort.getNext());
		assertEquals(rel.getTuples().get(7), (MemoryTuple) sort.getNext());
		assertNull(sort.getNext());
		sort.close();
	}

	@Test(expected = StreamStateException.class)
	public void testSortUnopened() throws TypeException {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Sort
		Sort sort = new Sort(feed);
		sort.typeCheck();

		sort.getNext();
	}

	private MemoryRelation getTestRelation() {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Augenfarbe", "string"));
		header.add(new RelationHeaderItem("Groesse", "int"));
		header.add(new RelationHeaderItem("Name", "string"));

		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		tuples.add(new MemoryTuple(new AttributeString("Blau"),
				new AttributeInt(180), new AttributeString("Mark")));
		tuples.add(new MemoryTuple(new AttributeString("Grau"),
				new AttributeInt(185), new AttributeString("Marius")));
		tuples.add(new MemoryTuple(new AttributeString("Blau"),
				new AttributeInt(172), new AttributeString("Olli")));
		tuples.add(new MemoryTuple(new AttributeString("Braun"),
				new AttributeInt(159), new AttributeString("Luna")));
		tuples.add(new MemoryTuple(new AttributeString("Braun"),
				new AttributeInt(157), new AttributeString("Maria")));
		tuples.add(new MemoryTuple(new AttributeString("Blau"),
				new AttributeInt(198), new AttributeString("Mike")));
		tuples.add(new MemoryTuple(new AttributeString("Braun"),
				new AttributeInt(159), new AttributeString("Luna")));
		tuples.add(new MemoryTuple(new AttributeString("Gruen"),
				new AttributeInt(188), new AttributeString("Carla")));

		MemoryRelation relation = new MemoryRelation(header);
		relation.setTuples(tuples);
		return relation;
	}

}
