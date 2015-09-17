package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import gui.SecondoObject;
import mmdb.data.MemoryObject;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.RelationHeaderItem;
import mmdb.error.memory.MemoryException;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.parser.NestedListProcessor;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Rename;

import org.junit.Test;

import unittests.mmdb.util.TestUtilParser;
import unittests.mmdb.util.TestUtilRelation;

public class RenameTests {

	@Test
	public void testRename() throws TypeException, MemoryException {
		String postfix = "test";

		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Rename rename = new Rename(feed, postfix);
		rename.typeCheck();

		String firstID = ((MemoryTuple) feed.getOutputType())
				.getTypecheckInfo().get(0).getIdentifier();
		String secondID = ((MemoryTuple) feed.getOutputType())
				.getTypecheckInfo().get(1).getIdentifier();

		// Did the rename itself work?
		assertEquals(firstID + "_" + postfix,
				((MemoryTuple) rename.getOutputType()).getTypecheckInfo()
						.get(0).getIdentifier());
		assertEquals(secondID + "_" + postfix,
				((MemoryTuple) rename.getOutputType()).getTypecheckInfo()
						.get(1).getIdentifier());

		// Does the rename put out the same tuples?
		rename.open();
		for (MemoryTuple tuple : rel.getTuples()) {
			assertEquals(tuple, rename.getNext());
		}
		rename.close();
	}

	@Test(expected = TypeException.class)
	public void testRenameEmptyPostfix() throws TypeException {
		String postfix = "";

		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, true,
				false);
		ObjectNode relNode = ConstantNode.createConstantNode(rel, rel);
		Feed feed = new Feed(relNode);
		Rename rename = new Rename(feed, postfix);
		rename.typeCheck();
	}

	@Test
	public void testQuery() throws Exception {
		MemoryRelation rel = TestUtilRelation.getIntStringRelation(5, false,
				false);
		SecondoObject sobject = TestUtilParser.getSecondoObject(rel, "REL");
		String query = "(query (consume (rename (feed REL) a)))";
		ObjectNode result = NestedListProcessor.buildOperatorTree(query,
				TestUtilParser.getList(sobject));
		result.typeCheck();
		MemoryObject mobject = result.getResult();

		MemoryRelation expected = TestUtilRelation.getIntStringRelation(5,
				false, false);
		RelationHeaderItem old_item0 = expected.getHeader().get(0);
		RelationHeaderItem old_item1 = expected.getHeader().get(1);
		expected.getHeader().remove(0);
		expected.getHeader().remove(0);
		expected.getHeader().add(
				new RelationHeaderItem(old_item0.getIdentifier() + "_a",
						old_item0.getTypeName()));
		expected.getHeader().add(
				new RelationHeaderItem(old_item1.getIdentifier() + "_a",
						old_item1.getTypeName()));
		assertEquals(expected, mobject);
	}

}
