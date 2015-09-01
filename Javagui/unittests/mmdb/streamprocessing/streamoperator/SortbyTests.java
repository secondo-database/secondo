package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import java.util.ArrayList;
import java.util.LinkedHashMap;
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
import mmdb.streamprocessing.streamoperators.Sortby;

import org.junit.Test;

public class SortbyTests {

	@Test
	public void testSortbyInt() throws TypeException {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Map
		LinkedHashMap<String, String> map = new LinkedHashMap<String, String>();
		map.put("Augenfarbe", "desc");
		map.put("Groesse", "asc");

		// Sortby
		Sortby sortBy = new Sortby(feed, map);
		sortBy.typeCheck();

		assertEquals(feed.getOutputType(), sortBy.getOutputType());

		sortBy.open();
		assertEquals(rel.getTuples().get(7), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(1), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(4), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(3), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(6), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(2), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(0), (MemoryTuple) sortBy.getNext());
		assertEquals(rel.getTuples().get(5), (MemoryTuple) sortBy.getNext());
		assertNull(sortBy.getNext());
		sortBy.close();
	}

	@Test(expected = StreamStateException.class)
	public void testSortbyUnopened() throws TypeException {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Map
		LinkedHashMap<String, String> map = new LinkedHashMap<String, String>();
		map.put("Augenfarbe", "desc");
		map.put("Groesse", "asc");

		// Sortby
		Sortby sortBy = new Sortby(feed, map);
		sortBy.typeCheck();

		sortBy.getNext();
	}

	@Test(expected = TypeException.class)
	public void testSortbyWrongDirection() throws TypeException {
		MemoryRelation rel = getTestRelation();
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);

		// Map
		LinkedHashMap<String, String> map = new LinkedHashMap<String, String>();
		map.put("Augenfarbe", "ascii");

		// Sortby
		Sortby sortBy = new Sortby(feed, map);
		sortBy.typeCheck();
	}

	@Test
	public void testSortAttributeNullReaction() throws TypeException {
		List<RelationHeaderItem> header = new ArrayList<RelationHeaderItem>();
		header.add(new RelationHeaderItem("Name", "string"));
		header.add(new RelationHeaderItem("Augenfarbe", "string"));
		header.add(new RelationHeaderItem("Groesse", "int"));

		List<MemoryTuple> tuples = new ArrayList<MemoryTuple>();
		tuples.add(new MemoryTuple(new AttributeString("Mark"), null, null));
		tuples.add(new MemoryTuple(new AttributeString("Marius"),
				new AttributeString("Blau"), new AttributeInt(170)));
		tuples.add(new MemoryTuple(new AttributeString("Olli"),
				new AttributeString("Blau"), null));
		tuples.add(new MemoryTuple(new AttributeString("Luna"), null,
				new AttributeInt(170)));

		MemoryRelation relation = new MemoryRelation(header);
		relation.setTuples(tuples);

		ObjectNode relNode = ConstantNode
				.createConstantNode(relation, relation);
		Feed feed = new Feed(relNode);

		// Map
		LinkedHashMap<String, String> map = new LinkedHashMap<String, String>();
		map.put("Augenfarbe", "desc");
		map.put("Groesse", "desc");

		// Sortby
		Sortby sortBy = new Sortby(feed, map);
		sortBy.typeCheck();

		sortBy.open();
		assertEquals(relation.getTuples().get(1),
				(MemoryTuple) sortBy.getNext());
		assertEquals(relation.getTuples().get(2),
				(MemoryTuple) sortBy.getNext());
		assertEquals(relation.getTuples().get(3),
				(MemoryTuple) sortBy.getNext());
		assertEquals(relation.getTuples().get(0),
				(MemoryTuple) sortBy.getNext());
		assertNull(sortBy.getNext());
		sortBy.close();
	}

	private MemoryRelation getTestRelation() {
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

}
