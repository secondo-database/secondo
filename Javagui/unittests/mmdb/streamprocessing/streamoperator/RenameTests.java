package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Rename;

import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class RenameTests {

	@Test
	public void testRename() throws TypeException {
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

}
