package unittests.mmdb.streamprocessing.streamoperator;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import mmdb.data.MemoryRelation;
import mmdb.data.MemoryTuple;
import mmdb.data.attributes.standard.AttributeInt;
import mmdb.data.attributes.standard.AttributeString;
import mmdb.error.streamprocessing.TypeException;
import mmdb.streamprocessing.objectnodes.ConstantNode;
import mmdb.streamprocessing.objectnodes.ObjectNode;
import mmdb.streamprocessing.streamoperators.Feed;
import mmdb.streamprocessing.streamoperators.Project;

import org.junit.Before;
import org.junit.Test;

import unittests.mmdb.util.TestUtilRelation;

public class ProjectTests {

	MemoryRelation rel = null;
	ObjectNode relNode = null;
	Feed feed = null;
	Project project = null;

	@Before
	public void setUp() {
		rel = TestUtilRelation.getIntStringRelation(5, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
	}

	@Test
	public void testProjectTuples1() throws TypeException {
		project = new Project(feed, new String[] {"identifierInt"});
		project.typeCheck();

		project.open();
		for (int i = 1; i <= 5; i++) {
			assertEquals(i,
					((AttributeInt) project.getNext().getAttribute(0))
							.getValue());
		}
		project.close();
	}

	@Test
	public void testProjectTuples2() throws TypeException {
		project = new Project(feed, new String[] {"identifierInt", "identifierString"});
		project.typeCheck();

		project.open();
		for (int i = 1; i <= 5; i++) {
			MemoryTuple tuple = project.getNext();
			assertEquals(i, ((AttributeInt) tuple.getAttribute(0)).getValue());
			assertEquals("string_" + i,
					((AttributeString) tuple.getAttribute(1)).getValue());
		}
		project.close();
	}

	@Test
	public void testProjectTuplesEmpty() throws TypeException {
		rel = TestUtilRelation.getIntStringRelation(0, true, false);
		relNode = ConstantNode.createConstantNode(rel, rel);
		feed = new Feed(relNode);
		project = new Project(feed, new String[] {"identifierInt", "identifierString"});
		project.typeCheck();

		project.open();
		assertNull(project.getNext());
		project.close();
	}

	@Test
	public void testProjectHeader1() throws TypeException {
		project = new Project(feed, new String[] {"identifierInt"});
		project.typeCheck();

		assertEquals(1, ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt", ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().get(0).getIdentifier());
	}

	@Test
	public void testProjectHeader2() throws TypeException {
		project = new Project(feed, new String[] {"identifierInt", "identifierString"});
		project.typeCheck();

		assertEquals(2, ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().size());
		assertEquals("identifierInt", ((MemoryTuple) project.getOutputType())
				.getTypecheckInfo().get(0).getIdentifier());
		assertEquals(
				"identifierString",
				((MemoryTuple) project.getOutputType()).getTypecheckInfo()
						.get(1).getIdentifier());
	}

	@Test(expected = TypeException.class)
	public void testProjectHeaderFail1() throws TypeException {
		project = new Project(feed, new String[] {"identifierReal"});
		project.typeCheck();
	}

	@Test(expected = TypeException.class)
	public void testProjectHeaderFail2() throws TypeException {
		project = new Project(feed, new String[] {"identifierInt", "identifierInt"});
		project.typeCheck();
	}

}
