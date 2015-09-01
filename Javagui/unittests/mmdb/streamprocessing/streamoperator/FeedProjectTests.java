package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.FeedProject;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class FeedProjectTests {

	MemoryRelation rel;
	ObjectNode relNode = null;
	FeedProject feedProject;

	@Before
	public void setUp() {
		rel = TestUtilRelation.getIntStringRelation(5, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
	}

	@Test
	public void testFeedProjectTuples1() throws TypeException {
		feedProject = new FeedProject(relNode, new String[] { "identifierInt" });
		feedProject.typeCheck();

		feedProject.open();
		for (int i = 1; i <= 5; i++) {
			assertEquals(i,
					((AttributeInt) feedProject.getNext().getAttribute(0))
							.getValue());
		}
		feedProject.close();
	}

	@Test
	public void testFeedProjectTuples2() throws TypeException {
		feedProject = new FeedProject(relNode, new String[] { "identifierInt",
				"identifierString" });
		feedProject.typeCheck();

		feedProject.open();
		for (int i = 1; i <= 5; i++) {
			MemoryTuple tuple = feedProject.getNext();
			assertEquals(i, ((AttributeInt) tuple.getAttribute(0)).getValue());
			assertEquals("string_" + i,
					((AttributeString) tuple.getAttribute(1)).getValue());
		}
		feedProject.close();
	}

	@Test
	public void testFeedProjectHeader1() throws TypeException {
		feedProject = new FeedProject(relNode, new String[] { "identifierInt" });
		feedProject.typeCheck();

		assertEquals(1, ((MemoryTuple) feedProject.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt",
				((MemoryTuple) feedProject.getOutputType()).getTypecheckInfo()
						.get(0).getIdentifier());
	}

	@Test
	public void testFeedProjectHeader2() throws TypeException {
		feedProject = new FeedProject(relNode, new String[] { "identifierInt",
				"identifierString" });
		feedProject.typeCheck();

		assertEquals(2, ((MemoryTuple) feedProject.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt",
				((MemoryTuple) feedProject.getOutputType()).getTypecheckInfo()
						.get(0).getIdentifier());
		assertEquals("identifierString",
				((MemoryTuple) feedProject.getOutputType()).getTypecheckInfo()
						.get(1).getIdentifier());
	}

	@Test(expected = TypeException.class)
	public void testFeedProjectHeaderFail1() throws TypeException {
		feedProject = new FeedProject(relNode,
				new String[] { "identifierReal" });
		feedProject.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testFeedProjectHeaderFail2() throws TypeException {
		feedProject = new FeedProject(relNode, new String[] { "identifierInt",
				"identifierInt" });
		feedProject.typeCheck();
	}

}
